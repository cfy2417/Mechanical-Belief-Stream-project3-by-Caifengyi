#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include <DHT.h>
#include <driver/i2s.h>

// ========== 麦克风定义（I2S） ==========
#define I2S_WS  14
#define I2S_SD  32  // 替代 GPIO32 的 FSR
#define I2S_SCK 27

// ========== 传感器定义 ==========
#define NUM_FSR 5
const int fsrPins[NUM_FSR] = {36, 39, 33, 34, 26};  // 直接用GPIO引脚
#define LIGHT_SENSOR_PIN 35
#define DHTPIN 25
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ========== WiFi & 屏幕配置 ==========
const char* ssid = "Qifei";
const char* password = "88888888";
const char* postUrl  = "http://172.20.10.14:5050/data";
const char* imageUrl = "http://172.20.10.14:5050/image.jpg";

TFT_eSPI tft = TFT_eSPI();
const char* tempImagePath = "/temp.jpg";

// ========== I2S 初始化 ==========
void setupI2SMic() {
  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

// ========== 平均音量读取 ==========
float readMicVolume() {
  const int SAMPLES = 256;
  int16_t buffer[SAMPLES];
  size_t bytesRead;
  i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

  float sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += abs(buffer[i]);
  }
  return sum / SAMPLES;
}

// ========== 通用函数 ==========
bool downloadImageToSPIFFS(const char* url, const char* path) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Image HTTP GET failed, code=%d\n", httpCode);
    http.end();
    return false;
  }

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    http.end();
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buff[128] = {0};
  int len = http.getSize();
  int total = 0;

  while (http.connected() && (len > 0 || len == -1)) {
    size_t sizeAvailable = stream->available();
    if (sizeAvailable) {
      int c = stream->readBytes(buff, ((sizeAvailable > sizeof(buff)) ? sizeof(buff) : sizeAvailable));
      file.write(buff, c);
      total += c;
      if (len > 0) len -= c;
    }
    delay(1);
  }

  file.close();
  http.end();
  Serial.printf("Downloaded %d bytes\n", total);
  return true;
}

void drawJPEGFromSPIFFS(const char* path) {
  File jpgFile = SPIFFS.open(path, FILE_READ);
  if (!jpgFile) {
    Serial.println("Failed to open JPG file");
    return;
  }

  JpegDec.decodeSdFile(jpgFile);
  tft.setSwapBytes(true);
  uint16_t *pImg;
  int mcu_x, mcu_y, mcu_w, mcu_h;

  while (JpegDec.read()) {
    mcu_x = JpegDec.MCUx * JpegDec.MCUWidth;
    mcu_y = JpegDec.MCUy * JpegDec.MCUHeight;
    mcu_w = JpegDec.MCUWidth;
    mcu_h = JpegDec.MCUHeight;
    pImg = JpegDec.pImage;
    tft.pushImage(mcu_x, mcu_y, mcu_w, mcu_h, pImg);
  }

  jpgFile.close();
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  // 设置引脚模式为输入
  for (int i = 0; i < NUM_FSR; i++) {
    pinMode(fsrPins[i], INPUT);
  }
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(DHTPIN, INPUT);

  analogReadResolution(12);  // 12位ADC分辨率
  dht.begin();
  setupI2SMic();

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    while (1);
  }

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Connecting WiFi...", 10, 10);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  tft.fillScreen(TFT_BLACK);
  tft.drawString("WiFi Connected", 10, 10);
}

// ========== LOOP ==========
void loop() {
  int fsrValues[NUM_FSR];
  for (int i = 0; i < NUM_FSR; i++) {
    fsrValues[i] = analogRead(fsrPins[i]);  // 统一使用 analogRead
  }
  int lightValue = analogRead(LIGHT_SENSOR_PIN);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float volume = readMicVolume();

  // 构造 JSON 字符串
  String payload = "{";
  for (int i = 0; i < NUM_FSR; i++) {
    payload += "\"fsr" + String(i + 1) + "\":" + String(fsrValues[i]) + ",";
  }
  payload += "\"light\":" + String(lightValue) + ",";
  payload += "\"temperature\":" + String(temperature, 1) + ",";
  payload += "\"humidity\":" + String(humidity, 1) + ",";
  payload += "\"volume\":" + String(volume, 1);
  payload += "}";

  Serial.println(payload);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(postUrl);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(payload);
    Serial.printf("POST response: %d\n", httpCode);
    http.end();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (downloadImageToSPIFFS(imageUrl, tempImagePath)) {
      drawJPEGFromSPIFFS(tempImagePath);
    } else {
      Serial.println("Failed to download image");
    }
  }

  delay(20);
}