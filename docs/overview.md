---
eyebrow: 'Package · Contract'
lede:    'The headers an I2C device driver is written against: register-level transfers, error codes and the descriptor that makes the chip a PHP class. Headers only; the platform transport supplies the code.'
see_also:
  - { href: './reference/transport.md', meta: '4 min' }
  - { href: './reference/driver-descriptor.md', meta: '6 min' }
  - { href: './recipes/write-a-driver.md', meta: '8 min' }
  - { href: 'package:php-baremetal/ext-i2c#docs/overview.md', meta: 'Component', label: 'ext-i2c' }
  - { href: 'package:php-baremetal/imu-qmi8658#docs/overview.md', meta: 'Driver', label: 'imu-qmi8658' }
  - { href: 'package:php-baremetal/esp32-i2c-transport', meta: 'Transport', label: 'esp32-i2c-transport' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'Transport calls', href: './reference/transport.md' }
---

# I2C driver contract

`i2c-contract` is the shared interface between I2C chip drivers and the platform code that drives the
bus. A driver written against it talks to its chip only through `i2c_dev_*` calls, so the same source
builds on every platform that has an I2C transport, and it ends up in PHP as a class under
`Baremetal\I2c\Driver\`.

## What it covers

| Area | Header | Reference |
| ---- | ------ | --------- |
| Transfers: device handle, register and raw reads and writes, delay, error codes | `bm_i2c.h` | [Transport calls](./reference/transport.md) |
| The descriptor a driver exports to become a PHP class | `i2c_driver_desc.h` | [Driver descriptor](./reference/driver-descriptor.md) |
| Constructor helpers shared by every driver class | `bm_i2c_driver.h` | [Driver descriptor](./reference/driver-descriptor.md) |

Headers only: no code, and nothing it adds is visible from PHP.

## When to use it

Use it when you are **writing a driver package for an I2C chip**; the walk-through is
[Write a driver package](./recipes/write-a-driver.md).

To use a chip from PHP you do not need it: read the driver's own documentation and
[ext-i2c](package:php-baremetal/ext-i2c#docs/overview.md). Your project's packages pull the contract in.

## Where it sits

| Package | Role |
| ------- | ---- |
| `i2c-contract` | These headers. No code. |
| `ext-i2c` | The PHP side: `Bus`, `Device`, the driver classes and the constructor helper. |
| a transport, e.g. `esp32-i2c-transport` | The code behind `i2c_dev_*` and `bm_delay_ms()` for one platform. Provides the virtual `php-baremetal/i2c-transport`. |
| a driver, e.g. `imu-qmi8658` | One chip: a descriptor plus its PHP methods. |

The firmware project picks the transport for its board. A driver never names one: it requires the virtual
`php-baremetal/i2c-transport`, and whichever provider the project installed satisfies it.

## Install

A project does not require the contract. A **driver package** does, together with `ext-i2c` and the
transport virtual:

<!-- @code-block language="json" label="composer.json — a driver package" -->
```js
{
    "require": {
        "php": ">=8.3",
        "php-baremetal/i2c-contract": "^1.0",
        "php-baremetal/ext-i2c": "^1.0",
        "php-baremetal/i2c-transport": "^1.0"
    }
}
```
<!-- @endcode-block -->

The full package layout is in [Write a driver package](./recipes/write-a-driver.md).

## Includes

A driver includes only `bm_i2c_driver.h`, which pulls in the other two headers, plus `zend_exceptions.h`
or `math.h` when it needs them. It never includes a platform SDK header.

## What a driver becomes in PHP

For a descriptor with `.php_class = "Tmp102"` the firmware gets:

- the class `Baremetal\I2c\Driver\Tmp102`, extending `Baremetal\I2c\Device`, so it also has `probe()`,
  `read()`, `write()`, `readReg()`, `writeReg()`, `poll()`, `sample()` and `drain()`;
- the methods listed in `.methods`;
- the interface named in `.capability`, if set, so PHP code can type-hint `Baremetal\Sensor\Imu` instead of
  a chip.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\I2c\Bus;
use Baremetal\I2c\Driver\Tmp102;

$t = new Tmp102(new Bus(sda: 11, scl: 10));   // address and speed from the descriptor
echo $t->celsius(), "\n";
```
<!-- @endcode-block -->

## Notes

- The device handle is opaque: a driver cannot see the bus, the address or the platform handle.
- A device is keyed by bus and address. The first object created for an address fixes its driver and
  speed, and `init` runs only then.
- `i2c_dev_*` calls lock the bus themselves; a driver never takes a lock.
