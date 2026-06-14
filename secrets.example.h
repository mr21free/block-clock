#pragma once

// Optional local bootstrap. Normal setup happens through the device setup page.
#define USE_SECRETS_BOOTSTRAP 1

#define BLOCKCLOCK_WIFI_SSID "YOUR_WIFI_NAME"
#define BLOCKCLOCK_WIFI_PASS "YOUR_WIFI_PASSWORD"

// Use DATA_SOURCE_ONLINE for mempool.space/CoinGecko.
// Use DATA_SOURCE_MQTT for your own node or Home Assistant topics.
#define BLOCKCLOCK_DATA_SOURCE_MODE DATA_SOURCE_ONLINE
// Available currencies: CURRENCY_USD, CURRENCY_EUR, CURRENCY_CHF,
// CURRENCY_GBP, CURRENCY_CAD, CURRENCY_AUD.
#define BLOCKCLOCK_CURRENCY_CODE CURRENCY_USD
#define BLOCKCLOCK_DISPLAY_THEME_MODE DISPLAY_THEME_LIGHT

#define BLOCKCLOCK_MQTT_SERVER "192.168.1.144"
#define BLOCKCLOCK_MQTT_PORT 1883
#define BLOCKCLOCK_MQTT_USER "mqtt"
#define BLOCKCLOCK_MQTT_PASS "mqtt"

#define BLOCKCLOCK_TOPIC_HEIGHT "home/bitcoin/height"
#define BLOCKCLOCK_TOPIC_HALVING "home/bitcoin/halving/blocks_remaining"
#define BLOCKCLOCK_TOPIC_HASHRATE "home/bitcoin/hashrate_ehs"
#define BLOCKCLOCK_TOPIC_PRICE_VALUE "home/bitcoin/price/usd"
