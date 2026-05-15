#include "pico/stdio.h"
#include "stdio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "hardware/gpio.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "mem-task/mem-task.h"
#include "ili9341-driver.h"
#include "hardware/spi.h"
#include "ili9341-display.h"
#include "ili9341-font.h"


#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"
#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9
// #define PIN_LED -> 3.3V

void rp2040_spi_write(const uint8_t* data, uint32_t size)
{
	spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t* buffer, uint32_t length)
{
	spi_read_blocking(spi0, 0, buffer, length);
}
void rp2040_gpio_cs_write(bool level)
{
	gpio_put(ILI9341_PIN_CS, level);
}
void rp2040_gpio_dc_write(bool level)
{
	gpio_put(ILI9341_PIN_DC, level);
}
void rp2040_gpio_reset_write(bool level)
{
	gpio_put(ILI9341_PIN_RESET, level);
}
void rp2040_delay_ms(uint32_t ms)
{
	sleep_ms(ms);
}

uint32_t global_variable = 0;

const uint32_t constant_variable = 42;

static ili9341_display_t ili9341_display = { 0 };

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
	int scanned = sscanf(args, "%lx %lx", &addr, &value);

	if (scanned == 2) {
		wmem(addr, value);
	}
	else {
	}
}
void disp_screen_callback(const char* args)
{
	uint32_t c = 0;
	int result = sscanf(args, "%x", &c);

	uint16_t color = COLOR_BLACK;

	if (result == 1)
	{
		color = RGB888_2_RGB565(c);
		printf("Display painted with color 0x%04X\n", color);
	}

	ili9341_fill_screen(&ili9341_display, color);
}
void disp_px_callback(const char* args)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t color = 0;

	int result = sscanf(args, "%u %u %i", &x, &y, &color);

	if (result == 3) {
		uint16_t color_565 = RGB888_2_RGB565(color);

		ili9341_draw_pixel(&ili9341_display, (uint16_t)x, (uint16_t)y, color_565);

	}
}

void disp_line_callback(const char* args)
{
	uint32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0, c = 0;
	int result = sscanf(args, "%u %u %u %u %i", &x0, &y0, &x1, &y1, &c);

	if (result == 5) {
		uint16_t color = RGB888_2_RGB565(c);
		ili9341_draw_line(&ili9341_display, (uint16_t)x0, (uint16_t)y0, (uint16_t)x1, (uint16_t)y1, color);
		printf("Line drawn from (%u,%u) to (%u,%u), color: 0x%04X\n", x0, y0, x1, y1, color);
	}
}

void disp_rect_callback(const char* args)
{
	uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
	int result = sscanf(args, "%u %u %u %u %i", &x, &y, &w, &h, &c);

	if (result == 5) {
		uint16_t color = RGB888_2_RGB565(c);
		ili9341_draw_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
		printf("Rectangle outline drawn at (%u,%u) size %ux%u, color: 0x%04X\n", x, y, w, h, color);
	}
}

void disp_frect_callback(const char* args)
{
	uint32_t x = 0, y = 0, w = 0, h = 0, c = 0;
	int result = sscanf(args, "%u %u %u %u %i", &x, &y, &w, &h, &c);

	if (result == 5) {
		uint16_t color = RGB888_2_RGB565(c);
		ili9341_draw_filled_rect(&ili9341_display, (uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, color);
		printf("Filled rectangle drawn at (%u,%u) size %ux%u, color: 0x%04X\n", x, y, w, h, color);
	}
}

void disp_text_callback(const char* args)
{
	uint32_t x = 0, y = 0, text_color_rgb = 0, bg_color_rgb = 0;
	char text_buffer[128] = { 0 };
	int result = sscanf(args, "%u %u %i %i %127[^\r\n]", &x, &y, &text_color_rgb, &bg_color_rgb, text_buffer);

	if (result == 5) {
		uint16_t fg_color = RGB888_2_RGB565(text_color_rgb);
		uint16_t bg_color = RGB888_2_RGB565(bg_color_rgb);

		ili9341_draw_text(&ili9341_display, (uint16_t)x, (uint16_t)y, text_buffer, &jetbrains_font, fg_color, bg_color);
		printf("Text '%s' printed at (%u,%u)\n", text_buffer, x, y);
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
	{"disp_screen", disp_screen_callback, "paint display"},
	{"disp_px", disp_px_callback, "paint pixel"},
	{"disp_line", disp_line_callback, "draw line>"},
	{"disp_rect", disp_rect_callback, "draw rect"},
	{"disp_frect", disp_frect_callback, "draw filled rect"},
	{"disp_text", disp_text_callback, "display text"},
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

	gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
	gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
	gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);

	gpio_init(ILI9341_PIN_CS);
	gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
	gpio_init(ILI9341_PIN_DC);
	gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
	gpio_init(ILI9341_PIN_RESET);
	gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);

	gpio_put(ILI9341_PIN_CS, 1);
	gpio_put(ILI9341_PIN_DC, 0);
	gpio_put(ILI9341_PIN_RESET, 0);

	spi_init(spi0, 10 * 1000 * 1000);

	ili9341_hal_t ili9341_hal = { 0 };
	ili9341_hal.spi_write = rp2040_spi_write;
	ili9341_hal.spi_read = rp2040_spi_read;
	ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
	ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
	ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
	ili9341_hal.delay_ms = rp2040_delay_ms;

	ili9341_init(&ili9341_display, &ili9341_hal);
	ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);
	
	ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
	sleep_ms(300);
	/* 2. Coloured rectangles */
	ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
	ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
	ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);
	/* 3. Hollow rectangle outline */
	ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);
	/* 4. Diagonal lines */
	ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
	ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);

	ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);

	ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);
	while (1) {
		protocol_task_handle(stdio_task_handle());
		led_task_handle();
	}
}
