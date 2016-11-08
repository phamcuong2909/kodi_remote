#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>

// Khai báo các chân nối với các module

#define IR_DECODER_PIN  D4 // mắt nhận hồng ngoại
#define IR_ENCODER_PIN  D8 // mắt phát hồng ngoại

#define RF_D0 D2 // module thu sóng RF gồm 5 chân
#define RF_D1 D1
#define RF_D2 D7
#define RF_D3 D6
#define RF_VT D5 // chân này sẽ active khi có tín hiệu RF thu được

// Cấu hình Wifi, sửa lại theo đúng mạng Wifi của bạn
const char* WIFI_SSID = "your_wifi_network";
const char* WIFI_PWD = "your_wifi_pass";

// Cấu hình cho giao thức MQTT
const char* clientId = "KodiRemote1";
const char* mqttServer = "192.168.1.110";
const int mqttPort = 1883;
// Username và password để kết nối đến MQTT server nếu server có
// bật chế độ xác thực trên MQTT server
// Nếu không dùng thì cứ để vậy
const char* mqttUsername = "<MQTT_BROKER_USERNAME>";
const char* mqttPassword = "<MQTT_BROKER_PASSWORD>";

// Mapping key trên remote RF với MQTT topic tương ứng để gửi đi
String rfKeys [][3] = { 
  {"0001", "/easytech.vn/LivingRoom/Light1/Command", "1"}, //key 1
  {"0010", "/easytech.vn/LivingRoom/Light1/Command", "0"}, //key 2
  {"0011", "/easytech.vn/LivingRoom/Light2/Command", "1"}, //key 3
  {"0100", "/easytech.vn/LivingRoom/Light2/Command", "0"}, //key 4
  
  {"0101", "/easytech.vn/LivingRoom/Light3/Command", "1"}, //key 5
  {"0110", "/easytech.vn/LivingRoom/Light3/Command", "0"}, //key 6
  {"0111", "/easytech.vn/LivingRoom/Light4/Command", "1"}, //key 7
  {"1000", "/easytech.vn/LivingRoom/Light4/Command", "0"}, //key 8

  {"1001", "/easytech.vn/BedRoom1/Light1/Command", "1"}, //key 9
  {"1010", "/easytech.vn/BedRoom1/Light1/Command", "0"}, //key 10
  {"1011", "/easytech.vn/BedRoom1/Light2/Command", "1"}, //key 11
  {"1100", "/easytech.vn/BedRoom1/Light2/Command", "0"} //key 12
};

// Mapping key trên remote hồng ngoại với MQTT topic tương ứng để gửi đi 
String irKeys [][3] = { 
  {"e0e020df", "/easytech.vn/LivingRoom/Light1/Command", "1"},
  {"e0e0a05f", "/easytech.vn/LivingRoom/Light1/Command", "0"},
  {"e0e0609f", "/easytech.vn/LivingRoom/Light2/Command", "1"},
  {"e0e010ef", "/easytech.vn/LivingRoom/Light2/Command", "0"},
  {"e0e0906f", "/easytech.vn/LivingRoom/Light3/Command", "1"},
  {"e0e050af", "/easytech.vn/LivingRoom/Light3/Command", "0"},
  {"e0e030cf", "/easytech.vn/LivingRoom/Light4/Command", "1"},
  {"e0e0b04f", "/easytech.vn/LivingRoom/Light4/Command", "0"},
};


// Tên MQTT topic để lắng nghe lệnh bật/tắt máy lạnh từ bộ điều khiển trung tâm
const char*  acCommandTopic = "/easytech.vn/Livingroom/AC";
// Tên MQTT topic để lắng nghe lệnh bật/tắt TV từ bộ điều khiển trung tâm
const char*  tvCommandTopic = "/easytech.vn/Livingroom/TV";

// Lệnh phát hồng ngoại để bật tắt đèn
// Dùng code mẫu trong File\Examples\IRremoteESP8266\IRrecvDumpV2 để học lệnh
unsigned int  acOnCommand[211] = {3850,1800, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,400, 500,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,400, 500,1350, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,1350, 550,350, 550,400, 550,400, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,1300, 550,400, 500,400, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,1350, 550,1300, 550,1300, 550,1350, 550,400, 550,350, 550,400, 550,1300, 550,400, 550,400, 500,400, 550};  // PANASONIC 555A:F3080088
unsigned int  acOffCommand[211] = {3850,1800, 550,350, 550,1350, 550,350, 550,1350, 500,400, 550,1300, 600,400, 550,1300, 550,400, 550,1300, 550,400, 550,1300, 550,1350, 550,350, 550,1350, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,400, 550,350, 550,1350, 550,1300, 550,400, 550,400, 500,400, 550,400, 550,1300, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,400, 550,1350, 500,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 500,450, 550,350, 550,400, 550,1300, 550,400, 550,400, 550,350, 550,400, 550,400, 500,400, 550,400, 500,400, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,400, 550,350, 550,400, 550,350, 550,1350, 550,1300, 550,1350, 550,1300, 550,1350, 550,350, 550,400, 550,400, 550,350, 550,1350, 550,350, 550,400, 550};
unsigned int tvOnOffCommand[67] = {9250,4550, 550,650, 550,600, 600,600, 550,1750, 600,600, 600,650, 550,600, 600,600, 550,1750, 600,1750, 550,1750, 550,650, 550,1750, 550,1750, 600,600, 550,1750, 600,600, 550,1750, 600,600, 550,650, 550,600, 600,600, 550,650, 550,600, 600,1750, 550,600, 600,1750, 550,1700, 600,1750, 600,1750, 550,1750, 550,1750, 600};  // UNKNOWN 53927E85

// Biến tạm lưu lại tín hiệu hồng ngoại cần phát
// 1 là bật máy lạnh
// 2 là tắt máy lạnh
// 3 là bật/tắt TV (lệnh bật tắt TV từ remote là giống nhau)
volatile unsigned int irCommandToSend = 0;

// Khởi tạo thư viện phát và nhận hồng ngoại
IRrecv irrecv(IR_DECODER_PIN);
IRsend irsend(IR_ENCODER_PIN);

// Khởi tạo thư viện để kết nối wifi và MQTT server
WiFiClient wclient;
PubSubClient client(wclient);

void setup() {
  Serial.begin(115200);

  // Thiết lập chế độ hoạt động của các chân nối module RF
  pinMode(RF_D0, INPUT);
  pinMode(RF_D1, INPUT);
  pinMode(RF_D2, INPUT);
  pinMode(RF_D3, INPUT);
  pinMode(RF_VT, INPUT);

  // Bật chức năng thu tín hiệu hồng ngoại và phát hồng ngoại
  irrecv.enableIRIn(); 
  irsend.begin();

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  int count = 0;
  Serial.print("Ket noi mang wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    count++;

    if (count > 20) {
      ESP.restart();
    }
  }

  Serial.println("thanh cong");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Kết nối tới MQTT server
  client.setServer(mqttServer, mqttPort);
  // Đăng ký hàm xử lý khi có dữ liệu từ MQTT server gửi về
  client.setCallback(onMQTTMessageReceived);  

  // Kết nối tới MQTT server cho đến khi thành công
  Serial.print("Dang ket noi voi MQTT server...");
  while (!client.connected()) {    
    Serial.print(".");
    if (client.connect(clientId, mqttUsername, mqttPassword)) {
      Serial.println("thanh cong");
      client.subscribe(acCommandTopic);
      client.subscribe(tvCommandTopic);
      delay(100);
    } else {
      Serial.print("fail, error code =");
      Serial.println(client.state());
      ESP.restart();
    }
  }
}

void loop() {
  // Nhận dữ liệu từ MQTT server nếu có
  client.loop();

  // Gửi lệnh khi có yêu cầu từ bộ điều khiển trung tâm
  if (irCommandToSend) {
    if (irCommandToSend == 1) {
      Serial.println("Nhan duoc lenh bat may lanh");
      // Phát tín hiệu hồng ngoại để bật máy lạnh
      irsend.sendRaw(acOnCommand, sizeof(acOnCommand) / sizeof(acOnCommand[0]), 38);      
    } else if (irCommandToSend == 2) {
      Serial.println("Nhan duoc lenh tat may lanh");     
      // Phát tín hiệu hồng ngoại để tắt máy lạnh
      irsend.sendRaw(acOffCommand, sizeof(acOffCommand) / sizeof(acOffCommand[0]), 38);      
    } else if (irCommandToSend == 3) {
      Serial.println("Nhan duoc lenh tat TV");     
      // Phát tín hiệu hồng ngoại để tắt TV
      irsend.sendRaw(tvOnOffCommand, sizeof(tvOnOffCommand) / sizeof(tvOnOffCommand[0]), 38);      
    }
    irCommandToSend = 0;
  }

  // Xử lý tín hiệu nhận được từ remote RF
  if (digitalRead(RF_VT)) {
    int d0 = digitalRead(RF_D0);
    int d1 = digitalRead(RF_D1);
    int d2 = digitalRead(RF_D2);
    int d3 = digitalRead(RF_D3);   

    char rfSignal[4];
    sprintf(rfSignal, "%d%d%d%d", d0, d1, d2, d3);
    Serial.println(rfSignal);
    handleRFSignal(String(rfSignal));    
    delay(1000);
  }

  // Xử lý tín hiệu nhận được từ remote hồng ngoại
  decode_results  results;
  if (irrecv.decode(&results)) {  // Giải mã code hồng ngoại IR
    String irSignal = String(results.value, HEX);
    handleIRSignal(irSignal);
    irrecv.resume();
    delay(1000);
  }
}

void handleRFSignal(String rfSignal) {
  Serial.print("Nhan duoc tin hieu tu remote RF: "); Serial.println(rfSignal);
  for(byte i=0; i<sizeof(rfKeys)/sizeof(rfKeys[0]); i++) {
    if (rfKeys[i][0] == rfSignal) {
      Serial.print("Gui lenh tuong ung ve server thong qua MQTT: "); Serial.println(rfKeys[i][1]);
      char topic[100];
      char message[5];
      rfKeys[i][1].toCharArray(topic, sizeof(topic));
      rfKeys[i][2].toCharArray(message, sizeof(message));
      client.publish(topic, message);
      return;
    }
  }
}

void handleIRSignal(String irSignal) {
  Serial.print("Nhan duoc tin hieu tu remote hong ngoai: "); Serial.println(irSignal);
  for(byte i=0; i<sizeof(irKeys)/sizeof(irKeys[0]); i++) {
    if (irKeys[i][0] == irSignal) {
      Serial.print("Gui lenh tuong ung ve server thong qua MQTT: "); Serial.println(irKeys[i][1]);
      char topic[100];
      char message[5];
      irKeys[i][1].toCharArray(topic, sizeof(topic));
      irKeys[i][2].toCharArray(message, sizeof(message));
      client.publish(topic, message);
      return;
    }
  }
}

void onMQTTMessageReceived(char* topic, byte* payload, unsigned int length) {
  // Đoạn code dưới đây kiểm tra xem lệnh nhận được là làm việc gì
  // và lưu lại vào biến irCommandToSend để thực hiện ở vòng loop tiếp theo
  // Ta không được phát lệnh hồng ngoại ngay trong hàm này
  // vì hàm này được gọi bằng chế độ interrupt khi có dữ liệu từ MQTT server gửi đến
  // do đó các tác vụ trong hàm này phải hoàn thành trong thời gian ngắn nhất có thể tránh gây ra lỗi
  if (strcmp(topic, acCommandTopic) == 0) {
    if ((char)payload[0] == '1') {
      // Phát tín hiệu hồng ngoại để bật máy lạnh
      irCommandToSend = 1;
    } else if ((char)payload[0] == '0') {
      // Phát tín hiệu hồng ngoại để tắt máy lạnh
      irCommandToSend = 2;
    }
  } else if (strcmp(topic, tvCommandTopic) == 0) {
    // Phát tín hiệu hồng ngoại để bật/tắt TV
    irCommandToSend = 3;
  }
  return;
}
