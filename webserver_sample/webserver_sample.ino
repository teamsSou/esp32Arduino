#include "WiFi.h"
//https://garretlab.web.fc2.com/arduino/esp32/reference/libraries/HTTPClient/
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ESPmDNS.h>

//WEBSERVER Sample
#include <WebServer.h>
// HTTPサーバインスタンス (ポート80)
WebServer server(80);

// WIFI利用例:https://wiki.seeedstudio.com/ja/xiao_wifi_usage_esp32c6/
// Replace with your network credentials
const char* ssid = "sou-F2886Q-tb3H-A";
const char* password = "HdYFCHKd7hfkf";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
}

// ルートハンドラ
void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP32 HTTP Server!</h1>"+WiFi.localIP());
}
void httpTask(void* pvParameters) {
  for (;;) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  delay(1000); // Wait for the serial port to connect

  //WIFI
  initWiFi();

    //ローカルドメインサービス http://esp32.local
  MDNS.begin("esp32");

  // ハンドラ登録
  server.on("/", handleRoot);

  // サーバ開始
  server.begin();
  xTaskCreate(httpTask,   "HttpTask",   4096, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
