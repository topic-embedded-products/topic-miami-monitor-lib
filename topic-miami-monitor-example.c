#include "topic-miami-monitor-lib.h"
#include <error.h>
#include <stdio.h>

struct monitor_item {
	int item;
	const char* name;
	const char* unit;
};

static void show_item(const struct monitor_item* m)
{
	int result;
	int value;

	result = get_topic_miami_monitor_value(m->item, &value);
	if (result != 0) {
		error(0, -result, "%s: ERROR %d", m->name, result);
		return;
	}
	printf("%s: %d %s\n", m->name, value, m->unit);
}

const struct monitor_item items[] = {
	{ TMM_MIAMI_VCC_mV, "Miami Vcc power", "mV"},
	{ TMM_MIAMI_TEMP_mC, "Miami board temp", "mC"},
	{ TMM_CPU_CURRENT_mA, "CPU current", "mA" },
	{ TMM_FPGA_CURRENT_mA, "FPGA current", "mA" },
	{ TMM_VCCO0_mV, "VCCO0", "mV" },
	{ TMM_VCCO1_mV, "VCCO1", "mV" },
	{ TMM_VCCO2_mV, "VCCO2", "mV" },
	{ TMM_VDDR_mV, "VDDR", "mV" },
};

int main()
{
	int i;

	for (i = 0; i < sizeof(items)/sizeof(items[0]); ++i)
		show_item(&items[i]);
	
	return 0;
}
