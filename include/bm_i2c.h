/* I2C transport contract: what a device driver calls. Implemented by the platform's i2c transport. */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int bm_err_t;

#define BM_OK             0
#define BM_ERR_NOT_FOUND  0x105

/* Opaque device handle. */
typedef struct i2c_dev i2c_dev_t;

bm_err_t i2c_dev_read(i2c_dev_t *d, uint8_t *buf, size_t n);
bm_err_t i2c_dev_write(i2c_dev_t *d, const uint8_t *buf, size_t n);
bm_err_t i2c_dev_read_reg(i2c_dev_t *d, uint8_t reg, uint8_t *buf, size_t n);
bm_err_t i2c_dev_write_reg(i2c_dev_t *d, uint8_t reg, const uint8_t *buf, size_t n);
bm_err_t i2c_dev_write_reg1(i2c_dev_t *d, uint8_t reg, uint8_t val);

void bm_delay_ms(uint32_t ms);
