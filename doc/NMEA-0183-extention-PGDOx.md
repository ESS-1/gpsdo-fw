# PGDOS NMEA-0183 frame

The GPSDO device sends a `$PGDOS` NMEA-0183 frame to the PC Communication Port about once per second. The frame lets PC software monitor the current GPSDO state, GPS lock status, frequency-control data, and communication error counters.

## Frame format

```text
$PGDOS,<STATE>,<UPTIME>,<NUM_SATS>,<ERR_MEAN>,<ERR_INST>,<PWM>,<COMM_ERRORS>*CS\r\n
```

`CS` is the NMEA checksum: an 8-bit XOR of all characters between `$` and `*`, encoded as two uppercase hexadecimal characters.

## Fields

| Field | Size | Description |
| --- | ---: | --- |
| `STATE` | 2 chars | Device and GPS state. |
| `UPTIME` | 8 chars | Device uptime in seconds, encoded as a 32-bit unsigned integer in uppercase hex. |
| `NUM_SATS` | 2 chars | Number of satellites, encoded as an 8-bit unsigned integer in uppercase hex. |
| `ERR_MEAN` | 8 chars | Mean frequency error in ppb/100, encoded as a 32-bit signed integer in uppercase hex. `7FFFFFFF` means unset. |
| `ERR_INST` | 8 chars | Last calculated frequency error in ppb/100, encoded as a 32-bit signed integer in uppercase hex. `7FFFFFFF` means unset. |
| `PWM` | 4 chars | OCXO control PWM value, encoded as a 16-bit unsigned integer in uppercase hex. |
| `COMM_ERRORS` | 6 chars | Three 8-bit communication error counters in uppercase hex. |

## STATE

`STATE` consists of two characters:

| Character | Meaning |
| --- | --- |
| char 1 | Device state. |
| char 2 | GPS state. |

Device state values:

| Value | Meaning |
| --- | --- |
| `+` | Normal operation. |
| `W` | Warmup. |
| `F` | Error or fault. |

GPS state values:

| Value | Meaning |
| --- | --- |
| `+` | GPS lock. |
| `N` | No GPS lock. |
| `F` | Error or fault. |

## COMM_ERRORS

`COMM_ERRORS` is encoded as three consecutive 8-bit counters:

| Characters | Counter |
| --- | --- |
| 1-2 | Number of invalid GPS frames. `FF` means 255 or more. |
| 3-4 | Number of GPS UART FIFO overflows. `FF` means 255 or more. |
| 5-6 | Number of PC communication UART FIFO overflows. `FF` means 255 or more. |

