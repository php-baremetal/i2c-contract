/* The single include for an I2C device driver. */
#pragma once

#include "php.h"
#include "bm_i2c.h"
#include "i2c_driver_desc.h"

/* Device behind $this; throws and returns NULL if not initialised. */
i2c_dev_t *i2c_device_this(zval *zthis);

/* Driver __construct(Bus $bus, int $address = default, int $hz = default). */
void i2c_driver_ctor(INTERNAL_FUNCTION_PARAMETERS, const i2c_driver_desc_t *desc);
