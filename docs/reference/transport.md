---
eyebrow: 'Package · Contract'
lede:    'bm_i2c.h: the five register-level transfers a driver uses to talk to its chip, the error codes they return, and a delay. Each call is one blocking transaction that locks the bus itself.'
see_also:
  - { href: '../overview.md', meta: '3 min' }
  - { href: './driver-descriptor.md', meta: '6 min' }
  - { href: '../recipes/write-a-driver.md', meta: '8 min' }
  - { href: 'package:php-baremetal/esp32-i2c-transport', meta: 'Transport', label: 'esp32-i2c-transport' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Driver descriptor', href: './driver-descriptor.md' }
---

# Transport calls

Everything in this page is declared in `bm_i2c.h` and implemented by the platform transport. A driver gets
it through `bm_i2c_driver.h`.

## Types and errors

| Name | Meaning |
| ---- | ------- |
| `i2c_dev_t` | Opaque handle to one device (a bus plus a 7-bit address). The driver receives it in `init`, in the poll `sample` hook, and from `i2c_device_this()`. |
| `bm_err_t` | `int`. `BM_OK` on success, otherwise the transport's own error code, returned unchanged. |
| `BM_OK` | `0`. |
| `BM_ERR_NOT_FOUND` | `0x105`. The code a driver returns from `init` when the chip answers but is not the expected one (wrong ID register). |

Test results against `BM_OK` only; do not compare against other numeric values, they differ per platform.
When `init` fails, the PHP constructor throws `cannot init <name> at 0x<addr>: <error name>`, where the
error name comes from the transport (`ESP_ERR_NOT_FOUND` for `BM_ERR_NOT_FOUND` on ESP32).

## Behaviour common to every call

- **Blocking.** The call returns when the transaction is done or has failed.
- **Self-locking.** The transport serialises transactions on a bus. If the PHP script opened the bus with
  `Bus::CORE1`, the transfer is handed to the bus owner task and waited for. The driver writes the same
  code either way.
- **One transaction per call.** A register read is a write of the register byte, a repeated start and the
  read. To keep a multi-byte value coherent, read it in one call rather than byte by byte.

## Calls

<!-- @method name="bm_err_t i2c_dev_read(i2c_dev_t *d, uint8_t *buf, size_t n)" returns="bm_err_t" -->

Reads `n` bytes from the device into `buf`, with no register byte first.

<!-- @params -->
<!-- @param name="d" type="i2c_dev_t *" required -->
The device.
<!-- @endparam -->
<!-- @param name="buf" type="uint8_t *" required -->
Destination, at least `n` bytes.
<!-- @endparam -->
<!-- @param name="n" type="size_t" required -->
Number of bytes to read.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="bm_err_t i2c_dev_write(i2c_dev_t *d, const uint8_t *buf, size_t n)" returns="bm_err_t" -->

Writes `n` bytes from `buf` as one transaction. Use it for chips driven by commands rather than registers.

<!-- @params -->
<!-- @param name="d" type="i2c_dev_t *" required -->
The device.
<!-- @endparam -->
<!-- @param name="buf" type="const uint8_t *" required -->
Bytes to send.
<!-- @endparam -->
<!-- @param name="n" type="size_t" required -->
Number of bytes.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="bm_err_t i2c_dev_read_reg(i2c_dev_t *d, uint8_t reg, uint8_t *buf, size_t n)" returns="bm_err_t" -->

Writes the register address `reg`, then reads `n` bytes. On chips with address auto-increment this reads
`n` consecutive registers.

<!-- @params -->
<!-- @param name="d" type="i2c_dev_t *" required -->
The device.
<!-- @endparam -->
<!-- @param name="reg" type="uint8_t" required -->
First register to read.
<!-- @endparam -->
<!-- @param name="buf" type="uint8_t *" required -->
Destination, at least `n` bytes.
<!-- @endparam -->
<!-- @param name="n" type="size_t" required -->
Number of bytes to read.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="bm_err_t i2c_dev_write_reg(i2c_dev_t *d, uint8_t reg, const uint8_t *buf, size_t n)" returns="bm_err_t" -->

Sends `reg` followed by the `n` bytes of `buf`, in one transaction.

<!-- @params -->
<!-- @param name="d" type="i2c_dev_t *" required -->
The device.
<!-- @endparam -->
<!-- @param name="reg" type="uint8_t" required -->
First register to write.
<!-- @endparam -->
<!-- @param name="buf" type="const uint8_t *" required -->
Register values.
<!-- @endparam -->
<!-- @param name="n" type="size_t" required -->
Number of bytes.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="bm_err_t i2c_dev_write_reg1(i2c_dev_t *d, uint8_t reg, uint8_t val)" returns="bm_err_t" -->

Writes one byte `val` to register `reg`. The common case during chip configuration.

<!-- @params -->
<!-- @param name="d" type="i2c_dev_t *" required -->
The device.
<!-- @endparam -->
<!-- @param name="reg" type="uint8_t" required -->
Register to write.
<!-- @endparam -->
<!-- @param name="val" type="uint8_t" required -->
Value.
<!-- @endparam -->
<!-- @endparams -->

<!-- @method name="void bm_delay_ms(uint32_t ms)" returns="void" -->

Waits `ms` milliseconds, letting other tasks run. Use it for the settle times a datasheet asks for after
reset or power-up.

<!-- @params -->
<!-- @param name="ms" type="uint32_t" required -->
Milliseconds to wait. The resolution is the platform's scheduler tick.
<!-- @endparam -->
<!-- @endparams -->

## Example

<!-- @code-block language="c" label="read a 16-bit big-endian register" -->
```c
static bm_err_t read_be16(i2c_dev_t *d, uint8_t reg, int16_t *out)
{
    uint8_t b[2];
    bm_err_t e = i2c_dev_read_reg(d, reg, b, 2);
    if (e == BM_OK) {
        *out = (int16_t) ((b[0] << 8) | b[1]);
    }
    return e;
}
```
<!-- @endcode-block -->
