#include "topic-miami-monitor-lib.h"
#include <errno.h>
#include <stdio.h>

static const char sysfile_som_vcc[] =
	"/sys/class/hwmon/hwmon0/device/vcc";
static const char sysfile_som_temp[] =
	"/sys/class/hwmon/hwmon0/device/temp_int";
static const char sysfile_cpu_current[] =
	"/sys/class/hwmon/hwmon0/device/v1v2_diff";
static const char sysfile_fpga_current[] =
	"/sys/class/hwmon/hwmon0/device/v3v4_diff";

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

int set_gpio_mode_input(int index)
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

int get_gpio_value(int index, int* value)
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

int get_topic_miami_monitor_value(int item, int* value)
{
	switch (item) {
	case TMM_MIAMI_VCC_mV:
		return divide_by(sysfile_som_vcc, value, 1000);
	case TMM_MIAMI_TEMP_mC:
		return read_sys_file_int(sysfile_som_temp, value);
	case TMM_CPU_CURRENT_mA:
		return divide_by(sysfile_cpu_current, value, 5);
	case TMM_FPGA_CURRENT_mA:
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
		return get_gpio_value(252+2, value);
	case TMM_DEBUGPRESENT:
		return get_gpio_value(252+3, value);
	}
	return -EINVAL;
}
