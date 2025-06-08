#ifndef GG_SHEETS_H
#define GG_SHEETS_H
#include <Arduino.h>
#define MAX_STUDENTS 10
struct Student {
    String id;
    char code[10];
    char name[30];
};

// Đọc danh sách học sinh từ Google Sheet
bool gg_read_students(Student* students, int& studentCount, const String& webAppUrl);
// Gửi UID lên Google Sheet
void gg_send_uid(const String& uid, const String& webAppUrl);
// Gửi log kết quả cân lên Google Sheet
void gg_send_weight_result(const String& uid, const String& name, const String& weight, const String& webAppUrl);
// Hàm phụ trợ
String gg_urlencode(String str);
char* gg_getStudentNameById(const char* uid, Student* students, int studentCount);
int gg_countElements(const char* data, char delimiter);

#endif
