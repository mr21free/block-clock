#pragma once

static String htmlEscape(const char* s) {
  String out;
  if (!s) return out;
  while (*s) {
    switch (*s) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      default: out += *s; break;
    }
    s++;
  }
  return out;
}

static String checkedAttr(bool checked) {
  return checked ? F(" checked") : F("");
}

static String selectedAttr(bool selected) {
  return selected ? F(" selected") : F("");
}

static String portalPage() {
  String html;
  html.reserve(9000);
  html += F("<!doctype html><html><head><meta charset=\"utf-8\">");
  html += F("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">");
  html += F("<title>Block Clock setup</title>");
  html += F("<style>");
  html += F("body{margin:0;background:#f5f1e8;color:#24211d;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;line-height:1.45}");
  html += F("main{max-width:760px;margin:0 auto;padding:28px 18px 44px}");
  html += F("h1{font-size:32px;font-weight:400;margin:0 0 8px}");
  html += F("p{color:#5c554c;margin:0 0 22px}");
  html += F("form{display:grid;gap:18px}");
  html += F("section{border-top:1px solid #d8cfc1;padding-top:18px}");
  html += F("h2{font-size:18px;font-weight:500;margin:0 0 12px}");
  html += F("label{display:block;font-size:13px;color:#5c554c;margin:12px 0 6px}");
  html += F("input,select{width:100%;box-sizing:border-box;border:1px solid #cfc5b6;border-radius:6px;padding:11px;background:#fffdf8;color:#24211d;font:inherit}");
  html += F(".row{display:grid;grid-template-columns:1fr 1fr;gap:14px}");
  html += F(".check{display:flex;gap:10px;align-items:center;margin-top:12px;color:#24211d}");
  html += F(".check input{width:auto}");
  html += F(".hint{font-size:12px;color:#7b7166;margin-top:6px}");
  html += F("button{border:0;border-radius:999px;background:#24211d;color:#f5f1e8;padding:13px 18px;font:inherit;cursor:pointer}");
  html += F("@media(max-width:640px){.row{grid-template-columns:1fr}}");
  html += F("</style></head><body><main>");
  html += F("<h1>Block Clock setup</h1>");
  html += F("<p>Choose Wi-Fi, data source, refresh interval, and battery display. No account, cloud setup, or PIN.</p>");
  html += F("<form method=\"post\" action=\"/save\">");

  html += F("<section><h2>Wi-Fi</h2>");
  html += F("<label for=\"wifi_ssid\">Network name</label>");
  html += F("<input id=\"wifi_ssid\" name=\"wifi_ssid\" maxlength=\"32\" value=\"");
  html += htmlEscape(deviceConfig.wifiSsid);
  html += F("\">");
  html += F("<label for=\"wifi_pass\">Password</label>");
  html += F("<input id=\"wifi_pass\" name=\"wifi_pass\" type=\"password\" maxlength=\"64\" value=\"");
  html += htmlEscape(deviceConfig.wifiPass);
  html += F("\">");
  html += F("</section>");

  html += F("<section><h2>Data</h2>");
  html += F("<label for=\"data_source\">Source</label>");
  html += F("<select id=\"data_source\" name=\"data_source\">");
  html += F("<option value=\"online\"");
  html += selectedAttr(sanitizeDataSourceMode(deviceConfig.dataSourceMode) == DATA_SOURCE_ONLINE);
  html += F(">Online, mempool.space and CoinGecko</option>");
  html += F("<option value=\"mqtt\"");
  html += selectedAttr(sanitizeDataSourceMode(deviceConfig.dataSourceMode) == DATA_SOURCE_MQTT);
  html += F(">MQTT, node or Home Assistant</option>");
  html += F("</select>");
  html += F("<div class=\"hint\">Online mode fetches public Bitcoin data. MQTT mode uses retained local topics.</div>");
  html += F("</section>");

  html += F("<section><h2>Refresh and battery</h2><div class=\"row\"><div>");
  html += F("<label for=\"refresh\">Refresh interval, minutes</label>");
  html += F("<input id=\"refresh\" name=\"refresh\" type=\"number\" min=\"1\" max=\"10080\" value=\"");
  html += String(deviceConfig.refreshIntervalMinutes);
  html += F("\"></div><div>");
  html += F("<label for=\"battery_pct\">Battery percent</label>");
  html += F("<select id=\"battery_pct\" name=\"battery_pct\">");
  html += F("<option value=\"1\"");
  html += selectedAttr(deviceConfig.showBatteryPercent);
  html += F(">Show percent</option><option value=\"0\"");
  html += selectedAttr(!deviceConfig.showBatteryPercent);
  html += F(">Icon only</option></select></div></div></section>");

  html += F("<section><h2>MQTT</h2>");
  html += F("<div class=\"row\"><div><label for=\"mqtt_server\">Server</label><input id=\"mqtt_server\" name=\"mqtt_server\" maxlength=\"63\" value=\"");
  html += htmlEscape(deviceConfig.mqttServer);
  html += F("\"></div><div><label for=\"mqtt_port\">Port</label><input id=\"mqtt_port\" name=\"mqtt_port\" type=\"number\" min=\"1\" max=\"65535\" value=\"");
  html += String(deviceConfig.mqttPort);
  html += F("\"></div></div>");
  html += F("<div class=\"row\"><div><label for=\"mqtt_user\">Username</label><input id=\"mqtt_user\" name=\"mqtt_user\" maxlength=\"63\" value=\"");
  html += htmlEscape(deviceConfig.mqttUser);
  html += F("\"></div><div><label for=\"mqtt_pass\">Password</label><input id=\"mqtt_pass\" name=\"mqtt_pass\" type=\"password\" maxlength=\"63\" value=\"");
  html += htmlEscape(deviceConfig.mqttPass);
  html += F("\"></div></div>");
  html += F("<label for=\"topic_height\">Block height topic</label><input id=\"topic_height\" name=\"topic_height\" maxlength=\"95\" value=\"");
  html += htmlEscape(deviceConfig.topicHeight);
  html += F("\">");
  html += F("<label for=\"topic_halving\">Blocks to halving topic</label><input id=\"topic_halving\" name=\"topic_halving\" maxlength=\"95\" value=\"");
  html += htmlEscape(deviceConfig.topicHalving);
  html += F("\">");
  html += F("<label for=\"topic_hashrate\">Hashrate topic</label><input id=\"topic_hashrate\" name=\"topic_hashrate\" maxlength=\"95\" value=\"");
  html += htmlEscape(deviceConfig.topicHashrate);
  html += F("\">");
  html += F("<label for=\"topic_price\">BTC price USD topic</label><input id=\"topic_price\" name=\"topic_price\" maxlength=\"95\" value=\"");
  html += htmlEscape(deviceConfig.topicPriceUsd);
  html += F("\">");
  html += F("</section>");

  html += F("<button type=\"submit\">Save and restart</button>");
  html += F("</form></main></body></html>");
  return html;
}

static void handlePortalRoot() {
  portalServer.send(200, "text/html", portalPage());
}

static void handlePortalSave() {
  DeviceConfig cfg = deviceConfig;
  safeCopyString(cfg.wifiSsid, sizeof(cfg.wifiSsid), portalServer.arg("wifi_ssid"));
  safeCopyString(cfg.wifiPass, sizeof(cfg.wifiPass), portalServer.arg("wifi_pass"));
  cfg.dataSourceMode = portalServer.arg("data_source") == "mqtt" ? DATA_SOURCE_MQTT : DATA_SOURCE_ONLINE;
  cfg.refreshIntervalMinutes = (uint16_t)clampInt(
    portalServer.arg("refresh").toInt(),
    MIN_REFRESH_INTERVAL_MINUTES,
    MAX_REFRESH_INTERVAL_MINUTES
  );
  cfg.showBatteryPercent = portalServer.arg("battery_pct") != "0";
  safeCopyString(cfg.mqttServer, sizeof(cfg.mqttServer), portalServer.arg("mqtt_server"));
  cfg.mqttPort = parsePort(portalServer.arg("mqtt_port"), DEFAULT_MQTT_PORT);
  safeCopyString(cfg.mqttUser, sizeof(cfg.mqttUser), portalServer.arg("mqtt_user"));
  safeCopyString(cfg.mqttPass, sizeof(cfg.mqttPass), portalServer.arg("mqtt_pass"));
  safeCopyString(cfg.topicHeight, sizeof(cfg.topicHeight), portalServer.arg("topic_height"));
  safeCopyString(cfg.topicHalving, sizeof(cfg.topicHalving), portalServer.arg("topic_halving"));
  safeCopyString(cfg.topicHashrate, sizeof(cfg.topicHashrate), portalServer.arg("topic_hashrate"));
  safeCopyString(cfg.topicPriceUsd, sizeof(cfg.topicPriceUsd), portalServer.arg("topic_price"));
  cfg.configured = true;
  normalizeDeviceConfig(cfg);

  deviceConfig = cfg;
  saveDeviceConfig(deviceConfig);
  portalShouldRestart = true;

  portalServer.send(
    200,
    "text/html",
    F("<!doctype html><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><body style=\"font-family:monospace;background:#f5f1e8;color:#24211d\"><main style=\"max-width:620px;margin:40px auto;padding:16px\"><h1>Saved</h1><p>Block Clock is restarting.</p></main></body>")
  );
  drawSetupSavedScreen();
}

static void handlePortalRedirect() {
  portalServer.sendHeader("Location", String("http://") + CONFIG_AP_IP.toString(), true);
  portalServer.send(302, "text/plain", "");
}

static void startPortalServers() {
  portalServer.on("/", HTTP_GET, handlePortalRoot);
  portalServer.on("/save", HTTP_POST, handlePortalSave);
  portalServer.on("/generate_204", HTTP_GET, handlePortalRedirect);
  portalServer.on("/hotspot-detect.html", HTTP_GET, handlePortalRedirect);
  portalServer.on("/connecttest.txt", HTTP_GET, handlePortalRedirect);
  portalServer.on("/ncsi.txt", HTTP_GET, handlePortalRedirect);
  portalServer.onNotFound(handlePortalRedirect);
  portalServer.begin();
}

static void runSetupPortal() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(CONFIG_AP_IP, CONFIG_AP_GATEWAY, CONFIG_AP_SUBNET);
  WiFi.softAP(portalApSsid, portalApPassword);

  portalDnsServer.start(CONFIG_DNS_PORT, "*", CONFIG_AP_IP);
  startPortalServers();
  drawSetupPortalReadyScreen();

  while (!portalShouldRestart) {
    portalDnsServer.processNextRequest();
    portalServer.handleClient();
    delay(CONFIG_PORTAL_DELAY_MS);
  }

  const uint32_t start = millis();
  while ((millis() - start) < CONFIG_PORTAL_RESTART_DELAY_MS) {
    portalServer.handleClient();
    delay(CONFIG_PORTAL_DELAY_MS);
  }
  ESP.restart();
}
