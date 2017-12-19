#include "DateTimeStamper.h"

DateTimeStamper::DateTimeStamper() {
    initialized_at = now();
}

time_t DateTimeStamper::now() {
    return time(0);
}

string DateTimeStamper::getDateTimeString(time_t *t) {
    return string(ctime(t));
}

string DateTimeStamper::getNowString() {
    time_t t = now();
    return getDateTimeString(&t);
}

string DateTimeStamper::getInitializedAtString() {
    return getDateTimeString(&initialized_at);
}

string DateTimeStamper::getMilitaryTimeString() {
    time_t t = now();
    struct tm * a = localtime(&t);

    int hour = a->tm_hour;
    int min = a->tm_min;

    string hourStr;
    string minStr;
    string retval;

    if (hour < 10) {
        hourStr = "0" + to_string(hour);
    }
    else {
        hourStr = to_string(hour);
    }

    if (min < 10) {
        minStr = "0" + to_string(min);
    }
    else {
        minStr = to_string(min);
    }

    retval = hourStr + ":" + minStr;

    return retval;
}



string DateTimeStamper::getDateString() {
    time_t t = now();
    struct tm * a = localtime(&t);

    int month = a->tm_mon + 1;
    int day = a->tm_mday;
    int year = a->tm_year + 1900;

    string monthStr;
    string dayStr;
    string yearStr;
    string dateStr;

    if (month < 10) {
        monthStr = "0" + to_string(month);
    }
    else {
        monthStr = to_string(month);
    }

    if (day < 10) {
        dayStr = "0" + to_string(day);
    }
    else {
        dayStr = to_string(day);
    }

    yearStr = to_string(year);
    dateStr = monthStr + "/" + dayStr + "/" + yearStr;

    return dateStr;
}

