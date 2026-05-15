#include "pico/stdio.h"
#include "stdio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "mem-task/mem-task.h"
#include "bme280-driver.h"
#include "hardware/i2c.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

const uint32_t constant_variable = 42;

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
	i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}
void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
	i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

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
void read_reg_callback(const char* args) {
	uint32_t addr = 0;
	uint32_t n = 0;

	int scanned = sscanf(args, "%x %x", &addr, &n);

	if (scanned == 2) {
		if (addr <= 0xFF && n <= 0xFF && (addr + n) <= 0x100) {
			uint8_t buffer[256] = { 0 };
			bme280_read_regs((uint8_t)addr, buffer, (uint8_t)n);

			for (uint32_t i = 0; i < n; i++) {
				printf("Reg 0x%02X: 0x%02X\n", (uint8_t)(addr + i), buffer[i]);
			}
		}
		else {
			printf("Неверно указан адрес или количество\n");
		}
	}
	else {
		printf("Использование: read_reg <addr_hex> <n_dec>\n");
	}
}
void write_reg_callback(const char* args) {
	uint32_t addr = 0;
	uint32_t val = 0; 

	int scanned = sscanf(args, "%x %x", &addr, &val);

	if (scanned == 2) {
		if (addr <= 0xFF && val <= 0xFF) {
			bme280_write_reg((uint8_t)addr, (uint8_t)val);
			printf("Записан 0x%02X: 0x%02X done\n", addr, val);
		}
		else {
			printf("Ошибка: адрес или значение выходят ха диапазон\n");
		}
	}
	else {
		printf("Использование: write_reg <addr_hex> <val_hex>\n");
	}
}
void temp_raw_callback(const char* args) {
	uint16_t raw = bme280_read_temp_raw();
	printf("Температура в отсчётах: %u\n", raw);
}

void pres_raw_callback(const char* args) {
	uint16_t raw = bme280_read_pres_raw();
	printf("Давление в отсчётах: %u\n", raw);
}

void hum_raw_callback(const char* args) {
	uint16_t raw = bme280_read_hum_raw();
	printf("Влажность в отсчётах: %u\n", raw);
}
void temp_callback(const char* args) {
	float t = bme280_compensate_temperature();
	printf("%.2f\n", t);
}
void pres_callback(const char* args) {
	bme280_compensate_temperature();
	uint16_t raw_p = bme280_read_pres_raw();
	float p = bme280_compensate_pressure((int32_t)raw_p << 4);
	printf("%.2f\n", p);
}
void hum_callback(const char* args) {
	bme280_compensate_temperature();
	uint16_t raw_h = bme280_read_hum_raw();
	float h = bme280_compensate_humidity((int32_t)raw_h);
	printf("%.2f\n", h);
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
	{"read_reg", read_reg_callback, "Read registers"},
	{"write_reg", write_reg_callback, "Write registers"},
	{"temp_raw", temp_raw_callback, "Raw temperature data"},
	{"pres_raw", pres_raw_callback, "Raw pressure data"},
	{"hum_raw", hum_raw_callback, "Raw humidity data"},
	{"temp", temp_callback, "Temperature data"},
	{"pres", pres_callback, "Pressure data"},
	{"hum", hum_callback, "Humidity data"},
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
	i2c_init(i2c1, 100000);
	gpio_set_function(14, GPIO_FUNC_I2C);
	gpio_set_function(15, GPIO_FUNC_I2C);
	bme280_init(rp2040_i2c_read, rp2040_i2c_write);
	stdio_init_all();
	led_task_init();
	stdio_task_init();
	protocol_task_init(device_api);
	while (1) {
		protocol_task_handle(stdio_task_handle());
		led_task_handle();
	}
}