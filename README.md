# Block Clock

Block Clock is a small open-source Bitcoin desk display for Heltec Vision Master e-ink boards. It shows Bitcoin network data without needing a commercial block clock.

The firmware supports:

- Heltec Vision Master E213
- Heltec Vision Master E290

The same source code is used for both boards. Select the matching board in Arduino IDE before compiling.

Read the site page at [freedomclock.io/diy-block-clock/](https://freedomclock.io/diy-block-clock/).

## What it shows

- Block height
- Blocks remaining to the next halving
- BTC price in USD
- Network hashrate in EH/s
- Battery icon, with optional percent
- Last update time

The display refreshes on the configured interval, then the ESP32 goes back to deep sleep.

## Data sources

Block Clock has two data modes.

MQTT mode is for people who already publish Bitcoin data on their local network, usually from a Bitcoin node or Home Assistant. Use retained MQTT messages so the clock can wake, read the latest values, refresh the display, and sleep again.

Online mode fetches public data directly:

- block height from mempool.space
- BTC price from mempool.space, with CoinGecko as fallback
- network hashrate from mempool.space
- halving countdown calculated locally from block height

MQTT is still the better privacy setup. Online mode is simpler.

## Hardware

- Heltec Vision Master E213, 2.13 inch e-ink display
- or Heltec Vision Master E290, 2.90 inch e-ink display
- 3.7 V LiPo battery with the right connector
- USB-C cable

No soldering is required for the basic build.

The E213 build has been tested around one week of runtime on a 500 mAh battery with hourly refreshes. Runtime on E290 depends on the battery, refresh interval, and Wi-Fi conditions.

## First setup

On first boot the device shows a welcome screen:

```text
BLOCK CLOCK
```

It then starts a setup Wi-Fi network and shows a QR code on the e-ink display. Scan the QR code, join the setup network, and open:

```text
http://192.168.4.1
```

The setup page lets you configure:

- Wi-Fi
- data source, MQTT or online
- refresh interval
- battery percent display
- MQTT server, credentials, and topics

There is no setup PIN. The block clock does not store personal financial data.

## Default MQTT topics

```text
home/bitcoin/height
home/bitcoin/halving/blocks_remaining
home/bitcoin/hashrate_ehs
home/bitcoin/price/usd
```

## Build and flash

1. Install Arduino IDE.
2. Install these libraries:
   - heltec-eink-modules by Todd Herbert
   - Heltec ESP32 Dev-Boards
   - PubSubClient by Nick O'Leary
   - ArduinoJson by Benoit Blanchon
3. Open `Block_Clock.ino`.
4. Connect the board with USB-C.
5. Select the matching board:
   - `Heltec Vision Master E213`
   - `Heltec Vision Master E290`
6. Upload the sketch.

Each board needs firmware compiled for that board profile.

## Optional secrets bootstrap

Normal setup happens through the browser page. If you want local defaults for development, copy `secrets.example.h` to `secrets.h` and edit it.

`secrets.h` is ignored by git.

## Code structure

- `Block_Clock.ino`: Arduino entrypoint
- `src/config.h`: board profile, defaults, constants, and config structs
- `src/config_runtime.h`: saved config, battery reading, device ID, and helpers
- `src/setup_portal.h`: QR setup screen and browser setup page
- `src/block_data.h`: MQTT and online data loading
- `src/display_screens.h`: welcome, setup, battery icon, and main dashboard drawing

## TODO

- TODO(miro): replace the temporary block mark with a dedicated Block Clock logo. Do not reuse the Freedom Clock logo.
- Add enclosure files when the physical design is ready.

## License

MIT
