//Khai bao thư viện kết nối esp32 vs Wifi (Wifi.h)
//2 thư viện cho bộ đếm thời gian (freertos/FreeRTOS.h và freertos/timers.h); thư viện kết nối esp32 với mqtt broker (AsyncMqttClient.h)
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
#define MQTT_PUB_StaRlamp "StaRoomLamp"       //khai bao cac cau lenh tien xu li or tu khoa tien xu li
#define MQTT_PUB_StaDoor  "StaDoor"
#define MQTT_SUB_ControlRlamp "ControlRoomLamp"
#define MQTT_SUB_StaGas "Stagas"
#define MQTT_SUB_StaDh "StaDh"

//Định nghĩa các chân
const int Rlamp = 18;
const int Doorlamp = 19;//khai bao cac hang, bien so
const int Window = 21;                  
const int Hcsr501 = 25;
const int TTP223 = 27;

//Khai báo các biến, hằng số
int count1 = 0;
int StaRlamp;
bool Rlamp_Prv_state = false;

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
    mqttClient.subscribe(MQTT_SUB_StaGas, 2);
    mqttClient.subscribe(MQTT_SUB_StaDh, 2);
    mqttClient.subscribe(MQTT_SUB_ControlRlamp, 2);

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
            
          if (messageTemp == "ONRlamp"){
          digitalWrite(Rlamp, HIGH);
          }

          else if (messageTemp == "OFFRlamp"){
          digitalWrite(Rlamp, LOW);
          }

          else if (messageTemp == "Nguy hiểm"){
          digitalWrite(Window, HIGH); 
          }
          else if (messageTemp == "ON"){
          digitalWrite(Window, LOW); 
          }
          else{
          digitalWrite(Window, LOW); 
          }

}

//Hàm thông báo esp32 mất kết nối vs mqtt broker lên màn hình serial, tự động kết nối lại vs mqtt broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  //Serial.println("Disconnected from MQTT.");
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

  
  Serial.begin(115200);
  pinMode(Rlamp, OUTPUT);
  pinMode(Doorlamp, OUTPUT);
  pinMode(Window, OUTPUT);
  pinMode(TTP223, INPUT);
  pinMode(Hcsr501, INPUT);

  Serial.println();

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
  //bật tắt đèn phòng
  if(digitalRead(TTP223) == HIGH) {
  count1 = count1 + 1;
  if (count1 == 1){
  digitalWrite(Rlamp, HIGH);  
  }
  }
  if (count1 == 2){
  digitalWrite(Rlamp, LOW);
  count1 = 0;
  }
  delay (50);
  
  //bật tắt đèn cửa
  if(digitalRead(Hcsr501) == HIGH) {
  digitalWrite(Doorlamp, HIGH);  
  } else{
  digitalWrite(Doorlamp, LOW);
  }
  
  
  //publish trạng thái đèn phòng lên topic tương ứng
  StaRlamp = digitalRead(Rlamp);
  if (StaRlamp == HIGH && Rlamp_Prv_state == false)
  {
    // Publish an MQTT message on topic StaBnl
    uint16_t packetIdPub5 = mqttClient.publish(MQTT_PUB_StaRlamp, 1, true, "ON");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_StaBnl, packetIdPub5);
    //Serial.println(" Message: ON");
    Rlamp_Prv_state = true;
  }
  else if (StaRlamp == LOW && Rlamp_Prv_state == true)
  {
    // Publish an MQTT message on topic StaBnl
    uint16_t packetIdPub6 = mqttClient.publish(MQTT_PUB_StaRlamp, 1, true, "OFF");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_StaBnl, packetIdPub6);
    //Serial.println(" Message: OFF");
    Rlamp_Prv_state = false;
  }  

}