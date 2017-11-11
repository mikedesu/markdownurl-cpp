#include "DateTimeStamper.h"

DateTimeStamper::DateTimeStamper() {
    initialized_at = now();
}

time_t DateTimeStamper::now() {
    return time(0);
}

std::string DateTimeStamper::getDateTimeString(time_t *t) {
    return std::string(ctime(t));
}

std::string DateTimeStamper::getNowString() {
    time_t t = now();
    return getDateTimeString(&t);
}

std::string DateTimeStamper::getInitializedAtString() {
    return getDateTimeString(&initialized_at);
}

std::string DateTimeStamper::getMilitaryTimeString() {
    time_t t = now();
    struct tm * a = localtime(&t);

    int hour = a->tm_hour;
    int min = a->tm_min;

    std::string hourStr;
    std::string minStr;

    if (hour < 10) {
        hourStr = "0" + std::to_string(a->tm_hour);
    }
    else {
        hourStr = std::to_string(hour);
    }

    if (min < 10) {
        minStr = "0" + std::to_string(min);
    }
    else {
        minStr = std::to_string(min);
    }

    return hourStr + ":" + minStr;
}

