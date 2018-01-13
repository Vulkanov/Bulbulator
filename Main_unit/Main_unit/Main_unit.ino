#include <VirtualWire.h>
#include <EEPROM.h>
#include <TM1628ts.h>
#include <IRremote.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#define WATERSENSOR4 4
#define PowerPin5 5
#define IonPin6 6
#define HeaterPin0 A0
#define ModePin1 A1
#define ModePin2 A2
#define ModePin3 A3


TM1628ts disp(9, 8, 7); // define - data pin 8, clock pin 9 and strobe pin 7

// Главные переменные


//const byte OFF = 0;
//const byte ON = 1;
//const byte NO_WATER = 3; // Питание 
bool PWR = LOW;
byte PWRSTATE = 0; 
bool WATERSENSOR = LOW;               // Датчик наличия бака и воды LOW = пустой или нет бака
byte IONSTATE_adress = 40;  
bool IONSTATE = EEPROM.read(IONSTATE_adress);                         // Ионизатор вкл/выкл
byte HEATSTATE_adress =45; //Адрес в памяти HEATING
bool HEATSTATE = EEPROM.read(HEATSTATE_adress);                    // Подогрев вкл/выкл
byte HUMPOW_adress = 50;              // Адрес в памяти HUM_MODE
byte HUMPOW = EEPROM.read(HUMPOW_adress); // Режим работы увлажн. 1-2-3 
byte HUMVAL_adress = 55;
byte HUMVAL = EEPROM.read(HUMVAL_adress); //Значение поддержания влажности 40 - 90
int button; // кнопки управления
long previousMillis = 0; //счетчик прошедшего времени для AutoMainScreen
long interval = 30000; //задержка автовозврата к MainScreen 3сек
unsigned long currentMillis;

byte msg[VW_MAX_MESSAGE_LEN]; // Буфер для хранения принимаемых данных 
byte msgLen = VW_MAX_MESSAGE_LEN; // Размер сообщения
int T = 10;
int H = 10;
int P = 740;
int display_intensity = 7;

const int led_pin = 13; // Пин светодиода 
const int receiver_pin = 12; // Пин подключения приемника
const int reset_pin = 10; // при высоком состоянии wi-fi модуль работает, при низком - reset. 3 раза reset - вход в режим настройки.
IRrecv irrecv(11); // Указываем пин, к которому подключен приемник
decode_results results;

void setup() {
  Serial.begin(9600); // Скорость передачиданных 
  mySerial.begin(9600);
  vw_set_rx_pin(receiver_pin); // Пин подключения приемника
  vw_setup(2000); // Скорость передачи данных (бит в секунду) 
  vw_rx_start(); // Активация применика 
  disp.init(7); // Запуск дисплея с максимальной яркостью
  irrecv.enableIRIn(); 
  pinMode(WATERSENSOR4, INPUT); // Датчик воды
  pinMode(PowerPin5, OUTPUT); // Включение 
  pinMode(IonPin6, OUTPUT); // Включение ИОН+
  pinMode(HeaterPin0, OUTPUT); // Подогрев
  pinMode(ModePin1, OUTPUT); // Режим 1
  pinMode(ModePin2, OUTPUT); // Режим 2
  pinMode(ModePin3, OUTPUT); //
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);
// Приветствие :)
 for (int i = 14; i > 0; i--) {
    
    disp.putDigitAt(i, 0);
    disp.putDigitAt(i, 2);
    disp.putDigitAt(i, 4);
    disp.putDigitAt(i, 6);
    disp.putDigitAt(i, 8);
    disp.putDigitAt(i, 10);
    disp.putDigitAt(i, 12);
    disp.writeBuffer();
    disp.setStatus(LEVEL_LED1, 1);
    disp.setStatus(LEVEL_LED2, 1);
    disp.setStatus(LEVEL_LED3, 1);
    disp.setStatus(LEVEL_LED4, 1);
    disp.setStatus(LEVEL_LED5, 1);
    disp.setStatus(LEVEL_LED6, 1);
    disp.setStatus(LEVEL_LED7, 1);
    disp.setStatus(LEVEL_LED8, 1);
    disp.setStatus(LEVEL_LED9, 1);
    delay(150);
  }

  delay(300);
  
}

// ========== ФУНКЦИИ ==========



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
  msgLen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(msg, &msgLen)) { // Если есть данные.. 
    digitalWrite(led_pin, HIGH); // Зажигаем светодиод в начале приема пакета 
    if (msg[0] != 'v' && msg[1] != 'u' && msg[2] != 'l' && msg[3] != 'v')
      {
        return; 
      } 
     delay(1); // для отладки, чтобы видеть, когда мигает светодиод
     digitalWrite(led_pin, LOW); // Гасим светодиод в конце 
     T = processing_data(4, 5);
     H = processing_data(6, 7);
     P = processing_data(8, 10);
      
     // Serial.print("T ");
     // Serial.println(T);
     // Serial.print("B ");
     // Serial.println(H);
     // Serial.print("D ");
     // Serial.println(P);
      }  
}


 

void power_control() {
  WATERSENSOR = digitalRead(WATERSENSOR4);
  if (PWR == LOW) {
    PWRSTATE = 0;
  }
  else if (PWR == HIGH && PWRSTATE == 0) {
    PWRSTATE = 1;
  }
  if (WATERSENSOR == LOW && PWR == HIGH) {
    PWRSTATE = 2;
    digitalWrite(PowerPin5, LOW);
    digitalWrite(ModePin3, HIGH);
    return;
  }
  else if (PWRSTATE == 2 && WATERSENSOR == HIGH && PWR == HIGH){
    PWRSTATE = 1;
  }
  if (PWRSTATE == 0 || PWRSTATE == 2 || PWRSTATE == 3){
    digitalWrite(PowerPin5, LOW);
    digitalWrite(ModePin3, HIGH);
    EEPROM.update(IONSTATE_adress, IONSTATE);
    EEPROM.update(HEATSTATE_adress, HEATSTATE);
    EEPROM.update(HUMPOW_adress, HUMPOW);
    EEPROM.update(HUMVAL_adress, HUMVAL);
  }
  else if (PWRSTATE == 1){
    digitalWrite(PowerPin5, HIGH);
    
  }
}
void mode_control(){
  if (PWRSTATE == 0 || PWRSTATE == 2 || PWRSTATE == 3){
    digitalWrite(ModePin1, LOW);
    digitalWrite(ModePin2, LOW);
    digitalWrite(ModePin3, HIGH);
  }
  else if (HUMPOW == 1 && PWRSTATE == 1){
    digitalWrite(ModePin1, HIGH);
    digitalWrite(ModePin2, LOW);
    digitalWrite(ModePin3, LOW);
  }
  else if (HUMPOW == 2 && PWRSTATE == 1){
    digitalWrite(ModePin1, LOW);
    digitalWrite(ModePin2, HIGH);
    digitalWrite(ModePin3, LOW);
  }
  else if (HUMPOW == 3 && PWRSTATE == 1){
    digitalWrite(ModePin1, LOW);
    digitalWrite(ModePin2, LOW);
    digitalWrite(ModePin3, LOW);
  }
}

void ion_control(){
  if (PWRSTATE == 0 || PWRSTATE == 2 || PWRSTATE == 3 || IONSTATE == 0){
    digitalWrite(IonPin6, LOW);
  }
  else if (IONSTATE == 1){
    digitalWrite(IonPin6, HIGH);
  }
}

void heat_control(){
  if (PWRSTATE == 0 || PWRSTATE == 2 || PWRSTATE == 3 || HEATSTATE == 0){
    digitalWrite(HeaterPin0, LOW);
  }
  else if (HEATSTATE == 1){
    digitalWrite(HeaterPin0, HIGH);
  }
}

void hum_control(){
  if (PWRSTATE == 1 && HUMVAL < (H-1)){
      PWRSTATE = 3;
    }
  else if (PWRSTATE == 3 && HUMVAL > (H+1)){
      PWRSTATE = 1;
  }
}

void keyboard() { 
  char inString;    
  button = disp.getKeyboard();
  if (button != 0){
        delay(150);
        if (button == 4) inString = '1';
        if (button == 16) inString = '2';
        if (button == 2) inString = '3';
        if (button == 8) inString = '4';
        if (button == 10) inString = '5';
        if (button == 1) inString = '6';
        control(inString);
  } 
  delay(100);  
}

void irr() {
  if (irrecv.decode(&results)) // Если данные пришли 
  {
    //Serial.println(results.value); // Отправляем полученную данную в консоль
    delay(500);
    if (results.value == 970202566) control('1');
    if (results.value == 3768077238) control('2');
    if (results.value == 1319256238) control('3');
    if (results.value == 924466310) control('4');
    if (results.value == 338831067) control('5');
    if (results.value == 2737486129) control('6');
    irrecv.resume(); // Принимаем следующую команду
  }
}

void web() {
  char inString;
  if (mySerial.available() > 0) {
    inString = mySerial.read();
    Serial.println(inString);
      
    control(inString);
    delay(5);
  }
}
  
void control(int val){
  switch(val) {
    
    case '1':
      if (PWR < 1) PWR++;
      else PWR = 0;
      Serial.println("CASE1");
      uartsend();
      break;
    case '2': // IONSTATE
      if (IONSTATE < 1) IONSTATE++;
      else IONSTATE = 0;
      uartsend();
      Serial.println("CASE2");
      break;
    case '3': // HEATSTATE
      if (HEATSTATE < 1) HEATSTATE++;
      else HEATSTATE = 0;
      uartsend();
      Serial.println("CASE3");
      break;
    case '4': // HUMPOW
      if (HUMPOW < 3) HUMPOW++;
      else HUMPOW = 1;
      uartsend();
      Serial.println("CASE4");
      break;
    case '5': // HUMVAL
      if (HUMVAL < 81) HUMVAL += 5;
      else HUMVAL = 45;
      uartsend();
      Serial.println("CASE5");
      break;
    case '6':
      uartsend();
      Serial.println("CASE6");
      break;
    default: 
      //delay(10);
      break;
  }
}

void led_display() {
  disp.setIntensity(display_intensity);
  disp.clearBuffer();
  disp.putNumberAt(P, 4, false, 10);
  disp.putNumberAt(H, 8, false, 10);
  disp.setStatus(RH_LED, 1);
  disp.putNumberAt(T, 12, false, 10);
  disp.setStatus(GRADUS_LED, 1);
  disp.setStatus(POWER_LED, (PWR > 0 ? 0 : 1));
  disp.setStatus(HEATER_LED, (HEATSTATE > 0 ? 1 : 0));
  disp.setStatus(NO_WATER_LED, (WATERSENSOR == LOW ? 1 : 0));
  disp.setStatus(ION_LED, (IONSTATE > 0 ? 1 : 0));
  disp.setStatus(LEVEL_LED1, (HUMPOW == 1 || HUMPOW == 2 || HUMPOW == 3 ? 1 : 0));
  disp.setStatus(LEVEL_LED2, (HUMPOW == 1 || HUMPOW == 2 || HUMPOW == 3 ? 1 : 0));
  disp.setStatus(LEVEL_LED3, (HUMPOW == 1 || HUMPOW == 2 || HUMPOW == 3 ? 1 : 0));
  disp.setStatus(LEVEL_LED4, (HUMPOW == 3 || HUMPOW == 2 ? 1 : 0));
  disp.setStatus(LEVEL_LED5, (HUMPOW == 3 || HUMPOW == 2 ? 1 : 0));
  disp.setStatus(LEVEL_LED6, (HUMPOW == 3 || HUMPOW == 2 ? 1 : 0));
  disp.setStatus(LEVEL_LED7, (HUMPOW == 3 ? 1 : 0));
  disp.setStatus(LEVEL_LED8, (HUMPOW == 3 ? 1 : 0));
  disp.setStatus(LEVEL_LED9, (HUMPOW == 3 ? 1 : 0));
  
  // disp.setStatus(ONE_DIGIT_LED, 1); // Цифра "1" слева от давления, не нужна
  // disp.setStatus(DVOETOCHIE_LED, 1); // Двоеточие между цифрами давления (для часов)
  //disp.setStatus(SETUP_LED, 1);
  disp.writeBuffer(); 
}

void loop() {
keyboard();
web();
irr();  
power_control();
mode_control();
ion_control();
heat_control();
hum_control();
recieve();  
led_display();

  

}
void uartsend(){
 String htmlsend; //= {T, "|", H, "|", "P", "|", PWRSTATE, "|", IONSTATE, "|", HEATSTATE, "|", HUMPOW, "|", HUMVAL};
    htmlsend += T;
    htmlsend += "|";
    htmlsend += H;
    htmlsend += "|";
    htmlsend += P;
    htmlsend += "|";
    htmlsend += PWRSTATE;
    htmlsend += "|";
    htmlsend += IONSTATE;
    htmlsend += "|";
    htmlsend += HEATSTATE;
    htmlsend += "|";
    htmlsend += HUMPOW;
    htmlsend += "|";
    htmlsend += HUMVAL;
    htmlsend += "|";
    
    mySerial.print(htmlsend);
   // Serial.println(htmlsend);
  }
