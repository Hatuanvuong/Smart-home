# Smart-home
Hướng dẫn sử dụng code
B1: Cài đặt và làm quen với các phần mềm, platform: Arduino IDE, Mosquitto, Node red, Blynk.
- Cài đặt và làm quen vs phần mềm Arduino IDE: https://www.youtube.com/watch?v=xFz7UVDwXGI&t=200s
- Cài đặt và làm quen vs phần mềm Node red: https://www.youtube.com/watch?v=p74GGc5CZoc
- Cài đặt và làm quen với phần mềm Mosquitto: https://www.youtube.com/watch?v=xLLFrLhegcw&t=343s
- Làm quen với platform Blynk: https://www.youtube.com/watch?v=JZzspkuMN94
B2: Dowload các thư viên có trong link driver sau:
https://drive.google.com/drive/folders/1BQWa2-bdjZ_OdMBaVmTlzBRBKNlH_sGr?usp=sharing
B3: Thiết lập code trong phần mềm Aduino IDE để nạp cho ESP32
- Mở phần mềm Aduino IDE, thêm các thư viện đã đề cập ở B2 và thêm 2 thư viện DHT library from Adafruit và Adafruit Unified Sensor library có sẵn trong mục thư viện online của phần mềm
- Copy paste code (có 3 hệ thống nhỏ tương ứng vs 3 source code)
- Build code và nạp code vào ESP32
B4: Lập trình trên phần mềm Node red
- Import source code sau: 
[{"id":"5a45b8da.52b0d8","type":"mqtt in","z":"b01416d3.f69f38","name":"","topic":"esp32/dht/temperature","qos":"1","datatype":"auto","broker":"8db3fac0.99dd48","x":300,"y":60,"wires":[["3042e15e.80a4ee"]]},{"id":"3042e15e.80a4ee","type":"ui_gauge","z":"b01416d3.f69f38","name":"","group":"37de8fe8.46846","order":2,"width":0,"height":0,"gtype":"gage","title":"Temperature","label":"ºC","format":"{{value}}","min":0,"max":"40","colors":["#00b500","#f7df09","#ca3838"],"seg1":"","seg2":"","x":590,"y":60,"wires":[]},{"id":"8ff168f0.0c74a8","type":"mqtt in","z":"b01416d3.f69f38","name":"","topic":"esp32/dht/humidity","qos":"1","datatype":"auto","broker":"8db3fac0.99dd48","x":290,"y":140,"wires":[["29251f29.6687c"]]},{"id":"29251f29.6687c","type":"ui_gauge","z":"b01416d3.f69f38","name":"","group":"37de8fe8.46846","order":2,"width":0,"height":0,"gtype":"gage","title":"Humidity","label":"%","format":"{{value}}","min":"30","max":"100","colors":["#53a4e6","#1d78a9","#4e38c9"],"seg1":"","seg2":"","x":580,"y":140,"wires":[]},{"id":"8db3fac0.99dd48","type":"mqtt-broker","z":"","name":"","broker":"localhost","port":"1883","clientid":"","usetls":false,"compatmode":false,"keepalive":"60","cleansession":true,"birthTopic":"","birthQos":"0","birthPayload":"","closeTopic":"","closeQos":"0","closePayload":"","willTopic":"","willQos":"0","willPayload":""},{"id":"37de8fe8.46846","type":"ui_group","z":"","name":"DHT","tab":"53b8c8f9.cfbe48","order":1,"disp":true,"width":"6","collapse":false},{"id":"53b8c8f9.cfbe48","type":"ui_tab","z":"","name":"Home","icon":"dashboard","order":2,"disabled":false,"hidden":false}]
B5: Lập trình trên Blynk
Thiết lập và kết nối các luồng dữ liệu từ Blynk đến Node red 
B6: Chạy hệ thống
