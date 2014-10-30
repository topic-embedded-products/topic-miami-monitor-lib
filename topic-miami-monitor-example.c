#include "topic-miami-monitor-lib.h"
#include <string.h>
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
	if (result != 0)
		printf("%16s: ERROR %d: %s\n", m->name, result, strerror(-result));
	else if (m->unit)
		printf("%16s: %5d %s\n", m->name, value, m->unit);
	else
		printf("%16s: %5s\n", m->name, value ? "YES" : "NO");
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
	{ TMM_VPRESENT, "Carrier board", NULL },
	{ TMM_DEBUGPRESENT, "Debug board", NULL },
};

int main()
{
	int i;

	for (i = 0; i < sizeof(items)/sizeof(items[0]); ++i)
		show_item(&items[i]);
	
	return 0;
}
