#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "stdint.h"

const uint ADC_PIN = 26;
const uint ADC_CHAN_TEST = 0;
const uint ADC_CHAN_TEMP = 4;
const float REF_VOLTAGE = 3.3;

void adc_task_init() {
	adc_init();
	adc_gpio_init(ADC_PIN);
	adc_set_temp_sensor_enabled(true);
}
float measure_voltage() {
	adc_select_input(ADC_CHAN_TEST);
	uint16_t voltage_counts = adc_read();
	float voltage_V = REF_VOLTAGE / 4096 * voltage_counts;
	return voltage_V;
}
float measure_temperature() {
	adc_select_input(ADC_CHAN_TEMP);
	uint16_t temp_counts = adc_read();
	float temp_V = REF_VOLTAGE / 4096 * temp_counts;
	float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;
	return temp_C;
}