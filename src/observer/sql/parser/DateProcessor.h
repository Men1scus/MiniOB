// Reference: https://github.com/CentaureaHO/miniob/blob/63f38ae6f6630f78141844a9ea5104764661d0c5/src/observer/sql/parser/DateProcessor.h
#pragma once
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

extern bool IsLeapYear(int Year);
extern void StrDate2IntDate(const char *StrDate, int &IntDate);
extern std::string IntDate2StrDate(int IntDate);
extern bool CheckDate(int IntDate);