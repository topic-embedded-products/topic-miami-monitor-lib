/*
 * topic-miami-monitor-example.c
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
