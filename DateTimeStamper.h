#pragma once

#include <string>

class DateTimeStamper {

    public:
        DateTimeStamper();

        time_t now();
        
        std::string getDateTimeString(time_t *t);
        std::string getNowString();
        std::string getInitializedAtString();
        std::string getMilitaryTimeString();

    private:
        time_t initialized_at;

};


