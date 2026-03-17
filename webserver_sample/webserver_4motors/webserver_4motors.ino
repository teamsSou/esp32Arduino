#include "WiFi.h"
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WebServer.h>

// HTTPサーバインスタンス (ポート80)
WebServer server(80);

// WiFi 账号密码 (请替换为你自己的)
const char* ssid = "iPhone";
const char* password = "5nqpq9egrfhon";

// 全局变量：电机速度
int speed = 0;

// 使用 R"rawliteral(...)rawliteral" 编写前端 HTML
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>XIAO ESP32C6 モータコントローラ</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }
    h1 { color: #333; }
    .speed-display { font-size: 48px; font-weight: bold; color: #007BFF; margin: 20px; }
    button { padding: 15px 30px; font-size: 20px; margin: 10px; cursor: pointer; border: none; border-radius: 8px; background-color: #28a745; color: white; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
    button:active { background-color: #218838; transform: translateY(2px); box-shadow: none; }
    
    /* 減速ボタン（黄色）のスタイルを追加 */
    .btn-down { background-color: #ffc107; color: #333; }
    .btn-down:active { background-color: #e0a800; }

    /* 急停ボタン（赤色）のスタイル */
    .btn-stop { background-color: #dc3545; }
    .btn-stop:active { background-color: #c82333; }
  </style>
  <script>
    // 向开发板发送请求，并更新网页上显示的速度
    function changeSpeed(action) {
      fetch('/' + action)
        .then(response => response.text())
        .then(text => {
          console.log("Current Speed: " + text);
          document.getElementById('speed-val').innerText = text;
        });
    }
  </script>
</head>
<body>
  <h1>四軸モータ Web Controller</h1>
  <div>current速度 (PWM): <div class="speed-display" id="speed-val">0</div></div>
  
  <button onclick="changeSpeed('speed_up')">加速 (Speed +16)</button>
  <button class="btn-down" onclick="changeSpeed('speed_down')">減速 (Speed -16)</button>
  <button class="btn-stop" onclick="changeSpeed('speed_stop')">急停 (Speed = 0)</button>
</body>
</html>
)rawliteral";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// 根目录网页响应
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// 处理网页发来的“加速”请求
void handleSpeedUp() {
  speed += 16;
  if (speed > 255) {
    speed = 255;
  }
  Serial.print("Web Command: Speed Up -> ");
  Serial.println(speed);
  server.send(200, "text/plain", String(speed)); 

  analogWrite(D7, speed);
  analogWrite(D8, speed);
  analogWrite(D9, speed);
  analogWrite(D10, speed);
}

// 处理网页发来的“减速”请求
void handleSpeedDown() {
  speed -= 16;
  if (speed < 0) {
    speed = 0;
  }
  Serial.print("Web Command: Speed Down -> ");
  Serial.println(speed);
  server.send(200, "text/plain", String(speed)); 

  analogWrite(D7, speed);
  analogWrite(D8, speed);
  analogWrite(D9, speed);
  analogWrite(D10, speed);
}

// 处理网页发来的“急停”请求
void handleSpeedStop() {
  speed = 0;
  Serial.println("Web Command: STOP -> 0");
  server.send(200, "text/plain", String(speed));

  analogWrite(D7, speed);
  analogWrite(D8, speed);
  analogWrite(D9, speed);
  analogWrite(D10, speed);
}

// 独立的后台 Web 监听任务
void httpTask(void* pvParameters) {
  for (;;) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200); 
  delay(1000); 

  pinMode(D0, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);

  analogWrite(D7, 0);
  analogWrite(D8, 0);
  analogWrite(D9, 0);
  analogWrite(D10, 0);

  initWiFi();

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started. You can access via http://esp32.local");
  }

  server.on("/", handleRoot);          
  server.on("/speed_up", handleSpeedUp);     
  server.on("/speed_down", handleSpeedDown);     
  server.on("/speed_stop", handleSpeedStop); 

  server.begin();
  Serial.println("HTTP server started");
  
  xTaskCreate(httpTask, "HttpTask", 4096, NULL, 1, NULL);
}

void loop() {
}