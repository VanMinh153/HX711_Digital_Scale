#include <MFRC522.h>
#include <SPI.h>
#define SS_PIN 5
#define RST_PIN 17
MFRC522 rfid(SS_PIN, RST_PIN);
String uidString;

#include "WiFi.h"
#include <HTTPClient.h>
const char *ssid = "Trang"; //--> Your wifi name
const char *password = "20202020";
// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/"
                     "AKfycbxHOTl9vG3vmrx5FT06FN_Qs6GfW_"
                     "mpsD3oWhvT1iTLuAjt5LAncO1F9NSitsHdgI2f/exec";
#define On_Board_LED_PIN 2
#define MAX_STUDENTS 10 // Số lượng học sinh tối đa
struct Student {
  String id;
  char code[10];
  char name[30];
};
Student students[MAX_STUDENTS];
int studentCount = 0;
int runMode = 2;
const int btnIO = 15;
bool btnIOState = HIGH;
unsigned long timeDelay = millis();
unsigned long timeDelay2 = millis();
bool InOutState = 0; // 0: vào, 1:ra
const int ledIO = 4;
const int buzzer = 18; // Đổi sang chân 18 để tránh trùng SS_PIN

void setup() {
  Serial.begin(115200);
  Serial.println("[DEBUG] Setup bắt đầu");
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  pinMode(btnIO, INPUT_PULLUP);
  pinMode(ledIO, OUTPUT);
  pinMode(On_Board_LED_PIN, OUTPUT);
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  Serial.println("-------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int wifiRetry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    wifiRetry++;
    if (wifiRetry > 20) {
      Serial.println("\n[ERROR] Không kết nối được WiFi, reset ESP32");
      ESP.restart();
    }
  }
  Serial.println("\n[DEBUG] Đã kết nối WiFi");
  Serial.println("[DEBUG] Chuẩn bị đọc dữ liệu Google Sheet");
  bool sheetOK = readDataSheet();
  Serial.print("[DEBUG] Kết quả đọc Google Sheet: "); Serial.println(sheetOK);
  if (!sheetOK) {
    Serial.println("[ERROR] Can't read data from google sheet!");
    delay(2000);
  } else {
    Serial.println("[DEBUG] Đọc dữ liệu Google Sheet thành công");
  }
  Serial.println("[DEBUG] Khởi tạo SPI");
  SPI.begin();     // Init SPI bus
  Serial.println("[DEBUG] Khởi tạo MFRC522");
  rfid.PCD_Init(); // Init MFRC522
  Serial.println("[DEBUG] Đã khởi tạo SPI và MFRC522");
}

void loop() {
  if (millis() - timeDelay2 > 500) {
    Serial.println("[DEBUG] Đọc UID thẻ RFID");
    readUID();
    timeDelay2 = millis();
  }
  if (digitalRead(btnIO) == LOW) {
    if (btnIOState == HIGH) {
      if (millis() - timeDelay > 500) {
        // Thực hiện lệnh
        InOutState = !InOutState;
        digitalWrite(ledIO, InOutState);
        Serial.print("[DEBUG] Đổi trạng thái InOutState: ");
        Serial.println(InOutState);
        timeDelay = millis();
      }
      btnIOState = LOW;
    }
  } else {
    btnIOState = HIGH;
  }
}
void beep(int n, int d) {
  Serial.print("[DEBUG] Beep: "); Serial.print(n); Serial.print(" lần, "); Serial.print(d); Serial.println(" ms");
  for (int i = 0; i < n; i++) {
    digitalWrite(buzzer, HIGH);
    delay(d);
    digitalWrite(buzzer, LOW);
    delay(d);
  }
}
void readUID() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  if (!rfid.PICC_IsNewCardPresent()) {
    //Serial.println("[DEBUG] Không phát hiện thẻ mới");
    return;
  }
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("[DEBUG] Không đọc được serial thẻ");
    return;
  }
  uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
    uidString.concat(String(rfid.uid.uidByte[i], HEX));
  }
  uidString.toUpperCase();
  Serial.print("[DEBUG] Card UID: "); Serial.println(uidString);
  beep(1, 200);
  if (runMode == 1)
    writeUIDSheet();
  else if (runMode == 2)
    writeLogSheet();
  byte piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("[DEBUG] Không phải thẻ MIFARE Classic");
    return;
  }
}
bool readDataSheet() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(On_Board_LED_PIN, HIGH);
    String Read_Data_URL = Web_App_URL + "?sts=read";
    Serial.println();
    Serial.println("-------------");
    Serial.println("Read data from Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Read_Data_URL);
    HTTPClient http;
    http.begin(Read_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);
    String payload;
    studentCount = 0;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.print("[DEBUG] Payload length: "); Serial.println(payload.length());
      if (payload.length() == 0) {
        Serial.println("[ERROR] Payload rỗng!");
        http.end();
        digitalWrite(On_Board_LED_PIN, LOW);
        return false;
      }
      if (payload.length() > 2048) {
        Serial.println("[ERROR] Payload quá lớn, không xử lý để tránh tràn bộ nhớ!");
        http.end();
        digitalWrite(On_Board_LED_PIN, LOW);
        return false;
      }
      char charArray[2048];
      payload.toCharArray(charArray, sizeof(charArray));
      int numberOfElements = countElements(charArray, ',');
      Serial.println("Number of elements: " + String(numberOfElements));
      if (numberOfElements < 3) {
        Serial.println("[ERROR] Dữ liệu Google Sheet không hợp lệ!");
        http.end();
        digitalWrite(On_Board_LED_PIN, LOW);
        return false;
      }
      char *token = strtok(charArray, ",");
      while (token != NULL && studentCount < MAX_STUDENTS && studentCount < numberOfElements / 3) {
        students[studentCount].id = String(token);
        token = strtok(NULL, ",");
        if (token == NULL) break;
        strncpy(students[studentCount].code, token, sizeof(students[studentCount].code) - 1);
        students[studentCount].code[sizeof(students[studentCount].code) - 1] = '\0';
        token = strtok(NULL, ",");
        if (token == NULL) break;
        strncpy(students[studentCount].name, token, sizeof(students[studentCount].name) - 1);
        students[studentCount].name[sizeof(students[studentCount].name) - 1] = '\0';
        studentCount++;
        token = strtok(NULL, ",");
      }
      Serial.print("[DEBUG] studentCount: "); Serial.println(studentCount);
      for (int i = 0; i < studentCount; i++) {
        Serial.print("ID: ");
        Serial.println(students[i].id);
        Serial.print("Code: ");
        Serial.println(students[i].code);
        Serial.print("Name: ");
        Serial.println(students[i].name);
      }
    } else {
      Serial.println("[ERROR] Không lấy được dữ liệu từ Google Sheet");
    }
    http.end();
    digitalWrite(On_Board_LED_PIN, LOW);
    Serial.println("-------------");
    if (studentCount > 0)
      return true;
    else
      return false;
  } else {
    Serial.println("[ERROR] WiFi chưa kết nối khi đọc Google Sheet");
    return false;
  }
}
void writeUIDSheet() {
  String Send_Data_URL = Web_App_URL + "?sts=writeuid";
  Send_Data_URL += "&uid=" + uidString;
  Serial.println();
  Serial.println("-------------");
  Serial.println("Send data to Google Spreadsheet...");
  Serial.print("URL : ");
  Serial.println(Send_Data_URL);
  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code : ");
  Serial.println(httpCode);
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload : " + payload);
  } else {
    Serial.println("[ERROR] Không gửi được UID lên Google Sheet");
  }
  http.end();
}
void writeLogSheet() {
  char charArray[uidString.length() + 1];
  uidString.toCharArray(charArray, uidString.length() + 1);
  char *studentName = getStudentNameById(charArray);
  if (studentName != nullptr) {
    Serial.print("Tên học sinh với ID ");
    Serial.print(uidString);
    Serial.print(" là: ");
    Serial.println(studentName);
    String Send_Data_URL = Web_App_URL + "?sts=writelog";
    Send_Data_URL += "&uid=" + uidString;
    Send_Data_URL += "&name=" + urlencode(String(studentName));
    if (InOutState == 0) {
      Send_Data_URL += "&inout=" + urlencode("VÀO");
    } else {
      Send_Data_URL += "&inout=" + urlencode("RA");
    }
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);
    HTTPClient http;
    http.begin(Send_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload : " + payload);
    } else {
      Serial.println("[ERROR] Không gửi được log lên Google Sheet");
    }
    http.end();
  } else {
    Serial.print("[ERROR] Không tìm thấy học sinh với ID ");
    Serial.println(uidString);
    beep(3, 500);
  }
}
String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;
}
char *getStudentNameById(char *uid) {
  for (int i = 0; i < studentCount; i++) {
    if (strcmp(students[i].code, uid) == 0) {
      Serial.print("[DEBUG] Tìm thấy học sinh: ");
      Serial.println(students[i].name);
      return students[i].name;
    }
  }
  Serial.println("[DEBUG] Không tìm thấy học sinh theo UID");
  return nullptr; // Trả về nullptr nếu không tìm thấy
}
int countElements(const char *data, char delimiter) {
  char dataCopy[strlen(data) + 1];
  strcpy(dataCopy, data);
  int count = 0;
  char delim[2] = {delimiter, '\0'};
  char *token = strtok(dataCopy, delim);
  while (token != NULL) {
    count++;
    token = strtok(NULL, delim);
  }
  return count;
}
