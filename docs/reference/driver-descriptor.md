---
eyebrow: 'Package · Contract'
lede:    'i2c_driver_desc_t is the one struct a driver exports: it names the PHP class, the default address and clock, the init hook, the methods and the background sampler. Fill what you need and leave the rest zeroed.'
see_also:
  - { href: '../overview.md', meta: '3 min' }
  - { href: './transport.md', meta: '4 min' }
  - { href: '../recipes/write-a-driver.md', meta: '8 min' }
  - { href: 'package:php-baremetal/ext-i2c#docs/usage/php-api.md', meta: 'Component', label: 'ext-i2c PHP API' }
prev: { label: 'Transport calls', href: './transport.md' }
next: { label: 'Write a driver package', href: '../recipes/write-a-driver.md' }
---

# Driver descriptor

Each driver defines exactly one descriptor, as a global `const` named `<driver>_driver_desc`, where
`<driver>` is the `extra.baremetal.driver` value of its `composer.json`. The build collects the descriptors
of every driver the project installed into the `i2c_drivers[]` table; `ext-i2c` reads that table at startup
and registers one PHP class per entry.

<!-- @code-block language="c" label="src/tmp102.c" -->
```c
const i2c_driver_desc_t tmp102_driver_desc = {
    .name            = "tmp102",
    .php_class       = "Tmp102",
    .capability      = "Baremetal\\Sensor\\Thermometer",
    .default_addrs   = { 0x48 },
    .n_default_addrs = 1,
    .default_hz      = 400000,
    .init            = tmp102_init,
    .methods         = tmp102_methods,
    .poll            = { .sample = tmp102_sample, .sample_size = 2 },
};
```
<!-- @endcode-block -->

## Identity

| Field | Type | Meaning |
| ----- | ---- | ------- |
| `name` | `const char *` | Short lowercase name, the same as `extra.baremetal.driver`. Appears in error messages. |
| `php_class` | `const char *` | Class name registered as `Baremetal\I2c\Driver\<php_class>`, extending `Baremetal\I2c\Device`. `NULL` registers no class. |
| `capability` | `const char *` | Fully qualified interface the class implements, or `NULL`. If no such interface exists yet it is created, empty. |

`ext-i2c` always defines `Baremetal\Sensor\Imu`, `Baremetal\Sensor\Magnetometer` and
`Baremetal\Input\Touch`. Reuse one of them when it fits, so PHP code written for one chip works with
another. A `capability` naming an existing class that is not an interface is ignored.

## Address and clock

| Field | Type | Meaning |
| ----- | ---- | ------- |
| `default_addrs` | `uint8_t[4]` | 7-bit addresses the chip can have. The first one is the constructor default. |
| `n_default_addrs` | `uint8_t` | How many entries of `default_addrs` are valid. `0` makes the default address `0`. |
| `default_hz` | `uint32_t` | Constructor default for the bus clock. `0` means 100 kHz. |

Only `default_addrs[0]` is used as a default. List the alternatives anyway, as documentation for users,
and say in the driver docs that those addresses must be passed explicitly.

## Lifecycle

| Field | Type | Meaning |
| ----- | ---- | ------- |
| `init` | `bm_err_t (*)(i2c_dev_t *)` | Called once, when the first PHP object for that bus and address is created. Check the chip ID and configure it. A result other than `BM_OK` makes the constructor throw. `NULL` skips it. |
| `deinit` | `void (*)(i2c_dev_t *)` | Not called: devices live as long as the firmware. Leave `NULL`. |
| `state_size` | `size_t` | Leave `0`. The contract gives no way to reach per-instance state from the handle. |

## PHP methods

| Field | Type | Meaning |
| ----- | ---- | ------- |
| `methods` | `const zend_function_entry *` | The class's method table, ending with `PHP_FE_END`. It must contain `__construct`. |

Write each method with `PHP_METHOD`, get the device with `i2c_device_this(ZEND_THIS)`, and make the
constructor a one-line call to `i2c_driver_ctor()`.

<!-- @method name="i2c_dev_t *i2c_device_this(zval *zthis)" returns="i2c_dev_t *" -->

Returns the device behind `$this`. If the object was never initialised it throws `I2c\Device is not
initialised` and returns `NULL`; the method then just does `RETURN_THROWS()`.

<!-- @params -->
<!-- @param name="zthis" type="zval *" required -->
Always `ZEND_THIS`.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="void i2c_driver_ctor(INTERNAL_FUNCTION_PARAMETERS, const i2c_driver_desc_t *desc)" returns="void" -->

The whole constructor. It parses `(Baremetal\I2c\Bus $bus, int $address = <default_addrs[0]>, int $hz =
<default_hz>)`, opens the device on the bus, runs `init` if this is the first object for that address, and
binds the PHP object to the device. On failure it throws `cannot init <name> at 0x<addr>: <error>`.

<!-- @params -->
<!-- @param name="INTERNAL_FUNCTION_PARAMETERS" type="macro" required -->
Pass `INTERNAL_FUNCTION_PARAM_PASSTHRU`.
<!-- @endparam -->
<!-- @param name="desc" type="const i2c_driver_desc_t *" required -->
The driver's own descriptor.
<!-- @endparam -->
<!-- @endparams -->

The constructor's `arginfo` is the driver's own: declare the same three parameters, with your defaults,
so reflection and error messages match.

## Background sampling: `poll`

Fill this block and the class's inherited `poll()`, `sample()` and `drain()` start working: PHP asks for a
rate, a background task calls `sample` at that rate, and the script collects raw records.

| Field | Type | Meaning |
| ----- | ---- | ------- |
| `poll.sample` | `bm_err_t (*)(i2c_dev_t *, void *out)` | Reads one record into `out`. Called from the background task, so use only `i2c_dev_*` calls and touch no PHP values. |
| `poll.sample_size` | `size_t` | Size in bytes of one record. `sample()` returns a string of this length and `drain()` an array of them. |
| `poll.max_hz` | `uint32_t` | Highest sensible rate, for documentation. It is not enforced. |

With `sample` or `sample_size` left at zero, `poll()` throws `this device has no poller (driver has no
poll block)`. Records are raw bytes: give the class a `decode(string $raw): array` method so PHP code does
not unpack them by hand.

## Unused blocks

`fifo`, `block`, `irq`, `power`, `events` and `n_events` are not read by `ext-i2c`. Leave them zeroed;
designated initialisers do that for you.
