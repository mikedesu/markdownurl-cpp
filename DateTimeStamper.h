#pragma once

#include <string>

using namespace std;

class DateTimeStamper {

    public:
        DateTimeStamper();

        time_t now();
        
        string getDateTimeString(time_t *t);
        string getNowString();
        string getInitializedAtString();
        string getMilitaryTimeString();

        string getDateString();

    private:
        time_t initialized_at;

};


