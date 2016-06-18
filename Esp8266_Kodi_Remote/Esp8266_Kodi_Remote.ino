#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>


// Cấu hình Wifi, sửa lại theo đúng mạng Wifi của bạn
const char* WIFI_SSID = "Sandiego";
const char* WIFI_PWD = "0988807067";

// Chân nối với mắt nhận hồng ngoại là D4
const int IR_RECV_PIN = D4;

// Cấu hình các chân nối với module thu sóng RF
const int RF_D0 = D2;
const int RF_D1 = D1;
const int RF_D2 = D7;
const int RF_D3 = D6;
const int RF_VT = D5; // chân này sẽ active khi có tín hiệu thu được

// Chân nối với mắt phát hồng ngoại là D8
const int IR_SEND_PIN = 15;
IRrecv irrecv(IR_RECV_PIN);
IRsend irsend(IR_SEND_PIN);

// Lệnh phát hồng ngoại để bật tắt đèn
// Dùng code mẫu trong File\Examples\IRremoteESP8266\IRrecvDumpV2 để học lệnh
unsigned int  acOnCommand[211] = {3850,1800, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,400, 500,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,400, 500,1350, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,1350, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,1300, 550,400, 500,400, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,1350, 550,1300, 550,1300, 550,1350, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 500,400, 550};  // PANASONIC 555A:F3080088
unsigned int  acOffCommand[211] = {3850,1800, 550,350, 550,1350, 550,350, 550,1350, 500,400, 550,1300, 600,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,350, 550,1350, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,400, 550,1350, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 500,450, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 500,400, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550};

unsigned int tvOnCommand[67] = {9250,4550, 550,650, 550,600, 600,600, 550,1750, 600,600, 600,650, 550,600, 600,600, 550,1750, 600,1750, 550,1750, 550,650, 550,1750, 550,1750, 600,600, 550,1750, 600,600, 550,1750, 600,600, 550,650, 550,600, 600,600, 550,650, 550,600, 600,1750, 550,600, 600,1750, 550,1700, 600,1750, 600,1750, 550,1750, 550,1750, 600};  // UNKNOWN 53927E85
unsigned int tvOffCommand[67] = {9250,4550, 550,650, 550,600, 600,600, 550,1750, 600,600, 600,650, 550,600, 600,600, 550,1750, 600,1750, 550,1750, 550,650, 550,1750, 550,1750, 600,600, 550,1750, 600,600, 550,1750, 600,600, 550,650, 550,600, 600,600, 550,650, 550,600, 600,1750, 550,600, 600,1750, 550,1700, 600,1750, 600,1750, 550,1750, 550,1750, 600};  // UNKNOWN 53927E85

unsigned int irCommandToSend = 0;

// Cấu hình cho giao thức MQTT
const char* clientId = "KodiRemote1";
const char* mqttServer = "192.168.1.110";
const int mqttPort = 1883;
// Username và password để kết nối đến MQTT server nếu có 
// bật chế độ xác thực trên MQTT server
// Nếu không dùng thì cứ để vậy
const char* mqttUsername = "<MQTT_BROKER_USERNAME>";
const char* mqttPassword = "<MQTT_BROKER_PASSWORD>";

// Tên MQTT topic để gửi lệnh bật/tắt đèn
const char* room1LightTopic = "/D1/CMD/2"; //"/eHome/Bedroom1/Light";
const char* room2LightTopic = "/D1/CMD/3"; //"/eHome/Livingroom/Light";

// Tên MQTT topic để lắng nghe lệnh bật/tắt máy lạnh từ bộ điều khiển trung tâm
const char* acCommandTopic = "/eHome/Livingroom/AC";
// Tên MQTT topic để lắng nghe lệnh bật/tắt TV từ bộ điều khiển trung tâm
const char* tvCommandTopic = "/eHome/Livingroom/TV";

// Khởi tạo thư viện để kết nối wifi và MQTT server
WiFiClient wclient;
PubSubClient client(wclient);

void setup() {
  // setup for RF315/433 Mhz
  Serial.begin(115200);
  pinMode(RF_D0, INPUT);
  pinMode(RF_D1, INPUT);
  pinMode(RF_D2, INPUT);
  pinMode(RF_D3, INPUT);
  pinMode(RF_VT, INPUT);

  // bật thư viện thu tín hiệu hồng ngoại
  irrecv.enableIRIn();  // Start the receiver

  // và thư viện phát hồng ngoại
  irsend.begin();

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  connectWifiMQTTServer();
}


void connectWifiMQTTServer() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Kết nối tới MQTT server
  client.setServer(mqttServer, mqttPort);
  // Đăng ký hàm sẽ xử lý khi có dữ liệu từ MQTT server gửi về
  client.setCallback(onMQTTMessageReceived);

  Serial.println("");
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Kết nối tới MQTT server cho đến khi thành công
  while (!client.connected()) {
    Serial.println("Dang ket noi voi MQTT server...");
    if (client.connect(clientId, mqttUsername, mqttPassword)) {
      Serial.println("Da ket noi thanh cong toi MQTT server");
      client.subscribe(acCommandTopic);
      delay(100);
      client.subscribe(tvCommandTopic);
      delay(100);
    } else {
      Serial.print("Ket noi that bai, error code =");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

void loop() {
  // Gửi lệnh khi có yêu cầu từ bộ điều khiển trung tâm
  if (irCommandToSend) {
    if (irCommandToSend == 1) {
      irsend.sendRaw(acOnCommand, sizeof(acOnCommand) / sizeof(acOnCommand[0]), 38);      
    } else if (irCommandToSend == 2) {
      irsend.sendRaw(acOffCommand, sizeof(acOffCommand) / sizeof(acOffCommand[0]), 38);      
    } else if (irCommandToSend == 3) {
      irsend.sendRaw(tvOnCommand, sizeof(tvOnCommand) / sizeof(tvOnCommand[0]), 38);      
    } else if (irCommandToSend == 4) {
      irsend.sendRaw(tvOnCommand, sizeof(tvOnCommand) / sizeof(tvOnCommand[0]), 38);      
    }
    irCommandToSend = 0;
  }
  // Nếu phát hiện bị ngắt kết nối tới server thì tiến hành kết nối lại
  if (!client.connected()) {
    connectWifiMQTTServer();
  } else {
    // Nhận dữ liệu từ MQTT server
    client.loop();
  }

  // Xử lý tín hiệu nhận được từ remote RF
  if (digitalRead(RF_VT)) {
    Serial.print("Nhan duoc tin hieu tu remote RF: ");
    int d0 = digitalRead(RF_D0);
    int d1 = digitalRead(RF_D1);
    int d2 = digitalRead(RF_D2);
    int d3 = digitalRead(RF_D3);
    Serial.print(d0);
    Serial.print(d1);
    Serial.print(d2);
    Serial.print(d3);
    Serial.println("");

    if (!d0 && !d1 && d2 && !d3) { // Nút 1 trên remote được nhấn (0010)
      Serial.println("Nut 1 duoc nhan");
      Serial.println("Bat den phong ngu");
      client.publish(room1LightTopic, "1");
    } else if (d0 && !d1 && !d2 && !d3) { // Nút 2 trên remote được nhấn (1000)
      Serial.println("Nut 2 duoc nhan");
      Serial.println("Tat den phong ngu");
      client.publish(room1LightTopic, "0");      
      irsend.sendRaw(acOffCommand, sizeof(acOffCommand) / sizeof(acOffCommand[0]), 38);    
    } else if (!d0 && !d1 && !d2 && d3) { // Nút 3 trên remote được nhấn (0001)
      Serial.println("Nut 3 duoc nhan");
      Serial.println("Bat den phong khach");
      client.publish(room2LightTopic, "1");
    } else if (!d0 && d1 && !d2 && !d3) { // Nút 4 trên remote được nhấn (0100)      
      Serial.println("Nut 4 duoc nhan");
      Serial.println("Tat den phong khach");
      client.publish(room2LightTopic, "0");      
    } 
    // else
    // Nếu bạn dùng remote nhiều nút hơn thì có thể thêm vài cái if else ở đây
    delay(1000);
  }

  // Xử lý tín hiệu nhận được từ remote hồng ngoại
  decode_results  results;
  if (irrecv.decode(&results)) {  // Giải mã code hồng ngoại IR
    Serial.print(results.value, HEX);
    Serial.println("");
    irrecv.resume();
    delay(1000);
  }
}

void onMQTTMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nhan duoc du lieu tu MQTT server [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Đoạn code dưới đây kiểm tra xem lệnh nhận được là làm việc gì
  // và lưu lại vào biến irCommandToSend để thực hiện ở vòng loop tiếp theo
  // Ta không được phát lệnh hồng ngoại ngay trong hàm này
  // vì hàm này được gọi bằng chế độ interrupt khi có dữ liệu từ MQTT server gửi đến
  // do đó các tác vụ trong hàm này phải hoàn thành trong thời gian ngắn nhất có thể tránh gây ra lỗi
  if (strcmp(topic, acCommandTopic) == 0) {
    if ((char)payload[0] == '1') {
      Serial.println("Nhan duoc lenh bat may lanh");
      // Phát tín hiệu hồng ngoại để bật máy lạnh
      irCommandToSend = 1;
    } else if ((char)payload[0] == '0') {
      Serial.println("Nhan duoc lenh tat may lanh");     
      // Phát tín hiệu hồng ngoại để tắt máy lạnh
      irCommandToSend = 2;
    }
  } else if (strcmp(topic, tvCommandTopic) == 0) {
    if ((char)payload[0] == '1') {
      Serial.println("Nhan duoc lenh bat TV");
      // Phát tín hiệu hồng ngoại để bật TV
      irCommandToSend = 3;
    } else if ((char)payload[0] == '0') {
      Serial.println("Nhan duoc lenh tat TV");     
      // Phát tín hiệu hồng ngoại để tắt TV
      irCommandToSend = 4;
    }
  } else {
    Serial.println("Lenh nhan duoc khong hop le: "); Serial.println(payload[0]);
  }
}
