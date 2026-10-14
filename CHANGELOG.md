# Changelog

## [v1.0.0]

First release.

### Added
- **Transport calls (`bm_i2c.h`).** The opaque device handle `i2c_dev_t` and the register-level transfers
  a driver uses: `i2c_dev_read()`, `i2c_dev_write()`, `i2c_dev_read_reg()`, `i2c_dev_write_reg()` and
  `i2c_dev_write_reg1()`, plus `bm_delay_ms()`. Each call is one blocking transaction that locks the bus
  itself. The platform's I2C transport implements them.
- **Error codes.** `bm_err_t` with `BM_OK` and `BM_ERR_NOT_FOUND`. Every other failure is the transport's
  own code, returned unchanged.
- **Driver descriptor (`i2c_driver_desc_t`).** It declares the PHP class (`Baremetal\I2c\Driver\<Class>`,
  extending `Device`), an optional capability interface, the default address and bus clock, the `init`
  hook, the method table, and the `poll` block that makes `poll()`, `sample()` and `drain()` work.
- **Driver helpers (`bm_i2c_driver.h`).** The single include a driver needs. It adds `i2c_device_this()`,
  which gets the device behind `$this`, and `i2c_driver_ctor()`, a complete
  `__construct(Bus $bus, int $address, int $hz)`.
