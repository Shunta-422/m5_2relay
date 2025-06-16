#include <M5GFX.h>
#include <Wire.h>

// I2C設定
#define I2C_SDA 12
#define I2C_SCL 11
#define MODULE_2RELAY_ADDR 0x25
#define MODULE_2RELAY_REG  0x00
#define MODULE_2RELAY_VERSION_REG 0xFE
#define MODULE_2RELAY_ADDR_CONFIG_REG 0xFF

// 関数プロトタイプ宣言
bool writeBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
bool begin(TwoWire* wire, uint8_t sda, uint8_t scl, uint8_t addr);
bool getRelayState(uint8_t index);
bool setRelay(uint8_t index, bool state);
void setupButtons();
void drawButtons();
int checkButtonTouch(int x, int y);
void onAllOnPressed();
void onAllOffPressed();
void onRelay0Pressed();
void onRelay1Pressed();
void updateButtonStates();

M5GFX display;

// ボタンの定義構造体
struct TouchButton {
  int x, y, w, h;        // ボタンの位置とサイズ
  String label;          // ボタンのラベル
  bool pressed;          // 押下状態
  bool isActive;         // アクティブ状態（リレーのON/OFF）
};

// 4つのタッチボタンを定義
TouchButton buttons[4];

// ボタンフィードバック用タイマー
unsigned long buttonPressTime = 0;
int pressedButtonIndex = -1;

/**
 * @brief I2C通信の初期化
 */
bool begin(TwoWire* wire, uint8_t sda, uint8_t scl, uint8_t addr) {
  wire->begin(sda, scl);
  delay(10);
  wire->beginTransmission(addr);
  uint8_t error = wire->endTransmission();
  return (error == 0);
}

/**
 * @brief I2Cデバイスにデータを書き込み
 */
bool writeBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(buffer, length);
  return (Wire.endTransmission() == 0);
}

/**
 * @brief I2Cデバイスからデータを読み込み
 */
bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length) {
  uint8_t index = 0;
  
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  
  if(Wire.requestFrom(addr, length)) {
    for(uint8_t i = 0; i < length; i++) {
      buffer[index++] = Wire.read();
    }
    return true;
  }
  return false;
}

/**
 * @brief 指定したリレーの状態を取得
 */
bool getRelayState(uint8_t index) {
  uint8_t data = 0;
  if(readBytes(MODULE_2RELAY_ADDR, MODULE_2RELAY_REG + index, &data, 1)) {
    return (data != 0);
  }
  return false;
}

/**
 * @brief 指定したリレーの状態を設定
 */
bool setRelay(uint8_t index, bool state) {
  uint8_t data = state ? 0xFF : 0x00;
  bool success = writeBytes(MODULE_2RELAY_ADDR, MODULE_2RELAY_REG + index, &data, 1);
  
  if (success) {
    Serial.printf("Relay%d set to %s\n", index, state ? "ON" : "OFF");
  } else {
    Serial.printf("Failed to set Relay%d\n", index);
  }
  
  return success;
}

/**
 * @brief ボタンの初期設定
 */
void setupButtons() {
  int buttonWidth = display.width() / 2 - 20;
  int buttonHeight = 60;
  int margin = 10;
  
  // 左上: ALL ON
  buttons[0] = {margin, margin, buttonWidth, buttonHeight, "ALL ON", false, false};
  
  // 右上: ALL OFF  
  buttons[1] = {display.width() / 2 + margin, margin, buttonWidth, buttonHeight, "ALL OFF", false, false};
  
  // 左下: Relay0
  buttons[2] = {margin, display.height() - buttonHeight - margin, buttonWidth, buttonHeight, "Relay0", false, false};
  
  // 右下: Relay1
  buttons[3] = {display.width() / 2 + margin, display.height() - buttonHeight - margin, buttonWidth, buttonHeight, "Relay1", false, false};
}

/**
 * @brief リレーの実際の状態でボタンの状態を更新
 */
void updateButtonStates() {
  // 個別リレーの状態を更新
  buttons[2].isActive = getRelayState(0);  // Relay0
  buttons[3].isActive = getRelayState(1);  // Relay1
  
  // ALL ONボタン: 両方のリレーがONの場合のみアクティブ
  buttons[0].isActive = buttons[2].isActive && buttons[3].isActive;
  
  // ALL OFFボタン: 両方のリレーがOFFの場合のみアクティブ
  buttons[1].isActive = !buttons[2].isActive && !buttons[3].isActive;
}

/**
 * @brief ボタンを描画
 */
void drawButtons() {
  display.clear();
  
  for (int i = 0; i < 4; i++) {
    uint32_t bgColor, textColor;
    
    // ボタンの状態に応じて色を決定
    if (buttons[i].pressed) {
      // 押下中
      bgColor = TFT_RED;
      textColor = TFT_WHITE;
    } else if (buttons[i].isActive) {
      // アクティブ状態（リレーON）
      bgColor = TFT_GREEN;
      textColor = TFT_BLACK;
    } else {
      // 非アクティブ状態（リレーOFF）
      bgColor = TFT_LIGHTGRAY;
      textColor = TFT_BLACK;
    }
    
    // ボタンの枠を描画
    display.fillRoundRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 8, bgColor);
    display.drawRoundRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 8, TFT_BLACK);
    
    // ボタンのテキストを描画
    display.setTextColor(textColor);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString(buttons[i].label, 
                      buttons[i].x + buttons[i].w / 2, 
                      buttons[i].y + buttons[i].h / 2);
  }
  
  // ステータス表示
  display.setTextColor(TFT_BLACK);
  display.setTextDatum(textdatum_t::top_left);
  display.setCursor(10, display.height() - 30);
  display.printf("R0:%s R1:%s", 
                getRelayState(0) ? "ON" : "OFF",
                getRelayState(1) ? "ON" : "OFF");
  
  display.display();
}

/**
 * @brief タッチ位置がボタン内かどうかを判定
 */
int checkButtonTouch(int x, int y) {
  for (int i = 0; i < 4; i++) {
    if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].w &&
        y >= buttons[i].y && y <= buttons[i].y + buttons[i].h) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief ボタン押下時の視覚的フィードバックを開始
 */
void startButtonFeedback(int buttonIndex) {
  if (buttonIndex >= 0 && buttonIndex < 4) {
    buttons[buttonIndex].pressed = true;
    pressedButtonIndex = buttonIndex;
    buttonPressTime = millis();
    drawButtons();
  }
}

/**
 * @brief ボタン押下フィードバックの終了チェック
 */
void checkButtonFeedback() {
  if (pressedButtonIndex >= 0 && millis() - buttonPressTime > 200) {
    buttons[pressedButtonIndex].pressed = false;
    pressedButtonIndex = -1;
    updateButtonStates();
    drawButtons();
  }
}

/**
 * @brief ALL ON ボタンが押された時の処理
 */
void onAllOnPressed() {
  Serial.println("ALL ON button pressed!");
  
  bool success = true;
  success &= setRelay(0, true);
  success &= setRelay(1, true);
  
  if (!success) {
    Serial.println("Error: Failed to turn on all relays");
  }
  
  startButtonFeedback(0);
}

/**
 * @brief ALL OFF ボタンが押された時の処理
 */
void onAllOffPressed() {
  Serial.println("ALL OFF button pressed!");
  
  bool success = true;
  success &= setRelay(0, false);
  success &= setRelay(1, false);
  
  if (!success) {
    Serial.println("Error: Failed to turn off all relays");
  }
  
  startButtonFeedback(1);
}

/**
 * @brief Relay0 ボタンが押された時の処理
 */
void onRelay0Pressed() {
  Serial.println("Relay0 button pressed!");
  
  bool currentState = getRelayState(0);
  bool newState = !currentState;
  
  if (setRelay(0, newState)) {
    Serial.printf("Relay0 toggled to %s\n", newState ? "ON" : "OFF");
  } else {
    Serial.println("Error: Failed to toggle Relay0");
  }
  
  startButtonFeedback(2);
}

/**
 * @brief Relay1 ボタンが押された時の処理
 */
void onRelay1Pressed() {
  Serial.println("Relay1 button pressed!");
  
  bool currentState = getRelayState(1);
  bool newState = !currentState;
  
  if (setRelay(1, newState)) {
    Serial.printf("Relay1 toggled to %s\n", newState ? "ON" : "OFF");
  } else {
    Serial.println("Error: Failed to toggle Relay1");
  }
  
  startButtonFeedback(3);
}

void setup() {
  Serial.begin(115200);
  
  display.init();
  display.begin();
  display.setFont(&fonts::lgfxJapanGothic_12);
  // ボタンの設定
  setupButtons();
  
  // 初期状態を更新して画面描画
  updateButtonStates();
  drawButtons();
  
  display.startWrite();
  
  // リレーモジュールの初期化
  display.println("Initializing AC 2RELAY...");
  while(!begin(&Wire, I2C_SDA, I2C_SCL, MODULE_2RELAY_ADDR)) {
    display.println("AC 2RELAY INIT ERROR");
    Serial.println("AC 2RELAY INIT ERROR");
    delay(1000);
  }
  display.println("AC 2RELAY INIT OK");
  Serial.println("AC 2RELAY INIT OK");
  
  delay(1000);
  
  
}

void loop() {
  static bool lastTouchState = false;
  lgfx::touch_point_t tp[1];

  // ボタンフィードバックの管理
  checkButtonFeedback();

  // タッチ検出
  int nums = display.getTouchRaw(tp, 1);
  
  if (nums > 0) {
    // タッチ座標を変換
    display.convertRawXY(tp, nums);
    
    // 新しいタッチの場合のみ処理
    if (!lastTouchState) {
      int buttonIndex = checkButtonTouch(tp[0].x, tp[0].y);
      
      if (buttonIndex >= 0) {
        // ボタンが押された時の処理を実行
        switch (buttonIndex) {
          case 0:
            onAllOnPressed();
            break;
          case 1:
            onAllOffPressed();
            break;
          case 2:
            onRelay0Pressed();
            break;
          case 3:
            onRelay1Pressed();
            break;
        }
      }
    }
    lastTouchState = true;
  } else {
    lastTouchState = false;
  }
  
  delay(50);  // CPU負荷軽減（100ms→50msに短縮）
}