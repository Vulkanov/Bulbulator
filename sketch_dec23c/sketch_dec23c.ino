//#include <Wire.h> 
#include <BMP085.h>
#include <DHT.h>

#include <VirtualWire.h>
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
BMP085 dps = BMP085();



float H; // значение влажности
float T; // значение температуры
long P; // давление
const int led_pin = 13; // Пин светодиода 
const int transmit_pin = 12; // Пин подключения передатчика
const String RADIO_PASSWORD = "vulk";



void setup()
{
  Serial.begin(9600); 

// Датчик DHT 
  dht.begin();
//  Wire.begin(); 
//  delay(1000);
  dps.init();  
// Передатчик 433МГц
  vw_set_tx_pin(transmit_pin); 
  vw_setup(2000);       // Скорость передачи (Бит в секунду) 
  pinMode(led_pin, OUTPUT); 
}


// =============== ФУНКЦИИ ==============

// Датчик температуры и влажности DHT 22
void tempsensor() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  H = dht.readHumidity();
  T = dht.readTemperature();
  if (isnan(T) || isnan(H)) {
    Serial.println("Failed to read from DHT");
delay(1000);
  }
}

void barsensor() { 
 long Press;
  dps.getPressure(&Press); 
  P = Press / 133.3;
//  long Temperature;
 //bmp.getTemperature(&Temperature); // Температура с датчика BMP180 не требуется
delay(1000);
}

 

// Передача данных по каналу 433МГц

void transmit() { 
  digitalWrite(led_pin, HIGH); // Зажигаем светодиод в начале передачи
  
int Ttransmit = round(T);
int Htransmit = round(H);
//int Ptransmit = round(P);

String strMsg = RADIO_PASSWORD;
  if (Ttransmit > -10 && Ttransmit < 10) strMsg += "0";
  strMsg += Ttransmit;
  if (Htransmit > -10 && Htransmit < 10) strMsg += "0";
  strMsg += Htransmit;
  strMsg += P;
  char msg[255];
  strMsg.toCharArray(msg, 255);
  
  vw_send((uint8_t *)msg, strlen(msg)); // Отправка сообщения 
  vw_wait_tx(); // Ожидаем окончания отправки сообщения 
  digitalWrite(led_pin, LOW); // Гасим светодиод в конце передачи 
   
}


void loop()
{
  tempsensor();
  barsensor();
//  transmit();
  delay(2000);
  
 // выводим для отладки:
    Serial.print("T: "); 
    Serial.print(T);
    Serial.print(" *C\t");
    Serial.print("H: "); 
    Serial.print(H);
    Serial.print(" %\t");
    Serial.print("P: ");
    Serial.print(P);
    Serial.println(" mm");  
   // delay(1000); 
    
     

}


