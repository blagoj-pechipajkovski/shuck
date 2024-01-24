#ifndef SHUCK_TIME_H
#define SHUCK_TIME_H

namespace ShuckOS {
    struct Time_t {
        int year   = -1; // 1970 - ?
        int month  = -1; // 1-12
        int day    = -1; // 1-31
        int wday   = -1; // 1- 7
        int hour   = -1; // 0-23
        int minute = -1; // 0-59
        int second = -1; // 0-59
    };

    // only works with UTC
    namespace Time {

        void get(Time_t& now) { // gets full
            struct tm tm_time;
            time_t current_t = time(NULL);
            localtime_r(&current_t, &tm_time);

            now.year   = tm_time.tm_year+1900; // The number of years since 1900 -> to normal year
            now.month  = tm_time.tm_mon+1    ; // 0-11 -> 1-12
            now.day    = tm_time.tm_mday     ; // 1-31
            now.wday   = tm_time.tm_wday+1   ; // 0- 6 -> 1-7
            now.hour   = tm_time.tm_hour     ; // 0-23
            now.minute = tm_time.tm_min      ; // 0-59
            now.second = tm_time.tm_sec      ; // 0-59
        }

        void set(Time_t& usr_new_time) { // sets non -1 // cannot set weekday
            // dont modify users given time
            Time_t new_time;
            memcpy(&new_time, &usr_new_time, sizeof(Time_t));

            // get current for those not provided by user
            Time_t now;
            get(now);

            if (new_time.year   == -1) new_time.year   = now.year;
            if (new_time.month  == -1) new_time.month  = now.month;
            if (new_time.day    == -1) new_time.day    = now.day;
            if (new_time.hour   == -1) new_time.hour   = now.hour;
            if (new_time.minute == -1) new_time.minute = now.minute;
            if (new_time.second == -1) new_time.second = now.second;
            // week day cannot be set
            
            struct tm tm;
            
            tm.tm_year = new_time.year - 1900; // years since 1900
            tm.tm_mon  = new_time.month - 1  ; // month                    [0 ... 11] 
            tm.tm_mday = new_time.day        ; // day of the month         [1 ... 31]
            tm.tm_hour = new_time.hour       ; // hours since midnight     [0 ... 23]
            tm.tm_min  = new_time.minute     ; // minutes after the hour   [0 ... 59]
            tm.tm_sec  = new_time.second     ; // seconds after the minute [0 ... 60]
            
            time_t t = mktime(&tm);
            struct timeval timeval_now = { .tv_sec = t };
            settimeofday(&timeval_now, NULL);
        }
        
        void set_year(int new_year) {
            Time_t now;
            now.year = new_year;
            set(now);
        }
        
        void set_month(int new_month) {
            Time_t now;
            now.month = new_month;
            set(now);
        }
        
        void set_day(int new_day) {
            Time_t now;
            now.day = new_day;
            set(now);
        }
        
        void set_hour(int new_hour) {
            Time_t now;
            now.hour = new_hour;
            set(now);
        }
        
        void set_minute(int new_minute) {
            Time_t now;
            now.minute = new_minute;
            set(now);
        }
        
        void set_second(int new_second) {
            Time_t now;
            now.second = new_second;
            set(now);
        }
        
        
        int get_year() {
            Time_t now;
            get(now);
            return now.year;
        }
        
        int get_month() {
            Time_t now;
            get(now);
            return now.month;
        }
        
        int get_day() {
            Time_t now;
            get(now);
            return now.day;
        }
        
        int get_wday() {
            Time_t now;
            get(now);
            return now.wday;
        }
        
        int get_hour() {
            Time_t now;
            get(now);
            return now.hour;
        }
        
        int get_minute() {
            Time_t now;
            get(now);
            return now.minute;
        }
        
        int get_second() {
            Time_t now;
            get(now);
            return now.second;
        }
        
        
        void date_str(char* out_str, Time_t& this_time) {
            sprintf(out_str, "%04d.%02d.%02d", this_time.year, this_time.month, this_time.day);
        }

        void clock_str(char* out_str, Time_t& this_time) {
            sprintf(out_str, "%02d:%02d:%02d", this_time.hour, this_time.minute, this_time.second);
        }
        
        void date_str(char* out_str) {
            Time_t now;
            get(now);
            date_str(out_str, now);
        }

        void clock_str(char* out_str) {
            Time_t now;
            get(now);
            clock_str(out_str, now);
        }
    };
}


ShuckCMD time_cmd("time", "Date and clock",[](ShuckArgs& args, ShuckConsole& console) -> int {
    
    const char* usage[5] = {
        "time",
        "time date",
        "time clock",
        "time set [second/minute/hour/day/month/year] [value]",
        "time set [year] [month] [day] [hour] [minute] [second]"
    };
    
    if (args.arg_is(1, "set")) {
        if (args.argc() == 8) {
            ShuckOS::Time::set_second(atoi(args.arg(7)));
            ShuckOS::Time::set_minute(atoi(args.arg(6)));
            ShuckOS::Time::set_hour(  atoi(args.arg(5)));
            ShuckOS::Time::set_day(   atoi(args.arg(4)));
            ShuckOS::Time::set_month( atoi(args.arg(3)));
            ShuckOS::Time::set_year(  atoi(args.arg(2)));
        }
        else {
            if (!args.has(2)) {
                console.print_event(args.arg(0), "Error", "Please specify unit");
                console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4]);
                return SHUCK_CMD_USAGE_ERR;
            }

            if (!args.has(3)) {
                console.print_event(args.arg(0), "Error", "Please specify value");
                console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4]);
                return SHUCK_CMD_USAGE_ERR;
            }
            
            if      (args.arg_is(2, "second"))
                ShuckOS::Time::set_second(atoi(args.arg(3)));
            else if (args.arg_is(2, "minute"))
                ShuckOS::Time::set_minute(atoi(args.arg(3)));
            else if (args.arg_is(2, "hour"))
                ShuckOS::Time::set_hour(atoi(args.arg(3)));
            else if (args.arg_is(2, "day"))
                ShuckOS::Time::set_day(atoi(args.arg(3)));
            else if (args.arg_is(2, "month"))
                ShuckOS::Time::set_month(atoi(args.arg(3)));
            else if (args.arg_is(2, "year"))
                ShuckOS::Time::set_year(atoi(args.arg(3)));
            else {
                console.print_event(args.arg(0), "Error", "Invalid unit");
                console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4]);
                return SHUCK_CMD_USAGE_ERR;
            }
        }
    }
    
    if (args.arg_is(1, "date")) {
        char date_str[12];
        ShuckOS::Time::date_str(date_str);
        console.out.print(date_str);
        console.out.print('\n');
        return SHUCK_CMD_SUCCESS;
    }
    
    if (args.arg_is(1, "clock")) {
        char clock_str[9];
        ShuckOS::Time::clock_str(clock_str);
        console.out.print(clock_str);
        console.out.print('\n');
        return SHUCK_CMD_SUCCESS;
    }
    
    char date_str[11];
    ShuckOS::Time::date_str(date_str);
        
    char clock_str[9];
    ShuckOS::Time::clock_str(clock_str);
    
    console.out.printf("%s %s\n", date_str, clock_str);
    
    return SHUCK_CMD_SUCCESS;
});

#endif