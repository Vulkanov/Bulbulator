#include <Wire.h> 
#include <BMP085.h>
#include <DHT.h>
#include <OzOLED.h>
#include <VirtualWire.h>
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PIN_PHOTO_SENSOR A2
DHT dht(DHTPIN, DHTTYPE);
BMP085 dps = BMP085();


float H; // значение влажности
float T; // значение температуры
int P; // давление
const int led_pin = 13; // Пин светодиода 
const int transmit_pin = 12; // Пин подключения передатчика
const String RADIO_PASSWORD = "vulk";
int Brightness;



void setup()
{
  Serial.begin(9600); 
// Датчик DHT 
  Wire.begin(); 
  dht.begin();
  dps.init();  
// Передатчик 433МГц
  vw_set_tx_pin(transmit_pin); 
  vw_setup(2000);       // Скорость передачи (Бит в секунду) 
  pinMode(led_pin, OUTPUT); 
  pinMode(PIN_PHOTO_SENSOR, INPUT);
  OzOled.init();
}


// =============== ФУНКЦИИ ==============

// Датчик температуры и влажности DHT 22
void tempsensor() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  H = dht.readHumidity() + 3;
  delay(3000);
  T = dht.readTemperature();
  if (isnan(T) || isnan(H)) {
   return; // Serial.println("Failed to read from DHT");
  }
}

void barsensor() { 
 long Press;
 dps.getPressure(&Press); 
 P = Press / 133.3;
 //long Temperature_BMP180; // Температура с датчика BMP180 не требуется
 //bmp.getTemperature(&Temperature_BMP180); // Температура с датчика BMP180 не требуется
}

void photosensor() {
  int val = analogRead(PIN_PHOTO_SENSOR);
  Brightness = map(val, 1023, 0, 0, 255); 
 // if (Brightness < 90) Brightness = 0;
 // else if (Brightness > 90) Brightness = 255;  
} 

// Вывод на диспоей
void display(float disp, byte x, byte y){
//OzOled.clearDisplay();           //clear the screen and set start position to top left corner
//OzOled.setNormalDisplay();       //Set display to Normal mode
//OzOled.setInverseDisplay(); 
  OzOled.setPageMode();            //Set addressing mode to Page Mode
  OzOled.setBrightness(Brightness);
  char tmp[10];
  dtostrf(disp, 0, 2, tmp);
  OzOled.printBigNumber(tmp, x, y, 4);
}

// Передача данных по каналу 433МГц
void transmit() { 
  byte msg[8];
  msg[0] = 186; // Первая цифра пароля
  msg[1] = 188; // Вторая цифра пароля
  msg[2] = 152; // Третья цифра пароля
  msg[3] = 154; // Четвертая цифра пароля
  msg[4] = round(T); // Температура округленная
  msg[5] = round(H); // Влажность округленная
  msg[6] = P % 100; // Давление (без цифры 7, ее прибавим на приемнике)
  msg[7] = Brightness;
  vw_send((uint8_t *)msg, strlen(msg)); // Отправка сообщения 
  vw_wait_tx(); // Ожидаем окончания отправки сообщения 
}

void loop()
{
  tempsensor();
  barsensor();
  photosensor();
  transmit();
  OzOled.clearDisplay();
  OzOled.printString("TEMP:");
  display(T, 2, 2);
  OzOled.printString("C",9,5);
  delay(3000);
  transmit();
  OzOled.clearDisplay();
  OzOled.printString("HUM: ");
  display(H, 2, 2);
  OzOled.printString("%",9,5);
  delay(3000);
  transmit();
  OzOled.clearDisplay();
  OzOled.printString("PRESS: ");
  display(P, 3, 2);
  OzOled.printString("MM",13,5);
  //delay(1000);
  
 // выводим для отладки:
 //   Serial.print("T: "); 
 //   Serial.print(T);
 //   Serial.print(" *C\t");
 //   Serial.print("H: "); 
 //   Serial.print(H);
 //   Serial.print(" %\t");
 //   Serial.print("P: ");
 //   Serial.print(P);
 //   Serial.println(" mm");  
 //   delay(1000); 
}


