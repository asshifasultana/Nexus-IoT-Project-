#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// =======================
// CAMERA MODEL
// =======================
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// =======================
// WiFi credentials
// =======================
const char *ssid     = "Device name";
const char *password = "password";

// =======================
// Telegram
// =======================
#define BOT_TOKEN  " Token number"
#define CHAT_ID   "ID"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // =======================
  // CAMERA CONFIG
  // =======================
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.fb_count    = 2;
    config.jpeg_quality = 10;
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count    = 1;
    config.jpeg_quality = 12;
  }

  // =======================
  // INIT CAMERA
  // =======================
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  Serial.println("Camera initialized");

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_QVGA);

  // =======================
  // WIFI CONNECT
  // =======================
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // =======================
  // START CAMERA SERVER
  // =======================
  startCameraServer();

  String ip = WiFi.localIP().toString();
  Serial.println("Camera Ready! IP: " + ip);

  // =======================
  // SEND IP TO TELEGRAM
  // =======================
  secured_client.setInsecure();

  String msg = "📷 *Camera Online!*\n";
         msg += "🌐 Stream: http://" + ip + ":81/stream\n";
         msg += "🎛️ Control: http://" + ip;

  bool ok = bot.sendMessage(CHAT_ID, msg, "Markdown");

  if (ok) {
    Serial.println("Telegram message sent!");
  } else {
    Serial.println("Telegram failed, retrying...");
    delay(2000);
    bot.sendMessage(CHAT_ID, msg, "Markdown");
  }
}

void loop() {
  delay(10000);
}
