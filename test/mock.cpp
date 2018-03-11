unsigned long sdk_current_time = 10 * 1000000;

unsigned long sdk_system_get_time()
{
	return sdk_current_time;
}

void vTaskDelay(int xTicksToDelay) {}
void logInfo(const char * msg, ...) {}
