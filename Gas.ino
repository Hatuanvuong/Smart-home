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
#define MQTT_PUB_GAS "Nongdogas"
#define MQTT_PUB_Stagas "Stagas"

//Định nghĩa các chân    
const int Coi = 18;
const int Quat = 19;
const int Gas_analog = 34;                     
const int Gas_digital = 32;

//Khai báo các biến, hằng số
int Valueana = 0;
int Valuedig = 0;

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

//ham nay dc goi khi da ket noi dc vs mqtt broker, in ra man hinh 1 so cai, dang ki esp32 vs cac topic can thiet
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  //Serial.print("Session present: ");
  //Serial.println(sessionPresent);

}



//khi esp32 mat ket noi vs mqtt broker ham nay se dc goi de thong bao va cố gắng kết nối lại vs mqtt broker  
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
  pinMode(Coi, OUTPUT);
  pinMode(Quat, OUTPUT);  
  pinMode(Gas_digital, INPUT);
 
  Serial.println();
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  
  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectToWifi();
}

//vòng lặp vô hạn
void loop() {

//điều khiển còi quạt
  Valueana = analogRead(Gas_analog);
  Valuedig = digitalRead(Gas_digital);

  //Serial.print("Gas Sensor: ");
  //Serial.print(Valueana);
  //Serial.print("\t");
  //Serial.print("Gas Class: ");
  //Serial.print(Valuedig);
  //Serial.print("\t");
  //Serial.print("\t");
  
  if (Valueana > 1500) {
    //Serial.println("Gas");
    digitalWrite (Coi, HIGH); 
    delay(500);
    digitalWrite (Coi, LOW) ;
    digitalWrite (Quat, HIGH);
  }
  else {
    //Serial.println("No Gas");
    digitalWrite (Quat, LOW);
  }
  
  //publish dữ liệu nồng độ gas, trạng thái gas lên topic tương ứng
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
     previousMillis = currentMillis;
    //Publish an MQTT message on topic esp32/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_GAS, 1, true, String(Valueana).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_GAS, packetIdPub1);
    Serial.printf("Message: %d ",Valueana);


  if ((Valueana >=500)&&(Valueana<=1500))
  {
    // Publish an MQTT message on topic StaDh
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_Stagas, 1, true, "An toàn");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_StaDh, packetIdPub3);
    //Serial.println(" Message: ON");
  }
  if ((Valueana > 1500))
  {
    // Publish an MQTT message on topic StaDh
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_Stagas, 1, true, "Nguy hiểm");
    //Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_StaDh, packetIdPub4);
    //Serial.println(" Message: OFF");
  }
    
  }
}