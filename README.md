# i2c-contract

This package is part of [php-baremetal](https://github.com/php-baremetal).

## What it is for

`i2c-contract` is the shared interface between I2C chip drivers and the platform code that drives the
bus. A driver written against it never touches a platform SDK: it reads and writes registers through a
small set of calls, and the I2C transport for the board supplies the implementation. The same driver
source therefore builds on every platform that has a transport.

It covers three things:

- **the transfers**: an opaque device handle, register and raw reads and writes, a delay, and the error
  codes they return;
- **the driver descriptor**: the one struct a driver exports to become a PHP class under
  `Baremetal\I2c\Driver\`, with its default address and clock, its init hook, its methods and an optional
  background sampler;
- **the constructor helpers**: the code every driver class shares to open the device and bind `$this`.

It is headers only: no code, and nothing it adds is visible from PHP.

## When to use it

Require it when you are **writing a driver package for an I2C chip**. Your package requires
`i2c-contract`, `ext-i2c` and the virtual `php-baremetal/i2c-transport`, and its C source includes
`bm_i2c_driver.h`. The full walk-through is in
[Write a driver package](docs/recipes/write-a-driver.md).

You do not need it to use I2C from PHP: a firmware project requires `ext-i2c`, the drivers for its chips
and the transport for its board, and they pull this package in for you.

## Headers

| Header | Defines |
| ------ | ------- |
| `bm_i2c_driver.h` | The one include a driver needs. Pulls in the two below plus `i2c_device_this()` and `i2c_driver_ctor()`. |
| `bm_i2c.h` | `bm_err_t`, `BM_OK`, `BM_ERR_NOT_FOUND`, the opaque `i2c_dev_t`, the `i2c_dev_*` calls and `bm_delay_ms()`. |
| `i2c_driver_desc.h` | `i2c_driver_desc_t`: name, PHP class, capability interface, default address and clock, `init`, methods, the poll block. |

## A driver in short

```c
#include "bm_i2c_driver.h"

static bm_err_t tmp102_init(i2c_dev_t *d)
{
    uint8_t cfg[2];
    return i2c_dev_read_reg(d, 0x01, cfg, 2);   /* fails if nothing answers */
}

const i2c_driver_desc_t tmp102_driver_desc = {
    .name = "tmp102", .php_class = "Tmp102",
    .default_addrs = { 0x48 }, .n_default_addrs = 1, .default_hz = 400000,
    .init = tmp102_init, .methods = tmp102_methods,
};
```

The firmware then has `new Baremetal\I2c\Driver\Tmp102($bus)`, running on any platform that has an I2C
transport.

## Documentation

- [Overview](docs/overview.md): where the contract sits, what a driver package contains.
- [Transport calls](docs/reference/transport.md): `i2c_dev_*`, errors, `bm_delay_ms()`.
- [Driver descriptor](docs/reference/driver-descriptor.md): every field of `i2c_driver_desc_t` and the constructor helper.
- [Write a driver package](docs/recipes/write-a-driver.md): a complete driver, from `composer.json` to PHP.

## AI usage

AI is used as a tool here, on top of human engineering. The contract design, the architecture and the
hardware bring-up are done by hand. AI mostly helps with the documentation and with mechanical work that
can be verified. The full policy, including what is expected of contributions made with AI, is in
**[AI_USAGE.md](AI_USAGE.md)**.

Not for you? No worries at all. Nobody is forcing it on you, and a free, open-source project you didn't
spend a minute or a cent on owes you no explanation anyway. Take it or leave it, no hard feelings.

And for everyone who's on board: when contributing, please follow the guidelines in
[AI_USAGE.md](AI_USAGE.md). Thanks!

## License

MIT.
