#include "gg_sheets.h"
#include <HTTPClient.h>
#include <WiFi.h>

bool gg_read_students(Student *students, int &studentCount,
                      const String &webAppUrl) {
  if (WiFi.status() == WL_CONNECTED) {
    String Read_Data_URL = webAppUrl + "?sts=read";
    Serial.println("[gg_read_students] URL: " + Read_Data_URL);
    HTTPClient http;
    http.begin(Read_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("[gg_read_students] HTTP Status Code: ");
    Serial.println(httpCode);
    String payload;
    studentCount = 0;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("[gg_read_students] Payload: " + payload);
      char charArray[payload.length() + 1];
      payload.toCharArray(charArray, payload.length() + 1);
      char *token = strtok(charArray, ",");
      while (token != NULL) {
        // Đọc id
        students[studentCount].id = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL)
          break;
        // Đọc code
        strncpy(students[studentCount].code, token,
                sizeof(students[studentCount].code) - 1);
        students[studentCount].code[sizeof(students[studentCount].code) - 1] =
            '\0';
        token = strtok(NULL, ",");
        if (token == NULL)
          break;
        // Đọc name
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
        token = strtok(NULL, ",");
      }
    } else {
      Serial.println("[gg_read_students] HTTP request failed!");
    }
    http.end();
    return studentCount > 0;
  }
  Serial.println("[gg_read_students] WiFi not connected!");
  return false;
}

void gg_send_uid(const String &uid, const String &webAppUrl) {
  String Send_Data_URL = webAppUrl + "?sts=writeuid";
  Send_Data_URL += "&uid=" + uid;
  Serial.println("[gg_send_uid] URL: " + Send_Data_URL);
  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("[gg_send_uid] HTTP Status Code: ");
  Serial.println(httpCode);
  http.end();
}

void gg_send_weight_result(const String &uid, const String &name,
                           const String &weight, const String &webAppUrl) {
  String Send_Data_URL = webAppUrl + "?sts=writelog";
  Send_Data_URL += "&uid=" + uid;
  Send_Data_URL += "&name=" + gg_urlencode(name);
  Send_Data_URL += "&weight=" + gg_urlencode(weight);
  Serial.println("[gg_send_weight_result] URL: " + Send_Data_URL);
  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("[gg_send_weight_result] HTTP Status Code: ");
  Serial.println(httpCode);
  http.end();
}

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
    yield();
  }
  return encodedString;
}

char *gg_getStudentNameById(const char *uid, Student *students,
                            int studentCount) {
  for (int i = 0; i < studentCount; i++) {
    if (strcmp(students[i].code, uid) == 0) {
      return students[i].name;
    }
  }
  return nullptr;
}

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
