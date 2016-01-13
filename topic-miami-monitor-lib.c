/*
 * topic-miami-monitor-lib.c
 *
 * Example and/or library for interpreting Topic Miami SOM monitor devices 
 *
 * (C) Copyright 2013,2015 Topic Embedded Products B.V. (http://www.topic.nl).
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA or see <http://www.gnu.org/licenses/>.
 *
 * You can contact Topic by electronic mail via info@topic.nl or via
 * paper mail at the following address: Postbus 440, 5680 AK Best, The Netherlands.
 */

#include "topic-miami-monitor-lib.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static const char sysfile_ltc2990_pattern[] =
	"/sys/class/hwmon/hwmon%d/%s";
static char* sysfile_som_vcc = NULL;
static char* sysfile_som_temp = NULL;
static char* sysfile_cpu_current = NULL;
static char* sysfile_fpga_current = NULL;

static float ad2999_voltage_scale = 0;
static const char sysfile_ad2999_voltage_scale[] =
	"/sys/bus/iio/devices/iio:device0/in_voltage_scale";
static const char sysfile_ad2999_voltage_0_raw[] =
	"/sys/bus/iio/devices/iio:device0/in_voltage0_raw";
static const char sysfile_ad2999_voltage_1_raw[] =
	"/sys/bus/iio/devices/iio:device0/in_voltage1_raw";
static const char sysfile_ad2999_voltage_2_raw[] =
	"/sys/bus/iio/devices/iio:device0/in_voltage2_raw";
static const char sysfile_ad2999_voltage_3_raw[] =
	"/sys/bus/iio/devices/iio:device0/in_voltage3_raw";

static int gpio_pca9536_base = -1;

static int read_sys_file_int(const char* filename, int* value)
{
	int err;
	FILE* f = fopen(filename, "r");

	if (f == NULL)
		return -errno;
	err = fscanf(f, "%d", value);
	fclose(f);
	if (err != 1)
		return (err < 0) ? err : -EINVAL;
	return 0;
}

static float read_sys_file_float(const char* filename, float* value)
{
	int err;
	FILE* f = fopen(filename, "r");

	if (f == NULL)
		return -errno;
	err = fscanf(f, "%f", value);
	fclose(f);
	if (err != 1)
		return (err < 0) ? err : -EINVAL;
	return 0;
}

static int divide_by(const char* filename, int* value, int divider)
{
	int r = read_sys_file_int(filename, value);
	if (r == 0)
		*value /= divider;
	return r;
}

static int raw_to_millivolt(const char* filename, int* value)
{
	int r;

	if (!ad2999_voltage_scale) {
		r = read_sys_file_float(sysfile_ad2999_voltage_scale, &ad2999_voltage_scale);
		if (r)
			return r;
		ad2999_voltage_scale *= 1.25f; /* 5k/20k divider */
	}
	r = read_sys_file_int(filename, value);
	if (r)
		return r;
	*value = (int)(*value * ad2999_voltage_scale);
	return 0;
}

static int set_gpio_mode_input(int index)
{
	char fn[64];
	FILE* f;

	f = fopen("/sys/class/gpio/export", "w");
	if (!f)
		return -errno;
	sprintf(fn, "%d", index);
	fputs(fn, f);
	fclose(f);
	
	sprintf(fn, "/sys/class/gpio/gpio%d/direction", index);
	f = fopen(fn, "w");
	if (!f)
		return -errno;
	fputs("in", f);
	fclose(f);

	return 0;
}

static int get_gpio_value(int index, int* value)
{
	char fn[64];
	FILE* f;
	int r;

	sprintf(fn, "/sys/class/gpio/gpio%d/value", index);
	f = fopen(fn, "r");
	if (!f) {
		r = set_gpio_mode_input(index);
		if (r)
			return r;
		f = fopen(fn, "r");
		if (!f)
			return -errno;
	}
	r = fscanf(f, "%d", value);
	if (r != 1)
		return (r < 0) ? r : -EINVAL;
	fclose(f);
}

static int find_ltc2990_sysfiles()
{
	int index;
	FILE* f;
	char buffer[128];
	int status;

	status = -ENODEV;
	for (index = 0; index < 10; ++index) {
		sprintf(buffer, sysfile_ltc2990_pattern, index, "name");
		f = fopen(buffer, "r");
		if (f) {
			if (fgets(buffer, sizeof(buffer), f)) {
				if (strncmp(buffer, "ltc2990", 7) == 0) {
					sprintf(buffer, sysfile_ltc2990_pattern, index, "temp1_input");
					sysfile_som_temp = strdup(buffer);
					sprintf(buffer, sysfile_ltc2990_pattern, index, "curr1_input");
					sysfile_cpu_current = strdup(buffer);
					sprintf(buffer, sysfile_ltc2990_pattern, index, "curr2_input");
					sysfile_fpga_current = strdup(buffer);
					sprintf(buffer, sysfile_ltc2990_pattern, index, "in0_input");
					sysfile_som_vcc = strdup(buffer);
					status = 0;
				}
			}
			fclose(f);
			if (status == 0)
				return 0;
		}
	}
	return status;
}

static int find_ltc2990()
{
	if (sysfile_som_vcc)
		return 0;
	return find_ltc2990_sysfiles();
}

static const char sys_class_gpio[] = "/sys/class/gpio";

static int find_gpiochip_pca9536(const struct dirent* entry)
{
	int result = -ENODEV;
	char buffer[80];
	FILE* f;
	if (strncmp("gpiochip", entry->d_name, 8))
		return -ENODEV;
	sprintf(buffer, "%s/%s/label", sys_class_gpio, entry->d_name);
	f = fopen(buffer, "r");
	if (!f)
		return -ENODEV;
	if (fgets(buffer, sizeof(buffer), f)) {
		/* Check if label matches */
		if (strncmp("pca9536", buffer, 7) == 0) {
			fclose(f);
			sprintf(buffer, "%s/%s/base", sys_class_gpio, entry->d_name);
			f = fopen(buffer, "r");
			if (!f)
				return -ENODEV;
			fscanf(f, "%d", &result);
		}
	}
	fclose(f);
	return result;
}

static int find_pca9536()
{
	if (gpio_pca9536_base < 0) {
		DIR* handle = opendir(sys_class_gpio);
		int result = -ENODEV;
		struct dirent* entry;
		if (!handle)
			return -errno;
		while ((entry = readdir(handle)) != NULL) {
			result = find_gpiochip_pca9536(entry);
			if (result >= 0) /* found it */
				break;
		}
		closedir(handle);
		gpio_pca9536_base = result;
	}
	return gpio_pca9536_base;
}

int get_topic_miami_monitor_value(int item, int* value)
{
	int status;

	switch (item) {
	case TMM_MIAMI_VCC_mV:
		status = find_ltc2990();
		if (status)
			return status;
		return read_sys_file_int(sysfile_som_vcc, value);
	case TMM_MIAMI_TEMP_mC:
		status = find_ltc2990();
		if (status)
			return status;
		return read_sys_file_int(sysfile_som_temp, value);
	case TMM_CPU_CURRENT_mA:
		status = find_ltc2990();
		if (status)
			return status;
		return divide_by(sysfile_cpu_current, value, 5);
	case TMM_FPGA_CURRENT_mA:
		status = find_ltc2990();
		if (status)
			return status;
		return divide_by(sysfile_fpga_current, value, 5);
	case TMM_VCCO0_mV:
		return raw_to_millivolt(sysfile_ad2999_voltage_0_raw, value);
	case TMM_VCCO1_mV:
		return raw_to_millivolt(sysfile_ad2999_voltage_1_raw, value);
	case TMM_VCCO2_mV:
		return raw_to_millivolt(sysfile_ad2999_voltage_2_raw, value);
	case TMM_VDDR_mV:
		return raw_to_millivolt(sysfile_ad2999_voltage_3_raw, value);
	case TMM_VPRESENT:
		status = find_pca9536();
		if (status < 0)
			return status;
		return get_gpio_value(status + 2, value);
	case TMM_DEBUGPRESENT:
		status = find_pca9536();
		if (status < 0)
			return status;
		return get_gpio_value(status + 3, value);
	}
	return -EINVAL;
}
