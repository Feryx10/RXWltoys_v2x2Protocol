# 📡 RXWltoys_v2x2Protocol

RXWltoys_v2x2Protocol is an open-source receiver implementation for the WLtoys V2x2 series radio protocol, designed for the **ESP32-C3** and **Beken BK2425** or **nRF24L01(+)** modules.  

---

## 🔌 Wiring (ESP32-C3 ↔ BK2425)

| Signal | ESP32-C3 | BK2425    |
|--------|----------|-----------|
| CE     | GPIO 2   | CE        |
| CSN    | GPIO 3   | CSN       |
| SCK    | GPIO 4   | SCK       |
| MISO   | GPIO 5   | MISO      |
| MOSI   | GPIO 6   | MOSI      |
| VCC    | 3.3V     | VCC       |
| GND    | GND      | GND       |

> ⚠️ Make sure to use a capacitor (e.g., 10μF) across VCC-GND near the BK2425 to avoid brownouts.

---

## 📦 Requirements

- ESP32-C3 board (Arduino IDE compatible)
- Beken BK2425 - nRF24L01(+) module
- Arduino IDE
- `RF24` library by TMRh20

---

## ⚙️ Applications
- DIY drone or robot receiver using WLtoys transmitters.
- Custom RC vehicles with native WLtoys control support.
- Protocol reverse engineering and radio analysis.

---

## 🧠 Protocol Summary

---

## 🤝 Credits

Based on:

- [execuc/v202-receiver](https://github.com/execuc/v202-receiver)
- [mishakevlishvili/wltoys-v202-protocol](https://github.com/mishakevlishvili/wltoys-v202-protocol)
- [DeviationTX - v202_nrf24l01.c](https://github.com/DeviationTX/deviation/blob/master/src/protocol/v202_nrf24l01.c)
- [gbraad/v202-protocol](https://github.com/gbraad/v202-protocol)
- [goebish/nrf24_multipro - V2x2.ino](https://github.com/goebish/nrf24_multipro/blob/master/nRF24_multipro/V2x2.ino)
- WLtoys protocol reverse engineering efforts