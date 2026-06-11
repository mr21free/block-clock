#pragma once

static bool hasText(const char* s) {
  return s && s[0] != '\0';
}

static int clampInt(int value, int minValue, int maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

static void safeCopy(char* dst, size_t dstSize, const char* src, size_t srcLen) {
  if (!dst || dstSize == 0) return;
  size_t n = (src && srcLen > 0) ? srcLen : 0;
  if (n > dstSize - 1) n = dstSize - 1;
  if (src && n > 0) memcpy(dst, src, n);
  dst[n] = '\0';
}

static void safeCopyCString(char* dst, size_t dstSize, const char* src) {
  if (!src) {
    safeCopy(dst, dstSize, "", 0);
    return;
  }
  safeCopy(dst, dstSize, src, strlen(src));
}

static void safeCopyString(char* dst, size_t dstSize, const String& src) {
  safeCopy(dst, dstSize, src.c_str(), src.length());
}

static bool checkboxArgIsTrue(const String& rawValue) {
  return rawValue == "1" || rawValue == "true" || rawValue == "on" || rawValue == "yes";
}

static uint16_t parsePort(const String& rawValue, uint16_t fallback) {
  const int parsed = rawValue.toInt();
  if (parsed <= 0 || parsed > 65535) return fallback;
  return (uint16_t)parsed;
}

static DataSourceMode sanitizeDataSourceMode(uint8_t rawValue) {
  return rawValue == DATA_SOURCE_MQTT ? DATA_SOURCE_MQTT : DATA_SOURCE_ONLINE;
}

static const char* dataSourceModeLabel(uint8_t rawValue) {
  return sanitizeDataSourceMode(rawValue) == DATA_SOURCE_MQTT ? "MQTT" : "ONLINE";
}

static void buildDeviceId(char* dst, size_t dstSize) {
  if (!dst || dstSize == 0) return;
  uint64_t chipId = ESP.getEfuseMac();
  uint32_t hash = 2166136261UL;
  for (uint8_t i = 0; i < 6; i++) {
    hash ^= (uint8_t)((chipId >> (i * 8)) & 0xFF);
    hash *= 16777619UL;
  }

  static constexpr uint32_t BASE36_6 = 2176782336UL;
  uint32_t value = hash % BASE36_6;
  char id[7];
  id[6] = '\0';
  for (int i = 5; i >= 0; i--) {
    const uint8_t digit = value % 36;
    id[i] = (digit < 10) ? (char)('0' + digit) : (char)('A' + digit - 10);
    value /= 36;
  }
  safeCopyCString(dst, dstSize, id);
}

static void buildPortalCredentials() {
  buildDeviceId(deviceId, sizeof(deviceId));
  snprintf(portalApSsid, sizeof(portalApSsid), "%s%s", AP_SSID_PREFIX, deviceId);
  snprintf(portalApPassword, sizeof(portalApPassword), "%s%s", AP_PASSWORD_PREFIX, deviceId);
}

static void buildMqttClientId(char* dst, size_t dstSize) {
  if (!dst || dstSize == 0) return;
  if (deviceId[0] == '\0') buildDeviceId(deviceId, sizeof(deviceId));
  snprintf(dst, dstSize, "BlockClock-%s", deviceId);
}

static void normalizeDeviceConfig(DeviceConfig& cfg) {
  cfg.version = CONFIG_VERSION;
  cfg.dataSourceMode = (uint8_t)sanitizeDataSourceMode(cfg.dataSourceMode);
  cfg.refreshIntervalMinutes = (uint16_t)clampInt(
    (int)cfg.refreshIntervalMinutes,
    MIN_REFRESH_INTERVAL_MINUTES,
    MAX_REFRESH_INTERVAL_MINUTES
  );
  if (cfg.refreshIntervalMinutes == 0) cfg.refreshIntervalMinutes = DEFAULT_REFRESH_INTERVAL_MINUTES;
  if (cfg.mqttPort == 0) cfg.mqttPort = DEFAULT_MQTT_PORT;
  if (!hasText(cfg.mqttServer)) safeCopyCString(cfg.mqttServer, sizeof(cfg.mqttServer), DEFAULT_MQTT_SERVER);
  if (!hasText(cfg.topicHeight)) safeCopyCString(cfg.topicHeight, sizeof(cfg.topicHeight), DEFAULT_TOPIC_HEIGHT);
  if (!hasText(cfg.topicHalving)) safeCopyCString(cfg.topicHalving, sizeof(cfg.topicHalving), DEFAULT_TOPIC_HALVING);
  if (!hasText(cfg.topicHashrate)) safeCopyCString(cfg.topicHashrate, sizeof(cfg.topicHashrate), DEFAULT_TOPIC_HASHRATE);
  if (!hasText(cfg.topicPriceUsd)) safeCopyCString(cfg.topicPriceUsd, sizeof(cfg.topicPriceUsd), DEFAULT_TOPIC_PRICE_USD);
}

static void applyDefaultDeviceConfig(DeviceConfig& cfg) {
  memset(&cfg, 0, sizeof(cfg));
  cfg.version = CONFIG_VERSION;
  cfg.configured = false;
  cfg.dataSourceMode = DATA_SOURCE_ONLINE;
  cfg.refreshIntervalMinutes = DEFAULT_REFRESH_INTERVAL_MINUTES;
  cfg.showBatteryPercent = true;
  safeCopyCString(cfg.mqttServer, sizeof(cfg.mqttServer), DEFAULT_MQTT_SERVER);
  cfg.mqttPort = DEFAULT_MQTT_PORT;
  safeCopyCString(cfg.mqttUser, sizeof(cfg.mqttUser), DEFAULT_MQTT_USER);
  safeCopyCString(cfg.mqttPass, sizeof(cfg.mqttPass), DEFAULT_MQTT_PASS);
  safeCopyCString(cfg.topicHeight, sizeof(cfg.topicHeight), DEFAULT_TOPIC_HEIGHT);
  safeCopyCString(cfg.topicHalving, sizeof(cfg.topicHalving), DEFAULT_TOPIC_HALVING);
  safeCopyCString(cfg.topicHashrate, sizeof(cfg.topicHashrate), DEFAULT_TOPIC_HASHRATE);
  safeCopyCString(cfg.topicPriceUsd, sizeof(cfg.topicPriceUsd), DEFAULT_TOPIC_PRICE_USD);
}

static bool loadDeviceConfig(DeviceConfig& cfg) {
  applyDefaultDeviceConfig(cfg);
  if (!preferences.begin(CONFIG_NAMESPACE, true)) return false;
  const size_t storedSize = preferences.getBytesLength("device");
  if (storedSize == sizeof(DeviceConfig)) {
    DeviceConfig stored = {};
    preferences.getBytes("device", &stored, sizeof(stored));
    if (stored.version == CONFIG_VERSION) {
      cfg = stored;
      normalizeDeviceConfig(cfg);
      preferences.end();
      return true;
    }
  }
  preferences.end();
  return false;
}

static bool saveDeviceConfig(const DeviceConfig& cfg) {
  DeviceConfig copy = cfg;
  normalizeDeviceConfig(copy);
  if (!preferences.begin(CONFIG_NAMESPACE, false)) return false;
  bool ok = preferences.putBytes("device", &copy, sizeof(copy)) == sizeof(copy);
  preferences.end();
  return ok;
}

static void applySecretsBootstrap(DeviceConfig& cfg) {
#if HAS_SECRETS && USE_SECRETS_BOOTSTRAP
  if (!cfg.configured && strlen(BLOCKCLOCK_WIFI_SSID) > 0) {
    safeCopyCString(cfg.wifiSsid, sizeof(cfg.wifiSsid), BLOCKCLOCK_WIFI_SSID);
    safeCopyCString(cfg.wifiPass, sizeof(cfg.wifiPass), BLOCKCLOCK_WIFI_PASS);
    safeCopyCString(cfg.mqttServer, sizeof(cfg.mqttServer), BLOCKCLOCK_MQTT_SERVER);
    cfg.mqttPort = BLOCKCLOCK_MQTT_PORT;
    safeCopyCString(cfg.mqttUser, sizeof(cfg.mqttUser), BLOCKCLOCK_MQTT_USER);
    safeCopyCString(cfg.mqttPass, sizeof(cfg.mqttPass), BLOCKCLOCK_MQTT_PASS);
    cfg.configured = true;
    saveDeviceConfig(cfg);
  }
#else
  (void)cfg;
#endif
}

static BlockData emptyBlockData() {
  BlockData data = {};
  safeCopyCString(data.blockHeight, sizeof(data.blockHeight), "--");
  safeCopyCString(data.blocksToHalving, sizeof(data.blocksToHalving), "--");
  safeCopyCString(data.priceUsd, sizeof(data.priceUsd), "--");
  safeCopyCString(data.hashrateEhs, sizeof(data.hashrateEhs), "--");
  data.batteryVoltage = 0.0f;
  data.batteryPercent = -1;
  data.updatedAt = 0;
  data.wifiOk = false;
  data.dataOk = false;
  return data;
}

static bool hasUsefulBlockData(const BlockData& data) {
  return strcmp(data.blockHeight, "--") != 0 ||
    strcmp(data.blocksToHalving, "--") != 0 ||
    strcmp(data.priceUsd, "--") != 0 ||
    strcmp(data.hashrateEhs, "--") != 0;
}

static int batteryPercentFromVoltage(float v) {
  static constexpr int NUM_POINTS = 24;
  static constexpr float voltTable[NUM_POINTS] = {
    3.20f, 3.26f, 3.32f, 3.36f, 3.42f, 3.52f, 3.62f, 3.67f,
    3.71f, 3.74f, 3.76f, 3.78f, 3.82f, 3.87f, 3.89f, 3.90f,
    3.91f, 3.92f, 3.94f, 3.95f, 3.97f, 4.00f, 4.02f, 4.08f
  };
  static constexpr int socTable[NUM_POINTS] = {
    0, 0, 1, 2, 5, 10, 15, 20,
    25, 30, 38, 48, 55, 60, 63, 69,
    78, 83, 88, 91, 94, 98, 99, 100
  };

  if (v <= voltTable[0]) return socTable[0];
  if (v >= voltTable[NUM_POINTS - 1]) return socTable[NUM_POINTS - 1];
  for (int i = 0; i < NUM_POINTS - 1; i++) {
    const float v1 = voltTable[i];
    const float v2 = voltTable[i + 1];
    if (v >= v1 && v <= v2) {
      const int soc1 = socTable[i];
      const int soc2 = socTable[i + 1];
      const float t = (v - v1) / (v2 - v1);
      return clampInt((int)(soc1 + t * (soc2 - soc1) + 0.5f), 0, 100);
    }
  }
  return 0;
}

static float readBatteryVoltage() {
  pinMode(PIN_ADC_CTRL, OUTPUT);
  digitalWrite(PIN_ADC_CTRL, HIGH);
  delay(BATTERY_ADC_SETTLE_DELAY_MS);

  pinMode(PIN_BAT_ADC, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BAT_ADC, ADC_0db);

  static constexpr uint8_t SAMPLE_COUNT = 12;
  uint32_t millivoltsSum = 0;
  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    millivoltsSum += (uint32_t)analogReadMilliVolts(PIN_BAT_ADC);
    delay(2);
  }

  digitalWrite(PIN_ADC_CTRL, LOW);
  const float adcVolts = ((float)millivoltsSum / (float)SAMPLE_COUNT) / 1000.0f;
  return adcVolts * VBAT_SCALE;
}

static bool syncClockFromNtp(uint16_t timeoutMs = NTP_SYNC_TIMEOUT_MS) {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  uint32_t start = millis();
  time_t now = time(nullptr);
  while (now < 1700000000 && (millis() - start) < timeoutMs) {
    delay(WIFI_POLL_DELAY_MS);
    now = time(nullptr);
  }
  return now >= 1700000000;
}

static String formatTimestamp(time_t when) {
  if (when < 1700000000) return "not synced";
  struct tm tmValue = {};
  if (!gmtime_r(&when, &tmValue)) return "not synced";
  char buffer[24];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &tmValue);
  return String(buffer);
}
