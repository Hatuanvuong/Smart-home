//Khai bao thư viện sử dụng cảm biến DHT11 (DHT.h); thư viện kết nối esp32 vs Wifi (Wifi.h)
//2 thư viện cho bộ đếm thời gian (freertos/FreeRTOS.h và freertos/timers.h); thư viện kết nối esp32 với mqtt broker (AsyncMqttClient.h)
#include "DHT.h"
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

//Khai báo thông tin mạng wifi sử dụng
#define WIFI_SSID "Tuan Truong"
#define WIFI_PASSWORD "0362972868"

//Khai báo thông tin mqtt broker
#define MQTT_HOST IPAddress(192, 168, 1, 2)               
#define MQTT_PORT 1883

//Khai báo các topic mà esp32 publish và subscribe
#define MQTT_PUB_TEMP "esp32/temperature"       
#define MQTT_PUB_HUM  "esp32/humidity"
#define MQTT_PUB_StaDh "StaDh"
#define MQTT_PUB_StaBnl "StaBnl"
#define MQTT_SUB_OutputDh "OutputControlDh"
#define MQTT_SUB_OutputBnl "OutputControlBnl"

//Định nghĩa các chân
const int LEDPINDh = 18;
const int LEDPINBnl = 19;                    
const int INTTP2231 = 34;
const int INTTP2232 = 35;

//Khai báo các biến, hằng số
int count1 = 0;
int count2 = 0;
int StaDh, StaBnl;
bool Dh_Prv_state = false;
bool Bnl_Prv_state = false;
#define DHTPIN 4  
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
float temp;
float hum;

//Khai báo biến mqttClient để quản lí các client
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;   
const long interval = 5000;

//Hàm kết nối esp32 vs wifi và hiển thị lên màn hình serial thông báo esp32 đang kết nối vs wifi
void connectToWifi() {                              
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

//Hàm kết nối esp32 vs mqtt broker và hiển thị lên màn hình serial thông báo esp32 đang kết nối vs mqtt broker
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

//Hàm thông báo esp32 đã kết nối thành công vs wifi hoặc đã ngắt kết nối vs wifi lên màn hình serial. Tự kết nối lại vs wifi khi mất kết nối
void WiFiEvent(WiFiEvent_t event) {
     //Serial.printf("[WiFi-event] event: %dn", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      //Serial.println("IP address: ");
      //Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); 
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

//Hàm thông báo esp32 đã kết nối thành công vs mqtt broker lên màn hình serial ; đăng kí esp32 vs những topic cần thiết
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  //Serial.print("Session present: ");
  //Serial.println(sessionPresent);

 /* uint16_t packetIdSub = mqttClient.subscribe(MQTT_PUB_StaDh, 2);
    Serial.print("Subscribing at QoS 2, packetId: ");
    Serial.println(packetIdSub);*/
    mqttClient.subscribe(MQTT_SUB_OutputDh, 2);
    mqttClient.subscribe(MQTT_SUB_OutputBnl, 2);

}

//Hàm in ra màn hình serial 1 số thông tin của topic mà esp32 đăng kí
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  //Serial.println("Subscribe acknowledged.");
  //Serial.print("  packetId: ");
  //Serial.println(packetId);
  //Serial.print("  qos: ");
  //Serial.println(qos);
}

//Hàm thông báo esp32 hủy đăng kí 1 topic lên màn hình serial 
void onMqttUnsubscribe(uint16_t packetId) {
  //Serial.println("Unsubscribe acknowledged.");
  //Serial.print("  packetId: ");
  //Serial.println(packetId);
}

//Hàm này như 1 chương trình phụ ngắt, khi có tín hiệu điều khiển từ 1 topic khác mà esp32 đăng kí, nó sẽ nhảy vào đây để thực hiên lệnh
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
          //Serial.println("\n Publish received.");
          //Serial.print("topic: ");
          //Serial.println(topic);
          String messageTemp;
          for (int i = 0; i < len; i++) {
            messageTemp += (char)payload[i];
          }
            //Serial.print("Message: ");
            //Serial.println(messageTemp);
            
          if (messageTemp == "ONDh"){
          digitalWrite(LEDPINDh, HIGH);
          //Serial.println("Dh is now ON!");
      
          }

          else if (messageTemp == "OFFDh"){
          digitalWrite(LEDPINDh, LOW);
          //Serial.println("Dh is now OFF");
          }

          else if (messageTemp == "ONBnl"){
          digitalWrite(LEDPINBnl, HIGH); 
          //Serial.println("Bnl is now ON!");
          }
          
          else{
          digitalWrite(LEDPINBnl, LOW); 
          //Serial.println("Bnl is now OFF");
          }

}

//Hàm thông báo esp32 mất kết nối vs mqtt broker lên màn hình serial, tự động kết nối lại vs mqtt broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

//Hàm in ra màn hình serial thông tin 1 số topic mà esp32 publish
void onMqttPublish(uint16_t packetId) {
  //Serial.print("Publish acknowledged.");
  //Serial.print("  packetId: ");
  //Serial.println(packetId);
}

//Hàm chuẩn bị: kết nối nối tiếp esp32 vs máy tính, cấu hình chân, gọi hàm
void setup() {

  Serial.begin(600);
  pinMode(LEDPINDh, OUTPUT);
  pinMode(LEDPINBnl, OUTPUT);  
  pinMode(INTTP2231, INPUT);
  pinMode(INTTP2232, INPUT);

  Serial.println();

  dht.begin();
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectToWifi();
}

//Vòng lặp vô hạn
void loop() {

  //bật tắt điều hòa
  if(digitalRead(INTTP2231) == HIGH) {
  count1 = count1 + 1;
  if (count1 == 1){
  digitalWrite(LEDPINDh, HIGH);  
  }
  }
  if (count1 == 2){
  digitalWrite(LEDPINDh, LOW);
  count1 = 0;
  }
  delay (50);
  //bặt tắt bình nóng lạnh
  if(digitalRead(INTTP2232) == HIGH) {
  count2 = count2 + 1;
  if (count2 == 1){
  digitalWrite(LEDPINBnl, HIGH);  
  }
  }
  if (count2 == 2){
  digitalWrite(LEDPINBnl, LOW);
  count2 = 0;
  }
  delay (50);

  //publish dữ liệu nhiệt độ độ ẩm lên các topic tương ứng
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    
    if (isnan(temp) || isnan(hum)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
    //Serial.printf("Message: %.2f n", temp);

    // Publish an MQTT message on topic esp32/humidity
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
    //Serial.printf("Message: %.2f n", hum);
  }

  //publish trạng thái on off của điều hòa và bình nóng lạnh lên các topic tương ứng
  StaDh = digitalRead(LEDPINDh);
  if (StaDh == HIGH && Dh_Prv_state == false)
  {
    // Publish an MQTT message on topic StaDh
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_StaDh, 1, true, "ON");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_StaDh, packetIdPub3);
    //Serial.println(" Message: ON");
    Dh_Prv_state = true;
  }
  else if (StaDh == LOW && Dh_Prv_state == true)
  {
    // Publish an MQTT message on topic StaDh
    uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_StaDh, 1, true, "OFF");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_StaDh, packetIdPub4);
    //Serial.println(" Message: OFF");
    Dh_Prv_state = false;
  }

  StaBnl = digitalRead(LEDPINBnl);
  if (StaBnl == HIGH && Bnl_Prv_state == false)
  {
    // Publish an MQTT message on topic StaBnl
    uint16_t packetIdPub5 = mqttClient.publish(MQTT_PUB_StaBnl, 1, true, "ON");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_StaBnl, packetIdPub5);
    //Serial.println(" Message: ON");
    Bnl_Prv_state = true;
  }
  else if (StaBnl == LOW && Bnl_Prv_state == true)
  {
    // Publish an MQTT message on topic StaBnl
    uint16_t packetIdPub6 = mqttClient.publish(MQTT_PUB_StaBnl, 1, true, "OFF");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_StaBnl, packetIdPub6);
    //Serial.println(" Message: OFF");
    Bnl_Prv_state = false;
  }  

}