#ifndef GG_SHEETS_H
#define GG_SHEETS_H
#include <Arduino.h>
#define MAX_STUDENTS 10 // Maximum number of students supported
// Student struct: holds id, code, and name for each student
struct Student {
    String id;      // Student ID (as string)
    char code[10];  // Student code (RFID or similar)
    char name[30];  // Student name
};

// Reads student list from Google Sheet
bool gg_read_students(Student* students, int& studentCount, const String& webAppUrl);
// Sends UID to Google Sheet
void gg_send_uid(const String& uid, const String& webAppUrl);
// Sends weight log to Google Sheet
void gg_send_weight_result(const String& uid, const String& name, const String& weight, const String& webAppUrl);
// Helper: URL-encode a string
String gg_urlencode(String str);
// Helper: Find student name by UID
char* gg_getStudentNameById(const char* uid, Student* students, int studentCount);
// Helper: Count elements in a delimited string
int gg_countElements(const char* data, char delimiter);

#endif
