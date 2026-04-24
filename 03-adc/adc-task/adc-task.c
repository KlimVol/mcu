#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "stdint.h"
#include <stdio.h>
#include "adc-task.h"

const uint ADC_PIN = 26;
const uint ADC_CHAN_TEST = 0;
const uint ADC_CHAN_TEMP = 4;
const float REF_VOLTAGE = 3.3;

adc_task_state_t adc_task_state = ADC_TASK_STATE_IDLE;
uint64_t ADC_TASK_MEAS_PERIOD_US = 1000;
uint64_t adc_ts;

void adc_task_init() {
	adc_ts = 0;
	adc_init();
	adc_gpio_init(ADC_PIN);
	adc_set_temp_sensor_enabled(true);
}
void adc_task_state_set(adc_task_state_t state) {
	adc_task_state = state;
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
void adc_task_handle() {
	switch (adc_task_state)
	{
	case ADC_TASK_STATE_RUN:
		if (time_us_64() > adc_ts)
		{
			adc_ts = time_us_64() + (ADC_TASK_MEAS_PERIOD_US / 2);
			float voltage_V = measure_voltage();
			float temp_C = measure_temperature();
			printf("%f %f\n", voltage_V, temp_C);
			fflush(stdout);
		}
		break;
	case ADC_TASK_STATE_IDLE:
		break;
	default:
		break;
	}
}