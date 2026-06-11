#pragma once

static constexpr int BATTERY_BODY_W = 20;
static constexpr int BATTERY_BODY_H = 10;
static constexpr int BATTERY_TIP_W = 1;
static constexpr int BATTERY_TIP_H = 5;
static constexpr int BATTERY_RIGHT_MARGIN = 3;
static constexpr int BATTERY_TEXT_GAP = 2;
static constexpr int BATTERY_ICON_Y = 3;
static constexpr int BATTERY_TEXT_Y = 5;

static void prepareScreen() {
  display.setRotation(1);
  display.clear();
  display.setTextColor(BLACK);
}

static void drawBlockClockMark(int x, int y, int size) {
  display.drawRect(x, y, size, size, BLACK);
  display.drawRect(x + 2, y + 2, size - 4, size - 4, BLACK);
  display.fillRect(x + size - 5, y + 3, 3, 3, BLACK);
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
  int tipH
) {
  pct = clampInt(pct, 0, 100);
  display.drawRect(x, y, bodyW, bodyH, BLACK);
  const int tipY = y + ((bodyH - tipH) / 2);
  display.fillRect(x + bodyW, tipY, tipW, tipH, BLACK);

  const int innerW = bodyW - 4;
  const int fillW = (innerW * pct + 50) / 100;
  if (fillW > 0) {
    display.fillRect(x + 2, y + 2, fillW, bodyH - 4, BLACK);
  }
}

static int drawBatteryStatus(int batteryPct, bool showPercent) {
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
    BATTERY_TIP_H
  );
  return iconX;
}

static void drawTitleLine(const char* title, int x, int y, int textSize) {
  const int textH = 8 * textSize;
  const int markSize = textH + 6;
  drawBlockClockMark(x, y - 3, markSize);
  display.setTextSize(textSize);
  display.setCursor(x + markSize + 8, y);
  display.print(title ? title : "");
}

static void drawStatusScreen(const char* title, const char* subtitle) {
  prepareScreen();
  const int titleSize = 2;
  const int titleW = (title ? (int)strlen(title) : 0) * 6 * titleSize;
  const int groupW = titleW + 22 + 10;
  const int x = (DEVICE_DISPLAY_WIDTH - groupW) / 2;
  const int y = (DEVICE_DISPLAY_HEIGHT / 2) - 14;
  drawTitleLine(title, x, y, titleSize);
  display.setTextSize(1);
  display.setCursor(x + 32, y + 26);
  display.print(subtitle ? subtitle : "");
  display.update();
}

static void drawWelcomeScreen() {
  drawStatusScreen("BLOCK CLOCK", "Starting setup");
}

static void portalQrRenderCallback(esp_qrcode_handle_t qrcode) {
  static constexpr int QUIET_PX = 4;
  const int modules = esp_qrcode_get_size(qrcode);
  int modulePx = (DEVICE_DISPLAY_HEIGHT - 10 - 2 * QUIET_PX) / modules;
  if (modulePx < 2) modulePx = 2;
  if (modulePx > 4) modulePx = 4;
  const int contentPx = modules * modulePx;
  const int totalPx = contentPx + 2 * QUIET_PX;
  const int originX = DEVICE_DISPLAY_WIDTH - totalPx - 10;
  const int originY = (DEVICE_DISPLAY_HEIGHT - totalPx) / 2;
  display.fillRect(originX, originY, totalPx, totalPx, WHITE);
  for (int row = 0; row < modules; row++) {
    for (int col = 0; col < modules; col++) {
      if (esp_qrcode_get_module(qrcode, col, row)) {
        display.fillRect(
          originX + QUIET_PX + col * modulePx,
          originY + QUIET_PX + row * modulePx,
          modulePx,
          modulePx,
          BLACK
        );
      }
    }
  }
}

static void drawSetupPortalReadyScreen() {
  prepareScreen();
  drawTitleLine("SETUP", 9, 12, 2);

  display.setTextSize(1);
  display.setCursor(9, 40);
  display.print("Scan QR to join Wi-Fi");
  display.setCursor(9, 54);
  display.print("then open 192.168.4.1");

  display.setCursor(9, 80);
  display.print("Wi-Fi:");
  display.setCursor(9, 94);
  display.print(portalApSsid);
  display.setCursor(9, 108);
  display.print("PW:");
  display.setCursor(33, 108);
  display.print(portalApPassword);

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
  drawStatusScreen("SAVED", "Restarting");
}

static int fitTextSizeForWidth(const char* text, int maxWidth, int preferredSize) {
  const int len = text ? (int)strlen(text) : 0;
  for (int size = preferredSize; size > 1; size--) {
    if (len * 6 * size <= maxWidth) return size;
  }
  return 1;
}

static void drawMetric(const char* value, const char* label, int x, int y, int maxW, int preferredSize) {
  const int size = fitTextSizeForWidth(value, maxW, preferredSize);
  display.setTextSize(size);
  display.setCursor(x, y);
  display.print(value ? value : "--");
  display.setTextSize(1);
  display.setCursor(x, y + (8 * size) + 2);
  display.print(label ? label : "");
}

static void drawBlockDashboard(const BlockData& data, const DeviceConfig& cfg) {
  prepareScreen();
  drawBatteryStatus(data.batteryPercent, cfg.showBatteryPercent);

  const int leftX = 10;
  const int rightX = DEVICE_DISPLAY_WIDTH / 2 + 8;
  const int colW = (DEVICE_DISPLAY_WIDTH / 2) - 18;
  const int row1Y = 17;
  const int row2Y = DEVICE_PROFILE_E213 ? 62 : 64;

  drawMetric(data.blockHeight, "BLOCK HEIGHT", leftX, row1Y, colW, 3);
  drawMetric(data.hashrateEhs, "HASHRATE EH/s", rightX, row1Y, colW, 3);
  drawMetric(data.blocksToHalving, "BLOCKS TO HALVING", leftX, row2Y, colW, 3);
  drawMetric(data.priceUsd, "BTC PRICE USD", rightX, row2Y, colW, 3);

  display.setTextSize(1);
  const int bottomY = DEVICE_DISPLAY_HEIGHT - 17;
  display.setCursor(10, bottomY);
  display.print(dataSourceModeLabel(cfg.dataSourceMode));
  display.print(data.dataOk ? " updated " : " cached ");
  display.print(formatTimestamp(data.updatedAt));

  if (!data.wifiOk) {
    display.setCursor(10, bottomY + 10);
    display.print("Wi-Fi offline");
  }

  display.update();
}
