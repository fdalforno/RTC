#include <Wire.h>
#include <DS1307.h>
const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/**
 * Conversione della data in unix time
 */
time_t time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * HOUR_PER_DAY + h) * SECS_PER_MIN + m) * SECS_PER_MIN + s;
}

/**
 * Numero di giorni dal 01/01/2000 valido fino al 2099
 */
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}
//Decimale to binary coded decimal
static uint8_t decToBcd(uint8_t val){
	return ( (val/10*16) + (val%10) );
}
//binary coded decimal to decimal
static uint8_t bcdToDec(uint8_t val){
	return ( (val/16*10) + (val%16) );
}

//converto il carattere ascii in decimale
static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

/**
 * Costruttore
 */
DS1307::DS1307(){
	this->count = 0;
	this->mallocSize = 0;
	this->nextMillis = -1;
	this->lastMillis = 0;
	this->initialCapacity = sizeof(RTCTimerInformation);
	
	Wire.begin();
}
/**
 * Controllo se l'orologio in tempo reale sta funzionando
 */
boolean DS1307::isrunning(void) {
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  uint8_t ss = Wire.read();
  return !(ss>>7);
}

/**
 * accendo l'orologio
 */
void DS1307::startClock(void){
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);                      
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  //Leggo il numereo di secondi e imposto a 0 il bit 8 (0 acceso 1 spento) 01111111
  uint8_t second = Wire.read() & 0x7f;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0); //Scrivo il registro 0
  Wire.write(second);
  Wire.endTransmission();
}

/**
 * spegno l'orologio
 */
void DS1307::stopClock(void){
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  uint8_t second = Wire.read() | 0x80;       //Imposto a 1 il bit 8 spengo l'orologio
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(second);                    //Scrivo nel registro 0 il bit 8 a 1 
  Wire.endTransmission();
}

/**
 * leggo la data dai registri e ritorno la data in formato POSIX secondi dal 01/01/1970
 */
time_t DS1307::time(void){
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(0);
	Wire.endTransmission();  
	Wire.requestFrom(DS1307_I2C_ADDRESS, 7); // leggo i 7 registri dell'orologio
	
	second = bcdToDec(Wire.read() & 0x7F);
	minute = bcdToDec(Wire.read());
	hour = bcdToDec(Wire.read() & 0x3f);
	
	dayOfWeek = bcdToDec(Wire.read());
	dayOfMonth = bcdToDec(Wire.read());
	month = bcdToDec(Wire.read());
	year = bcdToDec(Wire.read());
	
	uint16_t days = date2days(year,month,dayOfMonth);
	time_t value = time2long(days,hour,minute,second);
	value += SECS_YR_2000;
	
	return value;
}

/**
 * imposto i dati nei registri interni dell'orologio
 */
void DS1307::setTime(){
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(0);
	Wire.write(decToBcd(second)); // 0 al bit 7 se si vuole che il sistema funzioni
	Wire.write(decToBcd(minute));
	Wire.write(decToBcd(hour));  // Se si vuole un formato orario in 12 ore impostare il bit 6 a 1
	Wire.write(decToBcd(dayOfWeek));
	Wire.write(decToBcd(dayOfMonth));
	Wire.write(decToBcd(month));
	Wire.write(decToBcd(year));
	Wire.endTransmission();
}
/**
 * imposto la data nei registri dell'orologio partendo dalle date in formato stringa
 * esempio : date = "Dec 26 2009", time = "12:34:56"
 */
void DS1307::setTime(const char* date, const char* time){
	year = conv2d(date + 9);

	//Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (date[0]) {
		case 'J': month = date[1] == 'a' ? 1 : month = date[2] == 'n' ? 6 : 7; break;
		case 'F': month = 2; break;
		case 'A': month = date[2] == 'r' ? 4 : 8; break;
		case 'M': month = date[2] == 'r' ? 3 : 5; break;
		case 'S': month = 9; break;
		case 'O': month = 10; break;
		case 'N': month = 11; break;
		case 'D': month = 12; break;
	}
	
	dayOfMonth = conv2d(date + 4);
    hour = conv2d(time);
    minute = conv2d(time + 3);
    second = conv2d(time + 6);
	
	uint16_t day = date2days(year, month, dayOfMonth);
	dayOfWeek = (day + 6) % 7; //il 1 gennaio 2000 è sabato e ritorno 6
	
	this->setTime();
}
/**
 * Aggiungo un timer alla schedulazione
 */
void DS1307::addTimer(byte minute, byte hour, byte day, byte month, byte dayOfWeek, void (*onEvent)(TimeInformation* Sender)) {
	if (this->count > 0) {
		if (this->mallocSize < (sizeof(RTCTimerInformation)*(this->count+1))) {
			this->mallocSize = sizeof(RTCTimerInformation)*(this->count+1);
            this->timers = (RTCTimerInformation*) realloc(this->timers, this->mallocSize);
		}
	}else{
		if (this->initialCapacity >= sizeof(RTCTimerInformation)) {
			this->mallocSize = this->initialCapacity;
        } else {
            this->mallocSize = sizeof(RTCTimerInformation);
        }
	}
	
	this->currentTimer = this->timers+this->count; //array index
	this->currentTimer->minute = minute;
	this->currentTimer->hour = hour;
	this->currentTimer->dayOfWeek = dayOfWeek;
	this->currentTimer->day = day;
	this->currentTimer->month = month;
	this->currentTimer->onEvent = onEvent;

	this->count++;
}

/**
 * Libero la memoria dei timers
 */
void DS1307::clear() {
	if (this->mallocSize > 0)
		free(this->timers);

	this->count = 0;
	this->mallocSize = 0;
}

void DS1307::loop() {
	this->currMillis = millis();
    
	//Controllo di prima esecuzione ed overflow
	if (this->nextMillis == -1 || this->currMillis < this->lastMillis) {
		//prepare next step
		this->time();                         //aggiorno orologio
		this->nextMillis = (60 - this->second) * 1000; //millisecondi al prossimo minuto
		this->nextMillis += this->currMillis;    //sommo al tempo attuale per la temporizzazione
		this->lastMinute = this->minute;    //aggiorno ultimo minuto
    }else if (this->currMillis >= this->nextMillis) {
		//Colpito il trigger calcolo le funzioni da lanciare
		this->time(); // aggiorno orologio
		this->currMinute = this->minute;
		
		//Evito di chiamare la funzione più volte nello stesso minuto
		if ( this->lastMinute != this->currMinute ) {
			//Leggo la lista dei trigger
			for (this->index = 0; this->index < this->count; this->index++) {
				this->currentTimer = this->timers+this->index;
				if (this->currentTimer->minute == this->currMinute || this->currentTimer->minute == TIMER_ANY) {
					//Sono nel minuto corretto del trigger
					if (this->currentTimer->hour == this->hour || this->currentTimer->hour == TIMER_ANY) {
						//Sono nell'ora corretta del trigger
						if (this->currentTimer->dayOfWeek == this->dayOfWeek || this->currentTimer->dayOfWeek == TIMER_ANY) {
							//Sono nel giorno corretto della settimana del trigger
							if (this->currentTimer->day == this->dayOfMonth || this->currentTimer->day == TIMER_ANY) {
								//Sono nel giorno del mese corretto
								if (this->currentTimer->month == this->month || this->currentTimer->month == TIMER_ANY){
									//Sono nel mese corretto lancio la funzione
									
									//Imposto il timer dell'evento
									TimeInformation time = { this->second , this->minute , this->hour, this->dayOfWeek , this->dayOfMonth , this->month , this->year};
									this->currentTimer->onEvent(&time);
								}
							}
						}
					}
				}
			}
			
			this->time();                         //aggiorno ora
            this->nextMillis = (60 - this->second) * 1000; //millisecondi al prossimo minuto
            this->nextMillis += this->currMillis;
            this->lastMinute = this->currMinute;     //imposto il minuto per evitare ripetizioni
		}
		//aggiorno il last mills per avvicinarmi al prossimo trigger
		this->lastMillis = this->currMillis;
	}

}





