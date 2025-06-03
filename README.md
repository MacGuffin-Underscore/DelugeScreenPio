# DelugeScreenPio
### A slightly cheaper alternative, with some extra features
I made this to be a cheaper alternative to getting your deluge sent for a upgrade.  Even if I already paid for it. It uses the USB port of a USBA host feather to handle midi with PIO, and takes power from the USB C to charge the deluge/onboard battery.

Still a WIP, so features such as MIDI CC encoders, more button functionality, static screens, 7segment support, etc will be added as I feel like it.

## Bill of Materials
For the minimal version, you only need the Feather, OLED, and headers to connect them.  Everything else is optional.
| Part | Price | qty | Required |
| --------------------------------------------------------------------------- | ------ | --- | --- |
| [Adafruit Feather RP2040 USBA Host](https://www.adafruit.com/product/5723)  | $17.50 |   1 |   Y |
| [Adafruit FeatherWing 128x64 OLED](https://www.adafruit.com/product/4650)   | $14.95 |   1 |   Y |
| [Header Kit for Feather](https://www.adafruit.com/product/2886)             |  $0.95 |   1 |   Y |
| [JST-PH 2-pin Jumper Cable](https://www.adafruit.com/product/4714)          |  $0.95 |   1 |   N |
| [Adafruit MAX17048 Fuel Gauge](https://www.adafruit.com/product/4650)       |  $5.95 |   1 |   N |
| [STEMMA QT Cable](https://www.adafruit.com/product/4399)                    |  $0.95 |   4 |   N |
| [Adafruit 0.56" 7-Segment Display](https://www.adafruit.com/product/1002)   | $10.95 |   1 |   N |
| [Adafruit I2C Quad Encoder](https://www.adafruit.com/product/2886)          |  $7.50 |   2 |   N |
| [Encoders With Button**](https://www.amazon.com//dp/B07MW7D4FD)             |  $1.00 |   8 |   N |
| [1900H Davies Knobs***](https://www.amazon.com/dp/B07DLKQGLN)               |  $0.50 |   8 |   N |
| [3v7 LiPo Battery****](https://www.amazon.com/dp/B0D3LMBJC5)                | $20.00 |   1 |   N |

** This is a 10 pack, so you will only need one of these for the 8 encoders. Of course, if you add more encoder boards that means more encoders. Naturally.

*** Any knobs work (depending on which encoders you bought), and you can find these cheaper elsewhere

**** WARNING: This battery's connections are backwards for adafruit boards, if you buy this one you will have to switch the red and black wires in the JST connector or it will fry your board.  Always check polarity before plugging in.

## Known Bugs
Code is still freezing up after a time. It is probably stuck in some while loop

## Planned Features
- 7 Segment display support
- Encoder MIDI CC support
- Static screens for the unused display
- Battery indicator

If you have ideas, please suggest away!

## Give me money
(I really don't need it)
https://ko-fi.com/macguffin
