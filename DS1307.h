#ifndef __DS1307_H__
#define __DS1307_H__

#include <Arduino.h>

#define DS1307_I2C_ADDRESS 0x68

#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6
#define SUN 7

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define HOUR_PER_DAY (24UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k

#define TIMER_ANY 0xFF

typedef unsigned long time_t;

struct TimeInformation {
	byte second;
	byte minute;
	byte hour;
	byte dayOfWeek;
	byte day;
	byte month;
	short year;
};

struct RTCTimerInformation {
    byte minute;
    byte hour;
    byte dayOfWeek;
    byte day;
    byte month;
    void (*onEvent)(TimeInformation* Sender);
};


class DS1307{
private:
	short count;
    short mallocSize;
	short index;
	short lastMinute;
	short currMinute;
	
	RTCTimerInformation* timers;
    RTCTimerInformation* currentTimer;

	unsigned long nextMillis;
    unsigned long lastMillis;
    unsigned long currMillis;

	
public:
	DS1307();
	boolean isrunning(void);
	void startClock(void);
	void stopClock(void);
	time_t time(void);
    void setTime();
	void setTime(const char* date, const char* time);
	void addTimer(byte minute, byte hour, byte day, byte month, byte dayOfWeek, void (*onEvent)(TimeInformation* data));
	void loop(void);
	void clear(void);
	short initialCapacity;

	uint8_t second;
	uint8_t minute;
	uint8_t hour; 
	uint8_t dayOfWeek;
	uint8_t dayOfMonth;
	uint8_t month;
	uint16_t year;
};
#endif
