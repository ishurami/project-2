## Board

```
ESP8266 (WeMos D1 Mini)
```

## ESP8266 Board Manager URL

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

## Pin
### PH Sensor (PH-4502C)
| PH-4502C  | Micro     |
| --------- | --------- |
| To        | -         |
| Do        | -         |
| Po        | A0        |
| G         | -         |
| G         | GND       |
| V+        | 3.3V      |

### 4 Relay Set
| Relay     | Micro                 |
| --------- | --------------------- |
| GND       | GND                   |
| In1       | GPIO 03 / RX (Mixer)   |
| In2       | GPIO 04 / D2 (Asam)    |
| In3       | GPIO 05 / D1 (Basa)    |
| In4       | -                     |
| VCC       | 5V                    |

## Info
- Short BNC prob of PH-4502C module to get voltage at PH 7.
- Offset potentiometer near the BNC prob.
- Get analogRead of Po (A0) and save it in buf[100] array. After that, sort from least to greater and get avarage of value number 25 to 75.

## Reference
- https://www.youtube.com/watch?v=3_w4GhrEoQ8