#define TMM_MIAMI_VCC_mV 0
#define TMM_MIAMI_TEMP_mC 1
#define TMM_CPU_CURRENT_mA 2
#define TMM_FPGA_CURRENT_mA 3

/* Returns non-zero on error (as in errno.h). Result is stored
 * into "value" on success.
 */
int get_topic_miami_monitor_value(int item, int* value);
