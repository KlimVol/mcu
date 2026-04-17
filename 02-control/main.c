#include "pico/stdio.h"
#include "stdio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-task/mem-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

const uint32_t constant_variable = 42;

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}
void led_on_callback(const char* args)
{
	printf("Led turned on\n");
	led_task_state_set(LED_STATE_ON);
}
void led_off_callback(const char* args)
{
	printf("Led turned off\n");
	led_task_state_set(LED_STATE_OFF);
}
void led_blink_callback(const char* args)
{
	printf("Led is blinking now\n");
	led_task_state_set(LED_STATE_BLINK);
}
void led_blink_set_period_ms_callback(const char* args) {
	uint period_ms = 0;
	sscanf(args, "%u", &period_ms);
	if (period_ms == 0) {
		printf("Invalid time\n");
		return;
	}
	led_task_state_blink_period(period_ms);
}
void mem_callback(const char* args)
{
	uint32_t addr;
	sscanf(args, "%i", &addr);
	mem(addr);
}
void wmem_callback(const char* args) {
	uint32_t addr = 0, value = 0;
	// Используем %lx (long hex), так как uint32_t на Pico часто приравнен к unsigned long
	int scanned = sscanf(args, "%lx %lx", &addr, &value);

	if (scanned == 2) {
		wmem(addr, value);
	}
	else {
	}
}
void help_callback(const char* args);
api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
	{"on",      led_on_callback,  "turned led on"},
	{"off",     led_off_callback, "turned led off"},
	{"blink",   led_blink_callback, "led is blinking now"},
	{"set_period", led_blink_set_period_ms_callback, "blinking period changed"},
	{"mem", mem_callback, "Address value printed"},
	{"wmem", wmem_callback, "Addres value changed"},
	{"help", help_callback, "print commands with descriptions"},
	{NULL, NULL, NULL}
};
void help_callback(const char* args) {
	printf("Доступные команды:\n");
	for (int i = 0; device_api[i].command_name != NULL; i++) {
		printf("Команда: %s : %s\n",
			device_api[i].command_name,
			device_api[i].command_help);
	}
}

int main()
{
	stdio_init_all();
	led_task_init();
	stdio_task_init();
	protocol_task_init(device_api);
	while (1) {
		protocol_task_handle(stdio_task_handle());
		led_task_handle();
	}
}