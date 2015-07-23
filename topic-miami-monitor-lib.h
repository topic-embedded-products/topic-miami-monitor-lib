/*
 * topic-miami-monitor-lib.h
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

#define TMM_MIAMI_VCC_mV 0
#define TMM_MIAMI_TEMP_mC 1
#define TMM_CPU_CURRENT_mA 2
#define TMM_FPGA_CURRENT_mA 3

#define TMM_VCCO0_mV 4
#define TMM_VCCO1_mV 5
#define TMM_VCCO2_mV 6
#define TMM_VDDR_mV 7

#define TMM_VPRESENT 8
#define TMM_DEBUGPRESENT 9

/* Returns non-zero on error (as in errno.h). Result is stored
 * into "value" on success.
 */
int get_topic_miami_monitor_value(int item, int* value);
