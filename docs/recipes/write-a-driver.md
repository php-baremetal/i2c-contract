---
eyebrow: 'Package · Contract'
lede:    'A complete I2C driver package for the TMP102 temperature sensor: composer.json, one C file, and the PHP class it gives the firmware. Copy it as the starting point for your own chip.'
see_also:
  - { href: '../reference/driver-descriptor.md', meta: '6 min' }
  - { href: '../reference/transport.md', meta: '4 min' }
  - { href: 'package:php-baremetal/imu-qmi8658#docs/overview.md', meta: 'Driver', label: 'imu-qmi8658 (a real driver)' }
  - { href: 'package:php-baremetal/ext-i2c#docs/overview.md', meta: 'Component', label: 'ext-i2c' }
prev: { label: 'Driver descriptor', href: '../reference/driver-descriptor.md' }
next: { label: 'No next page', href: '#' }
---

# Write a driver package

## Goal

A package `acme/temp-tmp102` that adds `Baremetal\I2c\Driver\Tmp102` to the firmware, with a `celsius()`
method and background sampling. The TMP102 answers at `0x48` (up to `0x4B`, depending on its `ADD0` pin)
and keeps the temperature in register `0x00` as a 12-bit value, 0.0625 °C per step.

## Layout

<!-- @code-block language="text" label="package tree" -->
```text
temp-tmp102/
  composer.json
  src/
    tmp102.c
  README.md
```
<!-- @endcode-block -->

Every `.c` file in the source directory is compiled into the firmware. One package may hold several
drivers, but each needs its own descriptor.

## composer.json

<!-- @code-block language="json" label="composer.json" -->
```js
{
    "name": "acme/temp-tmp102",
    "description": "TMP102 temperature sensor driver for php-baremetal.",
    "version": "1.0.0",
    "type": "bm-driver",
    "license": "MIT",
    "require": {
        "php": ">=8.3",
        "php-baremetal/i2c-contract": "^1.0",
        "php-baremetal/ext-i2c": "^1.0",
        "php-baremetal/i2c-transport": "^1.0"
    },
    "extra": {
        "baremetal": {
            "kind": "driver",
            "bus": "i2c",
            "driver": "tmp102",
            "php-class": "Tmp102",
            "capability": "Baremetal\\Sensor\\Thermometer",
            "src-dir": "src"
        }
    }
}
```
<!-- @endcode-block -->

| Key | Meaning |
| --- | ------- |
| `type`, `kind` | `bm-driver` and `driver`: the build treats the package as a driver. |
| `bus` | `i2c`: the descriptor goes into the I2C driver table. |
| `driver` | Must equal the descriptor's `.name`; the descriptor symbol is `<driver>_driver_desc`. Without it the tooling reports the package as broken. |
| `php-class`, `capability` | Same values as in the descriptor, shown by the tooling and on the registry. |
| `src-dir` | Directory with the C sources. Defaults to `src`. |
| `php-baremetal/i2c-transport` | The virtual every I2C transport provides. Never require a concrete transport: the project chooses it. |

## src/tmp102.c

<!-- @code-block language="c" label="src/tmp102.c" -->
```c
/* TMP102 temperature sensor. celsius() in degC; poll block of 2 raw bytes. */
#ifdef PHP_I2C_BUILD
#include "bm_i2c_driver.h"
#include "zend_exceptions.h"

#define TMP_TEMP   0x00
#define TMP_CONFIG 0x01

extern const i2c_driver_desc_t tmp102_driver_desc;

static double to_celsius(const uint8_t *b)
{
    int16_t raw = (int16_t) ((b[0] << 8) | b[1]) >> 4;
    return raw * 0.0625;
}

static bm_err_t tmp102_init(i2c_dev_t *d)
{
    uint8_t cfg[2];
    return i2c_dev_read_reg(d, TMP_CONFIG, cfg, 2);
}

static bm_err_t tmp102_sample(i2c_dev_t *d, void *out)
{
    return i2c_dev_read_reg(d, TMP_TEMP, out, 2);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmp_ctor, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, bus, Baremetal\\I2c\\Bus, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, address, IS_LONG, 0, "0x48")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hz, IS_LONG, 0, "400000")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tmp_celsius, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tmp_decode, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, raw, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Tmp102, __construct)
{
    i2c_driver_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &tmp102_driver_desc);
}

PHP_METHOD(Tmp102, celsius)
{
    ZEND_PARSE_PARAMETERS_NONE();
    i2c_dev_t *d = i2c_device_this(ZEND_THIS);
    if (!d) {
        RETURN_THROWS();
    }
    uint8_t b[2];
    if (i2c_dev_read_reg(d, TMP_TEMP, b, 2) != BM_OK) {
        zend_throw_exception(zend_ce_exception, "TMP102 read failed", 0);
        RETURN_THROWS();
    }
    RETURN_DOUBLE(to_celsius(b));
}

/* decode(string $raw): float -- one record from sample()/drain(). */
PHP_METHOD(Tmp102, decode)
{
    zend_string *raw;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(raw)
    ZEND_PARSE_PARAMETERS_END();
    if (ZSTR_LEN(raw) < 2) {
        zend_argument_value_error(1, "a TMP102 sample is 2 bytes");
        RETURN_THROWS();
    }
    RETURN_DOUBLE(to_celsius((const uint8_t *) ZSTR_VAL(raw)));
}

static const zend_function_entry tmp102_methods[] = {
    PHP_ME(Tmp102, __construct, arginfo_tmp_ctor,    ZEND_ACC_PUBLIC)
    PHP_ME(Tmp102, celsius,     arginfo_tmp_celsius, ZEND_ACC_PUBLIC)
    PHP_ME(Tmp102, decode,      arginfo_tmp_decode,  ZEND_ACC_PUBLIC)
    PHP_FE_END
};

const i2c_driver_desc_t tmp102_driver_desc = {
    .name            = "tmp102",
    .php_class       = "Tmp102",
    .capability      = "Baremetal\\Sensor\\Thermometer",
    .default_addrs   = { 0x48, 0x49, 0x4A, 0x4B },
    .n_default_addrs = 4,
    .default_hz      = 400000,
    .init            = tmp102_init,
    .methods         = tmp102_methods,
    .poll            = { .sample = tmp102_sample, .sample_size = 2, .max_hz = 30 },
};
#endif
```
<!-- @endcode-block -->

<!-- @callout variant="tip" title="Why the #ifdef" -->
Driver sources are compiled as part of `ext-i2c`, which defines `PHP_I2C_BUILD`. The guard keeps the file
empty in any other compilation, and it is the convention every php-baremetal driver follows.
<!-- @endcallout -->

## Use it from a project

<!-- @steps -->
- **Require it**

  In the firmware project, next to `ext-i2c` and the project's transport:
  `composer require acme/temp-tmp102`. While developing, point a `path` repository at your checkout.

- **Build and flash**

  The build finds the driver through its `extra.baremetal` block and adds the class.

- **Call it from PHP**

  Use the script below.
<!-- @endsteps -->

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Tmp102;

$bus = new Bus(sda: 11, scl: 10);
$t   = new Tmp102($bus);              // 0x48, 400 kHz
printf("%.2f C\n", $t->celsius());

$t->poll(4);                          // 4 records per second in the background
while (true) {
    foreach ($t->drain() as $raw) {
        printf("%.2f C\n", $t->decode($raw));
    }
    usleep(1_000_000);
}
```
<!-- @endcode-block -->

## Check list

- `extra.baremetal.driver`, `.name` and the `<name>_driver_desc` symbol all match.
- The descriptor is a non-`static` global; everything else in the file is `static`.
- `init` returns `BM_ERR_NOT_FOUND` when an ID register holds the wrong value, so a wrong chip fails at
  construction instead of returning garbage later.
- `poll.sample` only calls `i2c_dev_*`: it runs outside the PHP thread.
- No platform SDK header is included.
