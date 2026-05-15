#pragma once
#include <stdint.h>

typedef void (*bme280_i2c_read)(uint8_t* buffer, uint16_t length);
typedef void (*bme280_i2c_write)(uint8_t* data, uint16_t size);

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write);
void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length);
void bme280_write_reg(uint8_t reg_address, uint8_t value);
float bme280_compensate_temperature();
float bme280_compensate_pressure(int32_t adc_P);
float bme280_compensate_humidity(int32_t adc_H);
uint16_t bme280_read_temp_raw();
uint16_t bme280_read_pres_raw();
uint16_t bme280_read_hum_raw();