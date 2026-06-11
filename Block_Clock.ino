#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <heltec-eink-modules.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "qrcode.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "src/config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#define HAS_SECRETS 1
#else
#define HAS_SECRETS 0
#endif

#ifndef USE_SECRETS_BOOTSTRAP
#define USE_SECRETS_BOOTSTRAP 0
#endif

#ifndef BLOCKCLOCK_WIFI_SSID
#define BLOCKCLOCK_WIFI_SSID ""
#endif

#ifndef BLOCKCLOCK_WIFI_PASS
#define BLOCKCLOCK_WIFI_PASS ""
#endif

#ifndef BLOCKCLOCK_MQTT_SERVER
#define BLOCKCLOCK_MQTT_SERVER DEFAULT_MQTT_SERVER
#endif

#ifndef BLOCKCLOCK_MQTT_PORT
#define BLOCKCLOCK_MQTT_PORT DEFAULT_MQTT_PORT
#endif

#ifndef BLOCKCLOCK_MQTT_USER
#define BLOCKCLOCK_MQTT_USER DEFAULT_MQTT_USER
#endif

#ifndef BLOCKCLOCK_MQTT_PASS
#define BLOCKCLOCK_MQTT_PASS DEFAULT_MQTT_PASS
#endif

RTC_DATA_ATTR BlockData rtcBlockData = {};
RTC_DATA_ATTR bool rtcHasBlockData = false;
RTC_DATA_ATTR int rtcBatteryPercent = -1;
RTC_DATA_ATTR float rtcBatteryVoltage = 0.0f;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
BlockClockDisplay display;
WebServer portalServer(CONFIG_PORTAL_PORT);
DNSServer portalDnsServer;
Preferences preferences;

DeviceConfig deviceConfig;
BlockData currentBlockData;
char deviceId[DEVICE_ID_SIZE] = "";
char portalApSsid[AP_SSID_SIZE] = "";
char portalApPassword[AP_PASSWORD_SIZE] = "";
bool portalShouldRestart = false;

#include "src/config_runtime.h"
#include "src/display_screens.h"
#include "src/block_data.h"
#include "src/setup_portal.h"

static bool setupButtonPressed() {
  pinMode(PIN_SETUP_BUTTON, INPUT_PULLUP);
  delay(20);
  return digitalRead(PIN_SETUP_BUTTON) == LOW;
}

static void prepareHardware() {
  pinMode(PIN_EINK_POWER, OUTPUT);
  digitalWrite(PIN_EINK_POWER, HIGH);
  delay(EINK_POWER_UP_DELAY_MS);
  display.begin();
  display.setRotation(1);
}

static void disconnectNetwork() {
  if (mqttClient.connected()) mqttClient.disconnect();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

static void goToSleep(uint16_t refreshMinutes) {
  digitalWrite(PIN_EINK_POWER, LOW);

  rtc_gpio_pullup_en((gpio_num_t)PIN_SETUP_BUTTON);
  rtc_gpio_pulldown_dis((gpio_num_t)PIN_SETUP_BUTTON);
  esp_sleep_enable_ext1_wakeup_io(SETUP_BUTTON_WAKE_MASK, ESP_EXT1_WAKEUP_ANY_LOW);

  const uint64_t sleepUs = (uint64_t)refreshMinutes * MICROSECONDS_PER_MINUTE;
  esp_sleep_enable_timer_wakeup(sleepUs);
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("==== Block Clock boot ====");

  prepareHardware();
  buildPortalCredentials();
  loadDeviceConfig(deviceConfig);
  applySecretsBootstrap(deviceConfig);

  const bool enterSetup = !deviceConfig.configured || setupButtonPressed();
  if (!deviceConfig.configured) {
    drawWelcomeScreen();
    delay(1400);
  }

  if (enterSetup) {
    runSetupPortal();
    return;
  }

  drawStatusScreen("BLOCK CLOCK", "Loading data");

  float batteryVoltage = readBatteryVoltage();
  int batteryPercent = batteryPercentFromVoltage(batteryVoltage);
  rtcBatteryVoltage = batteryVoltage;
  rtcBatteryPercent = batteryPercent;

  BlockData data = rtcHasBlockData ? rtcBlockData : emptyBlockData();
  bool wifiOk = connectWiFi(deviceConfig, WIFI_CONNECT_TIMEOUT_MS, false);
  bool dataOk = false;
  if (wifiOk) {
    syncClockFromNtp();
    if (deviceConfig.dataSourceMode == DATA_SOURCE_MQTT) {
      dataOk = fetchBlockDataFromMqtt(deviceConfig, data);
    } else {
      dataOk = fetchBlockDataOnline(deviceConfig, data);
    }
  }

  data.batteryVoltage = batteryVoltage;
  data.batteryPercent = batteryPercent;
  data.wifiOk = wifiOk;
  data.dataOk = dataOk;
  data.updatedAt = time(nullptr);
  if (data.updatedAt < 1700000000 && rtcHasBlockData) {
    data.updatedAt = rtcBlockData.updatedAt;
  }

  if (hasUsefulBlockData(data)) {
    rtcBlockData = data;
    rtcHasBlockData = true;
  }

  drawBlockDashboard(data, deviceConfig);
  disconnectNetwork();
  goToSleep(deviceConfig.refreshIntervalMinutes);
}

void loop() {
  delay(100);
}
