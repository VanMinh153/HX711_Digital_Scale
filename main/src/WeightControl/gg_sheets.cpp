#include "gg_sheets.h"
#include <HTTPClient.h>
#include <WiFi.h>

// Đọc danh sách học sinh từ Google Sheets qua HTTP GET
// Trả về true nếu thành công, false nếu thất bại
// students: mảng lưu thông tin học sinh (id, code, name)
// studentCount: số lượng học sinh đọc được
// webAppUrl: URL của Google Apps Script Web App
bool gg_read_students(Student *students, int &studentCount,
                      const String &webAppUrl) {
  if (WiFi.status() == WL_CONNECTED) { // Kiểm tra đã kết nối WiFi chưa
    String Read_Data_URL = webAppUrl + "?sts=read"; // Tham số sts=read để yêu cầu đọc danh sách
    Serial.println("[gg_read_students] URL: " + Read_Data_URL);
    HTTPClient http;
    http.begin(Read_Data_URL.c_str()); // Bắt đầu kết nối HTTP
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Theo dõi chuyển hướng nếu có
    int httpCode = http.GET(); // Gửi request GET
    Serial.print("[gg_read_students] HTTP Status Code: ");
    Serial.println(httpCode);
    String payload;
    studentCount = 0; // Reset số lượng học sinh
    if (httpCode > 0) { // Nếu request thành công
      payload = http.getString(); // Lấy dữ liệu trả về dạng chuỗi CSV
      Serial.print("[gg_read_students] Payload length: ");
      Serial.println(payload.length());
      // Kiểm tra payload rỗng hoặc quá lớn (bảo vệ bộ nhớ)
      if (payload.length() == 0) {
        Serial.println("[gg_read_students][ERROR] Payload rỗng!");
        http.end();
        return false;
      }
      if (payload.length() > 2048) {
        Serial.println(
            "[gg_read_students][ERROR] Payload quá lớn, không xử lý để tránh "
            "tràn bộ nhớ!");
        http.end();
        return false;
      }
      // Phân tích chuỗi CSV: id,code,name,...
      char charArray[2048];
      payload.toCharArray(charArray, sizeof(charArray));
      char *token = strtok(charArray, ",");
      while (token != NULL && studentCount < MAX_STUDENTS) {
        // Đọc id (cột 1)
        students[studentCount].id = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL)
          break;
        // Đọc code (cột 2)
        strncpy(students[studentCount].code, token,
                sizeof(students[studentCount].code) - 1);
        students[studentCount].code[sizeof(students[studentCount].code) - 1] =
            '\0';
        token = strtok(NULL, ",");
        if (token == NULL)
          break;
        // Đọc name (cột 3)
        strncpy(students[studentCount].name, token,
                sizeof(students[studentCount].name) - 1);
        students[studentCount].name[sizeof(students[studentCount].name) - 1] =
            '\0';
        Serial.print("[gg_read_students] Student ");
        Serial.print(studentCount);
        Serial.print(": code=");
        Serial.print(students[studentCount].code);
        Serial.print(", name=");
        Serial.println(students[studentCount].name);
        studentCount++;
        token = strtok(NULL, ","); // Chuyển sang học sinh tiếp theo
      }
      Serial.print("[gg_read_students] Tổng số học sinh đọc được: ");
      Serial.println(studentCount);
    } else {
      Serial.println("[gg_read_students] HTTP request failed!"); // Lỗi HTTP
    }
    http.end(); // Đóng kết nối HTTP
    return studentCount > 0; // Trả về true nếu có học sinh
  }
  Serial.println("[gg_read_students] WiFi not connected!"); // Lỗi chưa kết nối WiFi
  return false;
}

// Gửi UID lên Google Sheets (ghi nhanh UID mới nhất)
// uid: mã thẻ RFID, webAppUrl: URL Apps Script
void gg_send_uid(const String &uid, const String &webAppUrl) {
  String Send_Data_URL = webAppUrl + "?sts=writeuid";
  Send_Data_URL += "&uid=" + uid; // Tham số uid
  Serial.println("[gg_send_uid] URL: " + Send_Data_URL);
  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); // Gửi request GET
  Serial.print("[gg_send_uid] HTTP Status Code: ");
  Serial.println(httpCode);
  http.end();
}

// Gửi kết quả cân (uid, name, weight) lên Google Sheets
// Dùng khi đã xác định được tên học sinh và cân nặng
void gg_send_weight_result(const String &uid, const String &name,
                           const String &weight, const String &webAppUrl) {
  String Send_Data_URL = webAppUrl + "?sts=writelog";
  Send_Data_URL += "&uid=" + uid;
  Send_Data_URL += "&name=" + gg_urlencode(name); // Mã hóa tên cho an toàn URL
  Send_Data_URL += "&weight=" + gg_urlencode(weight); // Mã hóa cân nặng
  Serial.println("[gg_send_weight_result] URL: " + Send_Data_URL);
  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); // Gửi request GET
  Serial.print("[gg_send_weight_result] HTTP Status Code: ");
  Serial.println(httpCode);
  http.end();
}

// Hàm mã hóa URL (chuyển ký tự đặc biệt thành %XX, thay space bằng +)
// Đảm bảo dữ liệu gửi qua HTTP GET không bị lỗi do ký tự lạ
String gg_urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      // Encode non-alphanumeric as %XX
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
        code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
        code0 = c - 10 + 'A';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield(); // Nhường CPU cho các task khác (tránh treo)
  }
  return encodedString;
}

// Tìm tên học sinh theo UID trong mảng students
// Trả về con trỏ tới tên nếu tìm thấy, nullptr nếu không
char *gg_getStudentNameById(const char *uid, Student *students,
                            int studentCount) {
  for (int i = 0; i < studentCount; i++) {
    if (strcmp(students[i].code, uid) == 0) {
      return students[i].name;
    }
  }
  return nullptr;
}

// Đếm số phần tử trong chuỗi phân tách bởi delimiter (dùng cho debug, kiểm tra dữ liệu)
int gg_countElements(const char *data, char delimiter) {
  char dataCopy[strlen(data) + 1];
  strcpy(dataCopy, data);
  int count = 0;
  char delim[2] = {delimiter, 0};
  char *token = strtok(dataCopy, delim);
  while (token != NULL) {
    count++;
    token = strtok(NULL, delim);
  }
  return count;
}
