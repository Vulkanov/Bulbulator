#include <VirtualWire.h>
#include <EEPROM.h>
#include <TM1628.h>

TM1628 dvdLED(9, 8, 7); // define - data pin 8, clock pin 9 and strobe pin 7

// Главные переменные

bool POWER_STATE = LOW;                 // Питание 
bool WATWER_SENSOR = LOW;               // Датчик наличия бака и воды LOW = пустой или нет бака
byte ION_adress = 40;  
bool ION = EEPROM.read(ION_adress);                         // Ионизатор вкл/выкл
byte HEATING_adress =45; //Адрес в памяти HEATING
bool HEATING = EEPROM.read(HEATING_adress);                    // Подогрев вкл/выкл
byte HUM_MODE_adress = 50;              // Адрес в памяти HUM_MODE
byte HUM_MODE = EEPROM.read(HUM_MODE_adress); // Режим работы увлажн. 1-2-3 

int button; // кнопки управления

byte msg[VW_MAX_MESSAGE_LEN]; // Буфер для хранения принимаемых данных 
byte msgLen = VW_MAX_MESSAGE_LEN; // Размер сообщения
int T = 10;
int H = 10;
int P = 740;

const int led_pin = 13; // Пин светодиода 
const int receiver_pin = 12; // Пин подключения приемника


void setup() {
  Serial.begin(9600); // Скорость передачиданных 
  vw_set_rx_pin(receiver_pin); // Пин подключения приемника
  vw_setup(2000); // Скорость передачи данных (бит в секунду) 
  vw_rx_start(); // Активация применика 
  dvdLED.begin(ON, 2); // Запуск дисплея и клавиатуры

}

// ========== ФУНКЦИИ ==========

void keyboard_display() {     
  button = dvdLED.getButtons();
  if (button != 0){
    dvdLED.clear();
    for(int i=0;i<8;i++){
      if((button >> i) & 0x01) {
        dvdLED.print("No");
        dvdLED.print(i);
        dvdLED.print('-');
        dvdLED.print(button);
      }
    }
  } else {
    dvdLED.setCursor(0);
    dvdLED.print(" button");
  }
  delay(100);  
}

// Приемник MX-RM-5V
// Прием и обработка данных  
 
int processing_data(int arrStart, int arrEnd) { // параметр = T или H или P
  // Принимается массив из 11-ти символов
  // 0-3 = пароль для защиты от помех
  // 4-5 = температура
  // 6-7 = влажность
  // 8-10 = давление   
    int number = 0;
    for (int i = arrStart; i <= arrEnd ; i++) 
    {  
      number *= 10;
      number += msg[i] - '0';
     // Serial.write(msg[i]); // для отладки - пишем полученные данные в порт
     //number = int(number);
    } 
    return number;
  } 

void recieve() {
  if (vw_get_message(msg, &msgLen)) { // Если есть данные.. 
    digitalWrite(led_pin, HIGH); // Зажигаем светодиод в начале приема пакета 
    if (msg[0] != 'v' && msg[1] != 'u' && msg[2] != 'l' && msg[3] != 'v')
      {
        return; 
      } 
     delay(100); // для отладки, чтобы видеть, когда мигает светодиод
     digitalWrite(led_pin, LOW); // Гасим светодиод в конце 
     T = processing_data(4, 5);
     H = processing_data(6, 7);
     P = processing_data(8, 10);
      
      Serial.print("T ");
      Serial.println(T);
      Serial.print("B ");
      Serial.println(H);
      Serial.print("D ");
      Serial.println(P);
      }  
}

void loop() {


recieve();  



}
