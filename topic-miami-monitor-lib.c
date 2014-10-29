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
	}
	return -EINVAL;
}
