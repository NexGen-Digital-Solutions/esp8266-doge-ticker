# esp8266-doge-ticker
Simple ESP8266 Dogecoin Ticker Tracker - API

Simple NodeMCU ESP8266 Dogecoin crypto currency price tracker.

Enclosure is ready-to-print STL and Cura Profile @ 0.2mm resolution w/ proper supports for OLED.

[Link to ESP8266 used](https://www.amazon.com/gp/product/B081CSJV2V/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1)

[Link to OLED used](https://www.amazon.com/gp/product/B08L7QW7SR/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1)

[Link to RGB LED's used](https://www.amazon.com/Tricolor-Diffused-Multicolor-Electronics-Components/dp/B01C3ZZT8W)

Board is secured in place with [M3 screws](https://www.amazon.com/Litorange-Standoff-Threaded-Motherboard-Assortment/dp/B07D7828LC). I need to tweak design so they can be threaded in, right now they just slide it. It's enough force to just-barely hold the board in place. Don't go throwing it around -yet.

OLED pretty much snaps in place, recommend you secure with some hot glue.

5mm RGB LED diode in rocket slides up through hole and fits in place. I soldered and heat-shrunk the wires then secured at base of shaft with hot glue.

Arduino code pulls ticker from [Cryptonator API](https://www.cryptonator.com/api) every 30 seconds.

Please share comments and suggestions, feel free to fork.
