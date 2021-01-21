#include  <Wire.h>
#define   DS3231_ADDRESS    0x68  // Dirección del modulo DS3231
#define   seconds_per_day   86400 // Cantidad de segundos en un día
byte      second,
          minute,
          hour,
          dayOfWeek,
          dayOfMonth,
          month,
          year;

char      daysOfTheWeek[8][12] = {"NaN",
                                  "Domingo",
                                  "Lunes",
                                  "Martes",
                                  "Miercoles",
                                  "Jueves",
                                  "Viernes",
                                  "Sabado"
                                 };

bool      rtc_error = false;      // Bandera error comunicación rtc
int       time_since;             // 
int       time_aux;               // 
int       random_delay;           // 
unsigned long last_time;          // La ultima vez que se leyó la fecha/hora del RTC en segundos.
unsigned long count_aux;          // 
unsigned long count_time_since;   // Cantidad de segundos desde que se perdió la comunicación

void setup() {
  Wire.begin();                   // Comienzo de la comunicación I2C con el modulo
  Serial.begin(19200);
  // Seconds, Minutes, Hour, Day Of Week, Day Of Month, Month, Year) {
  
  //while (setRtc(00, 54, 22, 4, 20, 1, 21) == 0) {delay(500);} // Establecemos la hora al RTC
}

void loop() {
  random_delay = (random(1, 5) * 1000);
  if (getRtc() == 1) {
    print_date_time();
  } else {
    count_time_since = (millis() - time_since) / 1000;
    Serial.print("- RTC ERROR SINCE: ");
    Serial.print(count_time_since + time_aux);
    Serial.println(" sec");
    
    count_aux = last_time + count_time_since;               // Tiempo desde que se detuvo mas tiempo que se lleva contando en segundos.
    if (count_aux >= seconds_per_day) {
      Serial.println("---------------------");
      Serial.println("-    FATAL ERROR    -");
      Serial.println("- CHECK CONN TO RTC -");
      Serial.println("---------------------");
    } else {
      hour = (count_aux / 3600);                              // De segundos totales a hora.
      minute = ((count_aux - hour * 3600) / 60);              // De segundos totales a minuto.
      second = (count_aux - ((hour * 3600) + (minute * 60))); // De segundos totales a segundos(formato 60 seg)
      
      print_date_time();
    }
  }
  delay(random_delay);
}

byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

// conversion BCD to Dec
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}

bool setRtc(byte ss, byte mm, byte hh, byte DoW, byte DoM, byte M, byte y) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0);              // Set 0 to first register
  Wire.write(decToBcd(ss));   // Set second
  Wire.write(decToBcd(mm));   // Set minutes
  Wire.write(decToBcd(hh));   // Set hours
  Wire.write(decToBcd(DoW));  // Set day of week (1=su, 2=mo, 3=tu)
  Wire.write(decToBcd(DoM));  // Set day of month
  Wire.write(decToBcd(M));    // Set month
  Wire.write(decToBcd(y));    // Set year
  if (Wire.endTransmission() != 0) {
    Serial.println("---------------------");
    Serial.println("-    FATAL ERROR    -");
    Serial.println("- CHECK CONN TO RTC -");
    Serial.println("---------------------");
    return false;
  } else {
    return true;
  }
}

bool getRtc() {
  //Wire.begin();
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0);                            // Write "0"
  if (Wire.endTransmission() == 0) {        // Waits ACK from RTC
    Wire.requestFrom(DS3231_ADDRESS, 7);    // Request - 7 bytes from RTC
    second = bcdToDec(Wire.read() & 0x7f);
    minute = bcdToDec(Wire.read());
    hour = bcdToDec(Wire.read() & 0x3f);
    dayOfWeek = bcdToDec(Wire.read());
    dayOfMonth = bcdToDec(Wire.read());
    month = bcdToDec(Wire.read());
    year = bcdToDec(Wire.read());
    rtc_error = false;
    count_time_since = 0;
    time_aux = millis();
    return true;
  } else {
    if (rtc_error == false) {
      time_aux = (millis() - time_aux) / 1000;
      last_time = ((hour * 3600) + (minute * 60) + (second) + (time_aux)); // Convertimos la última hora guardada en segundos para llevar el tiempo de mejor manera
      time_since = millis();
      rtc_error = true;
    }
    return false;
  }
}

// Agregar un 0 adelante cuando el Nro sea menor a 10

void pad_zero(int num) {
  char buf[6];

  if (num < 10) {
    sprintf(buf, "%02d", num);
    Serial.print(buf);
  } else {
    sprintf(buf, "%01d", num);
    Serial.print(buf);
  }
}

void print_date_time() {
  Serial.print("Hora: ");
  /* ¿Hay que hacer formato 24/12 hrs? */
  pad_zero(hour);
  Serial.print(":");
  pad_zero(minute);
  Serial.print(":");
  pad_zero(second);
  Serial.print("\tFecha: ");              // "t" realiza un tab en el serial
  Serial.print(daysOfTheWeek[dayOfWeek]);
  Serial.print(", ");
  pad_zero(dayOfMonth);
  Serial.print("/");
  pad_zero(month);
  Serial.print("/");
  Serial.print(year);
  Serial.print("\tdelay: ");
  Serial.print(random_delay);
  Serial.println("ms");
}
