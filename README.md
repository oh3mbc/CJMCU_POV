ESP32S & CJMCU-2812-8 Stationary True POV Display Guide

This project utilizes an ESP32S and a CJMCU-2812-8 (8-bit addressable WS2812B RGB LED stick) to create a Stationary True POV (Persistence of Vision) display. Instead of relying on a spinning motor, it flashes patterns at an ultra-high frequency, leaving a temporary afterimage of text in the human retina when the user rapidly sweeps their eyes or shakes their head back and forth across the static LED stick. 

The device functions as an independent WiFi Access Point (AP) with an integrated asynchronous web server. You can connect directly to it via a smartphone or computer to dynamically change the text message, lighting color, global brightness, and timing intervals on the fly.

---

1. Hardware Connections & Wiring

Since addressable WS2812B LEDs require a stable 5V logic reference to operate reliably at peak brightness, connect the power rail directly to the 5V input of the ESP32S.

| CJMCU-2812-8 Pin | ESP32S Pin | Component Note |
| :--- | :--- | :--- |
| 5V / VCC | 5V / VIN | Powers the 8 RGB pixels. Do not use 3.3V. |
| GND / G | GND | Shared common ground. |
| DI (Data In) | D13 (GPIO 13) | Data line. Add a 330 ohm protective resistor if available. |
| DO (Data Out) | Unconnected | Leave floating. Used only for daisy-chaining multiple sticks. |

Note: Some CJMCU component batches have mislabeled silk screens where the positions of the text labels DI and DO are accidentally swapped. If your LEDs fail to light up upon completing the setup, try moving your ESP32S data wire to the opposite physical data pin on the stick.
