#include <Wire.h>
#include <DS1307.h>

DS1307 clock;

void setup(){
	Serial.begin(9600);
	clock = DS1307();
	clock.setTime(__DATE__,__TIME__);
	clock.addTimer(TIMER_ANY, TIMER_ANY, TIMER_ANY, TIMER_ANY, TIMER_ANY,minuteCall);
}

void loop(){
	clock.loop();
	printTime();
}
/*Function: Display time on the serial monitor*/
void printTime(){
	clock.time();
	Serial.print(clock.hour, DEC);
	Serial.print(":");
	Serial.print(clock.minute, DEC);
	Serial.print(":");
	Serial.print(clock.second, DEC);
	Serial.print("	");
	Serial.print(clock.month, DEC);
	Serial.print("/");
	Serial.print(clock.dayOfMonth, DEC);
	Serial.print("/");
	Serial.print(clock.year+2000, DEC);
	Serial.print(" ");
	Serial.print(clock.dayOfMonth);
	Serial.print("*");
	switch (clock.dayOfWeek)// Friendly printout the weekday
	{
		case MON:
		  Serial.print("MON");
		  break;
		case TUE:
		  Serial.print("TUE");
		  break;
		case WED:
		  Serial.print("WED");
		  break;
		case THU:
		  Serial.print("THU");
		  break;
		case FRI:
		  Serial.print("FRI");
		  break;
		case SAT:
		  Serial.print("SAT");
		  break;
		case SUN:
		  Serial.print("SUN");
		  break;
	}
	Serial.println(" ");
}

void minuteCall(TimeInformation* Sender) {
  Serial.println("Trigger launched! ");
}