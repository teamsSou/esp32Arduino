#include "WiFi.h"
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WebServer.h>

// HTTPサーバインスタンス (ポート80)
WebServer server(80);

// WiFi アカウントとパスワード
const char* ssid = "iPhone";
const char* password = "5nqpq9egrfhon";

// グローバル変数：モータースピード
int speed = 0;

// R"rawliteral(...)rawliteral" を使ったフロントエンドHTML
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
    
    /* 減速ボタン（黄色）のスタイル */
    .btn-down { background-color: #ffc107; color: #333; }
    .btn-down:active { background-color: #e0a800; }

    /* 急停ボタン（赤色）のスタイル */
    .btn-stop { background-color: #dc3545; }
    .btn-stop:active { background-color: #c82333; }
  </style>
  <script>
    // 開発ボードへリクエストを送り、Web上の速度表示を更新する
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
  
  <button onclick="changeSpeed('speed_up')">加速 (Speed +64)</button>
  <button class="btn-down" onclick="changeSpeed('speed_down')">減速 (Speed -64)</button>
  <button class="btn-stop" onclick="changeSpeed('speed_stop')">急停 (Speed = 0)</button>
</body>
</html>
)rawliteral";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  
  // Wi-Fi接続中は内蔵LEDを点滅させる
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); //LEDの状態をReadをした後にLED_BUILTINにWrite☆
    delay(500); // 0.5秒待機
  }
  
  // 接続完了したらLEDを「常時点灯」にする（XIAOはLOWで点灯します）
  digitalWrite(LED_BUILTIN, LOW); //☆digitalWrite(LED_BUILTIN,LOW又はHIGH)=digitalWrite(LOW)点、digitalWrite(HIGH)滅
  
  Serial.println();
  Serial.println("WiFi Connected! [Flight Ready]");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ルートディレクトリの応答
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// 加速リクエストの処理
void handleSpeedUp() {
  speed += 64;
  if (speed > 255) {
    speed = 255; // 上限を255でストップ
  }
  Serial.print("Web Command: Speed Up -> ");
  Serial.println(speed);
  server.send(200, "text/plain", String(speed)); 

  analogWrite(D9, speed);
  analogWrite(D5, speed);
  analogWrite(D8, speed);
  analogWrite(D4, speed);
}

// 減速リクエストの処理
void handleSpeedDown() {
  speed -= 64;
  if (speed < 0) {
    speed = 0; // 下限を0でストップ
  }
  Serial.print("Web Command: Speed Down -> ");
  Serial.println(speed);
  server.send(200, "text/plain", String(speed)); 

  analogWrite(D9, speed);
  analogWrite(D5, speed);
  analogWrite(D8, speed);
  analogWrite(D4, speed);
}

// 急停リクエストの処理
void handleSpeedStop() {
  speed = 0;
  Serial.println("Web Command: STOP -> 0");
  server.send(200, "text/plain", String(speed));

  analogWrite(D9, speed);
  analogWrite(D5, speed);
  analogWrite(D8, speed);
  analogWrite(D4, speed);
}

// 独立したバックグラウンドWebリスニングタスク
void httpTask(void* pvParameters) {
  for (;;) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200); 
  delay(1000); 

  // 内蔵LEDの初期設定（最初は消灯させておく：HIGHで消灯）
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 

  pinMode(D0, INPUT_PULLUP);
  pinMode(D9, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D4, OUTPUT);

  // 起動時にモーターが回らないように確実に0にする
  analogWrite(D9, 0);
  analogWrite(D5, 0);
  analogWrite(D8, 0);
  analogWrite(D4, 0);

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
  // 物理ボタンの処理などが必要な場合はここに記述
}