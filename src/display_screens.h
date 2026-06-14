#pragma once

static constexpr int BATTERY_BODY_W = 20;
static constexpr int BATTERY_BODY_H = 10;
static constexpr int BATTERY_TIP_W = 1;
static constexpr int BATTERY_TIP_H = 5;
static constexpr int BATTERY_RIGHT_MARGIN = 3;
static constexpr int BATTERY_TEXT_GAP = 2;
static constexpr int BATTERY_ICON_Y = 3;
static constexpr int BATTERY_TEXT_Y = 5;
static constexpr int NO_WIFI_ICON_W = 14;
static constexpr int NO_WIFI_ICON_H = 12;
static constexpr int STATUS_ICON_GAP = 5;

// Partial-refresh window state. When active, prepareScreen uses fillRect
// (keeping the existing panel image outside the window) so display.fastmodeOn()
// can do a true partial refresh — one quick blink instead of a full waveform.
static bool screenRenderWindowActive = false;
static int screenRenderWindowX = 0;
static int screenRenderWindowY = 0;
static int screenRenderWindowW = DEVICE_DISPLAY_WIDTH;
static int screenRenderWindowH = DEVICE_DISPLAY_HEIGHT;

static void setScreenRenderWindow(int x, int y, int w, int h) {
  screenRenderWindowActive = true;
  screenRenderWindowX = clampInt(x, 0, DEVICE_DISPLAY_WIDTH - 1);
  screenRenderWindowY = clampInt(y, 0, DEVICE_DISPLAY_HEIGHT - 1);
  screenRenderWindowW = clampInt(w, 1, DEVICE_DISPLAY_WIDTH - screenRenderWindowX);
  screenRenderWindowH = clampInt(h, 1, DEVICE_DISPLAY_HEIGHT - screenRenderWindowY);
}

static void clearScreenRenderWindow() {
  screenRenderWindowActive = false;
  screenRenderWindowX = 0;
  screenRenderWindowY = 0;
  screenRenderWindowW = DEVICE_DISPLAY_WIDTH;
  screenRenderWindowH = DEVICE_DISPLAY_HEIGHT;
}

static void prepareScreen(DisplayThemeMode themeMode = statusScreenThemeMode()) {
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  display.setRotation(1);
  if (screenRenderWindowActive) {
    // Partial refresh path: keep the existing panel image and only redraw the requested window.
    // No clearMemory — buffer keeps the pending frame so fastmode only diffs the changed region.
    display.setWindow(screenRenderWindowX, screenRenderWindowY, screenRenderWindowW, screenRenderWindowH);
    display.fillRect(screenRenderWindowX, screenRenderWindowY, screenRenderWindowW, screenRenderWindowH, theme.background);
  } else {
    // fastmodeOff() resets hardware mode but not winrot_*; this ensures update() refreshes the full screen.
    display.setWindow(0, 0, DEVICE_DISPLAY_WIDTH, DEVICE_DISPLAY_HEIGHT);
    display.clearMemory();
    display.fillScreen(theme.background);
  }
  display.setTextColor(theme.foreground, theme.background);
}

static void drawNoWifiStatusIcon(int x, int y, uint16_t color, uint16_t background) {
  // Tiny Wi-Fi-with-slash icon, tuned manually for 1-bit e-ink.
  display.fillRect(x, y, NO_WIFI_ICON_W, NO_WIFI_ICON_H, background);
  display.drawLine(x + 3, y + 1, x + 10, y + 1, color);
  display.drawPixel(x + 2, y + 2, color);
  display.drawPixel(x + 11, y + 2, color);
  display.drawLine(x + 4, y + 4, x + 9, y + 4, color);
  display.drawPixel(x + 3, y + 5, color);
  display.drawPixel(x + 10, y + 5, color);
  display.drawPixel(x + 5, y + 7, color);
  display.drawPixel(x + 6, y + 6, color);
  display.drawPixel(x + 7, y + 6, color);
  display.drawPixel(x + 8, y + 7, color);
  display.fillRect(x + 6, y + 9, 2, 2, color);
  display.drawLine(x + 2, y + 1, x + 11, y + 10, color);
}

static void drawNoWifiStatusLeftOfBattery(int batteryIconX, DisplayThemeMode themeMode) {
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  const int iconX = batteryIconX - STATUS_ICON_GAP - NO_WIFI_ICON_W;
  if (iconX < 1) return;
  drawNoWifiStatusIcon(iconX, BATTERY_ICON_Y - 1, theme.foreground, theme.background);
}

static void drawBlockClockMark(int x, int y, int size, uint16_t color = BLACK, uint16_t background = WHITE) {
  const int wall = size >= 20 ? 4 : 3;
  display.fillRect(x, y, size, size, color);
  display.fillRect(x + wall, y + wall, size - (wall * 2), size - (wall * 2), background);
}

static int estimateTextWidthSize1(const char* text) {
  return text ? (int)strlen(text) * 6 : 0;
}

static void drawBatteryIcon(
  int x,
  int y,
  int pct,
  int bodyW,
  int bodyH,
  int tipW,
  int tipH,
  DisplayThemeMode themeMode = DISPLAY_THEME_LIGHT
) {
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;

  const int pad = (bodyW >= 28) ? 3 : 2;

  display.drawRoundRect(x, y, bodyW, bodyH, 3, theme.foreground);
  display.fillRect(x + bodyW, y + (bodyH - tipH) / 2, tipW, tipH, theme.foreground);

  const int innerX = x + pad;
  const int innerY = y + pad;
  const int innerW = bodyW - (pad * 2);
  const int innerH = bodyH - (pad * 2);

  display.fillRect(innerX, innerY, innerW, innerH, theme.background);

  int fillW = (innerW * pct) / 100;
  if (fillW > 0) {
    display.fillRect(innerX, innerY, fillW, innerH, theme.foreground);
  }
}

static int drawBatteryStatus(int batteryPct, bool showPercent, DisplayThemeMode themeMode) {
  int iconX;
  if (showPercent && batteryPct >= 0) {
    char pctText[8];
    snprintf(pctText, sizeof(pctText), "%d%%", clampInt(batteryPct, 0, 100));
    const int textW = estimateTextWidthSize1(pctText);
    const int textX = DEVICE_DISPLAY_WIDTH - BATTERY_RIGHT_MARGIN - textW;
    iconX = textX - BATTERY_TEXT_GAP - BATTERY_BODY_W - BATTERY_TIP_W;
    display.setTextSize(1);
    display.setCursor(textX, BATTERY_TEXT_Y);
    display.print(pctText);
  } else {
    iconX = DEVICE_DISPLAY_WIDTH - BATTERY_RIGHT_MARGIN - BATTERY_BODY_W - BATTERY_TIP_W;
  }

  drawBatteryIcon(
    iconX,
    BATTERY_ICON_Y,
    batteryPct >= 0 ? batteryPct : 0,
    BATTERY_BODY_W,
    BATTERY_BODY_H,
    BATTERY_TIP_W,
    BATTERY_TIP_H,
    themeMode
  );
  return iconX;
}

static void drawTitleLine(const char* title, int x, int y, int textSize, DisplayThemeMode themeMode = statusScreenThemeMode()) {
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  const int textH = 8 * textSize;
  const int markSize = textH + 8;
  const int markY = y - ((markSize - textH) / 2);
  drawBlockClockMark(x, markY, markSize, theme.foreground, theme.background);
  display.setTextSize(textSize);
  display.setCursor(x + markSize + (6 * textSize), y);
  display.print(title ? title : "");
}

static void drawStatusScreen(const char* title, const char* subtitle) {
  const DisplayThemeMode themeMode = statusScreenThemeMode();
  prepareScreen(themeMode);
  const int titleSize = 2;
  const int titleW = (title ? (int)strlen(title) : 0) * 6 * titleSize;
  const int iconSize = (8 * titleSize) + 8;
  const int groupW = titleW + iconSize + (6 * titleSize);
  const int x = (DEVICE_DISPLAY_WIDTH - groupW) / 2;
  const int y = (DEVICE_DISPLAY_HEIGHT / 2) - 14;
  drawTitleLine(title, x, y, titleSize, themeMode);
  display.setTextSize(1);
  display.setCursor(x + iconSize + (6 * titleSize), y + 26);
  display.print(subtitle ? subtitle : "");
  display.update();
}

static void animateEllipsis(int x, int y, DisplayThemeMode themeMode = statusScreenThemeMode()) {
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  display.fastmodeOn(false);
  for (int i = 0; i < 3; i++) {
    display.setTextSize(1);
    display.setTextColor(theme.foreground, theme.background);
    display.setCursor(x + (i * 6), y);
    display.print(".");
    display.update();
    if (i < 2) delay(500);
  }
  display.fastmodeOff();
  display.setTextColor(theme.foreground, theme.background);
}

static int drawBrandedStatusScreen(
  const char* title,
  const char* subtitle,
  bool animateSubtitleEllipsis
) {
  const DisplayThemeMode themeMode = statusScreenThemeMode();
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  prepareScreen(themeMode);

  static constexpr int TITLE_SIZE = 2;
  static constexpr int TITLE_CHAR_W = 6 * TITLE_SIZE;
  static constexpr int TITLE_CHAR_H = 8 * TITLE_SIZE;
  static constexpr int TITLE_ICON_SIZE = TITLE_CHAR_H + 8;
  static constexpr int TITLE_ICON_GAP = TITLE_CHAR_W;
  const int titleLen = title ? (int)strlen(title) : 0;
  const int centerY = DEVICE_DISPLAY_HEIGHT / 2;
  const int titleTextW = titleLen * TITLE_CHAR_W;
  const int titleGroupW = TITLE_ICON_SIZE + TITLE_ICON_GAP + titleTextW;
  const int titleX = (DEVICE_DISPLAY_WIDTH - titleGroupW) / 2;
  const int titleY = centerY - TITLE_CHAR_H - 4;
  const int titleIconY = titleY - ((TITLE_ICON_SIZE - TITLE_CHAR_H) / 2);
  drawBlockClockMark(titleX, titleIconY, TITLE_ICON_SIZE, theme.foreground, theme.background);

  display.setTextSize(TITLE_SIZE);
  const int titleTextX = titleX + TITLE_ICON_SIZE + TITLE_ICON_GAP;
  display.setCursor(titleTextX, titleY);
  display.print(title ? title : "");

  display.setTextSize(1);
  const int subtitleY = centerY + 4;
  display.setCursor(titleTextX, subtitleY);
  display.print(subtitle ? subtitle : "");

  display.update();
  if (animateSubtitleEllipsis && subtitle && subtitle[0] != '\0') {
    animateEllipsis(titleTextX + ((int)strlen(subtitle) * 6), subtitleY, themeMode);
  }
  return titleTextX;
}

static void drawWelcomeScreen() {
  drawStatusScreen("BLOCK CLOCK", "Press any button to begin");
}

static void portalQrRenderCallback(esp_qrcode_handle_t qrcode) {
  const DisplayThemeMode themeMode = statusScreenThemeMode();
  const DisplayThemeColors theme = getDisplayThemeColors(themeMode);
  static constexpr int QUIET_PX = 4;
  const int modules = esp_qrcode_get_size(qrcode);
  int modulePx = (DEVICE_DISPLAY_HEIGHT - 10 - 2 * QUIET_PX) / modules;
  if (modulePx < 2) modulePx = 2;
  if (modulePx > 4) modulePx = 4;
  const int contentPx = modules * modulePx;
  const int totalPx = contentPx + 2 * QUIET_PX;
  const int originX = DEVICE_DISPLAY_WIDTH - totalPx - 14;
  const int originY = (DEVICE_DISPLAY_HEIGHT - totalPx) / 2;
  display.fillRect(originX, originY, totalPx, totalPx, theme.background);
  for (int row = 0; row < modules; row++) {
    for (int col = 0; col < modules; col++) {
      if (esp_qrcode_get_module(qrcode, col, row)) {
        display.fillRect(
          originX + QUIET_PX + col * modulePx,
          originY + QUIET_PX + row * modulePx,
          modulePx,
          modulePx,
          theme.foreground
        );
      }
    }
  }
}

static void drawSetupPortalReadyScreen() {
  const DisplayThemeMode themeMode = statusScreenThemeMode();
  prepareScreen(themeMode);
  drawTitleLine("SETUP", 9, 9, 2, themeMode);

  display.setTextSize(1);
  display.setCursor(9, 40);
  display.print("Scan QR to join Wi-Fi,");
  display.setCursor(9, 54);
  display.print("then open 192.168.4.1");

  static constexpr int LABEL_X = 9;
  static constexpr int VALUE_X = 51;
#if DEVICE_PROFILE_E213
  display.setCursor(9, 80);
  display.print("Wi-Fi:");
  display.setCursor(9, 94);
  display.print(portalApSsid);
  display.setCursor(9, 108);
  display.print("PW:");
  display.setCursor(VALUE_X, 108);
  display.print(portalApPassword);
#else
  display.setCursor(LABEL_X, 80);
  display.print("Wi-Fi:");
  display.setCursor(VALUE_X, 80);
  display.print(portalApSsid);
  display.setCursor(LABEL_X, 94);
  display.print("PW:");
  display.setCursor(VALUE_X, 94);
  display.print(portalApPassword);
#endif

  char qrData[96];
  snprintf(qrData, sizeof(qrData), "WIFI:T:WPA;S:%s;P:%s;;", portalApSsid, portalApPassword);
  esp_qrcode_config_t qrCfg = {
    .display_func = portalQrRenderCallback,
    .max_qrcode_version = 5,
    .qrcode_ecc_level = ESP_QRCODE_ECC_LOW
  };
  esp_qrcode_generate(&qrCfg, qrData);

  display.update();
}

static void drawSetupSavedScreen() {
  drawBrandedStatusScreen("SETTINGS SAVED", "Restarting ", true);
}

static void drawFactoryResetScreen() {
  drawBrandedStatusScreen("FACTORY RESET", "Restarting ", true);
}

static int fitTextSizeForWidth(const char* text, int maxWidth, int preferredSize) {
  const int len = text ? (int)strlen(text) : 0;
  for (int size = preferredSize; size > 1; size--) {
    if (len * 6 * size <= maxWidth) return size;
  }
  return 1;
}

static void drawMetric(const char* value, const char* label, int x, int y, int textSize) {
  const int size = textSize;
  display.setTextSize(size);
  display.setCursor(x, y);
  display.print(value ? value : "--");
  display.setTextSize(1);
  display.setCursor(x, y + (8 * size) + 2);
  display.print(label ? label : "");
}

static void drawBlockDashboard(const BlockData& data, const DeviceConfig& cfg) {
  const DisplayThemeMode themeMode = sanitizeThemeMode(cfg.displayThemeMode);
  prepareScreen(themeMode);
  const int batteryIconX = drawBatteryStatus(data.batteryPercent, cfg.showBatteryPercent, themeMode);

  const int leftX = 10;
  const int rightX = DEVICE_DISPLAY_WIDTH / 2 + 8;
  const int colW = (DEVICE_DISPLAY_WIDTH / 2) - 18;
  const int row1Y = 17;
  const int row2Y = DEVICE_PROFILE_E213 ? 62 : 64;
  int metricSize = 3;
  metricSize = min(metricSize, fitTextSizeForWidth(data.blockHeight, colW, 3));
  metricSize = min(metricSize, fitTextSizeForWidth(data.hashrateEhs, colW, 3));
  metricSize = min(metricSize, fitTextSizeForWidth(data.blocksToHalving, colW, 3));
  metricSize = min(metricSize, fitTextSizeForWidth(data.priceValue, colW, 3));

  char priceLabel[20];
  snprintf(priceLabel, sizeof(priceLabel), "BTC PRICE %s", currencyCodeLabel(cfg.currencyCode));

  drawMetric(data.blockHeight, "BLOCK HEIGHT", leftX, row1Y, metricSize);
  drawMetric(data.hashrateEhs, "HASHRATE EH/s", rightX, row1Y, metricSize);
  drawMetric(data.blocksToHalving, "BLOCKS TO HALVING", leftX, row2Y, metricSize);
  drawMetric(data.priceValue, priceLabel, rightX, row2Y, metricSize);

  display.setTextSize(1);
  const int bottomY = DEVICE_DISPLAY_HEIGHT - 17;
  display.setCursor(10, bottomY);
  display.print("Updated ");
  const time_t nowT = time(nullptr);
  display.print(formatTimestamp(nowT >= 1700000000 ? nowT : data.updatedAt));

  if (!data.wifiOk) {
    drawNoWifiStatusLeftOfBattery(batteryIconX, themeMode);
  }

  display.update();
}
