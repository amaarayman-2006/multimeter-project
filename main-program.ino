#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define VOLT_IN A0
#define OHM A2
#define VOLT_DETECT 2
#define CURRENT_DETECT 4
#define OHM_DETECT 7
#define CURRENT_IN A1
#define POWER_DETECT 5
#define RX 9
#define TX 8

int v_on = 0, v_on1 = 0;
int c_on = 0, c_on1 = 0;
int o_on = 0, o_on1 = 0;
int p_on = 0, p_on1 = 0;
float volt = 0;
float Iamp = 0;
float r = 0;
float Vr = 0;
float avg = 0;
float power = 0;

String msgBuffer = "";
String masseg = "";
SoftwareSerial BTSerial(RX, TX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  BTSerial.begin(9600);

  pinMode(VOLT_IN, INPUT);
  pinMode(VOLT_DETECT, INPUT);
  pinMode(CURRENT_IN, INPUT);
  pinMode(CURRENT_DETECT, INPUT);
  pinMode(OHM, INPUT);
  pinMode(OHM_DETECT, INPUT);
  pinMode(POWER_DETECT, INPUT);
}

void usingApp() {
  while (BTSerial.available() > 0) {
    char data = (char) BTSerial.read();
    msgBuffer += data;
    if (data == ';') {
      masseg = msgBuffer;
      msgBuffer = "";
    }
    p_on1 = (masseg == "p;" || p_on == HIGH) && v_on != HIGH && c_on != HIGH && o_on != HIGH ? HIGH : LOW;
    v_on1 = (masseg == "v;" || v_on == HIGH) && p_on != HIGH && c_on != HIGH && o_on != HIGH ? HIGH : LOW;
    c_on1 = (masseg == "c;" || c_on == HIGH) && v_on != HIGH && p_on != HIGH && o_on != HIGH ? HIGH : LOW;
    o_on1 = (masseg == "o;" || o_on == HIGH) && v_on != HIGH && c_on != HIGH && p_on != HIGH ? HIGH : LOW;
  }
}

void loop() {
  p_on = digitalRead(POWER_DETECT);
  v_on = digitalRead(VOLT_DETECT);
  c_on = digitalRead(CURRENT_DETECT);
  o_on = digitalRead(OHM_DETECT);

  usingApp();

  for (int i = 0; i < 100; i++) {
    volt = (analogRead(VOLT_IN)) * (5.0 / 1023);
    volt = (volt * (100 / 5) - 0.2);
    if (i != 0) {
      volt = volt + avg;
    }
    avg = volt;
  }
  volt = volt / 100;
  avg = 0;

  float Vacross = (analogRead(CURRENT_IN)) * (5.0 / 1023);
  Iamp = ((Vacross) / 22);

  for (int i = 0; i < 300; i++) {
    Vr = (analogRead(OHM)) * (5.0 / 1023);
    r = ((2 * Vr) / (1 - (Vr / 5)));
    delay(3);
    if (i != 0) {
      r = r + avg;
    }
    avg = r;
  }
  r = r / 300;

  if ((v_on == HIGH || v_on1 == HIGH) && (c_on == LOW) && (o_on == LOW) && (p_on == LOW)) {
    lcd.print("Voltmeter");
    lcd.setCursor(0, 1);
    lcd.print("Volt =");
    lcd.setCursor(11, 1);
    lcd.print("V");
    lcd.setCursor(6, 1);
    lcd.print(volt);
    delay(1000);
    volt = 0;
    lcd.clear();
  }
  else if ((v_on == LOW) && (c_on == HIGH || c_on1 == HIGH) && (o_on == LOW) && (p_on == LOW)) {
    lcd.print("Ammeter");
    lcd.setCursor(0, 1);
    lcd.print("Current= ");
    if (floor(Iamp)) {
      lcd.setCursor(8, 1);
      lcd.print(Iamp - 0.44);
      lcd.setCursor(14, 1);
      lcd.print("A");
    }
    else {
      Iamp = Iamp * 1000;
      lcd.setCursor(8, 1);
      lcd.print(Iamp - 0.44);
      lcd.setCursor(14, 1);
      lcd.print("mA");
    }
    delay(1000);
    Iamp = 0;
    lcd.clear();
  }
  else if ((v_on == LOW) && (c_on == LOW) && (o_on == HIGH || o_on1 == HIGH) && (p_on == LOW)) {
    lcd.print("Ohmmeter");
    lcd.setCursor(0, 1);
    lcd.print("R =");
    if (floor(r)) {
      lcd.setCursor(3, 1);
      lcd.print(r);
      lcd.setCursor(12, 1);
      lcd.print("Kohm");
    }
    else {
      r = r * 1000;
      lcd.setCursor(3, 1);
      lcd.print(r);
      lcd.setCursor(12, 1);
      lcd.print("ohm");
    }
    delay(1000);
    r = 0;
    lcd.clear();
  }
  else if ((v_on == LOW) && (c_on == LOW) && (o_on == LOW) && (p_on == HIGH || p_on1 == HIGH)) {
    power = volt * Iamp;
    lcd.print("Wattmeter");
    lcd.setCursor(0, 1);
    lcd.print("Power= ");
    if (floor(power)) {
      lcd.setCursor(7, 1);
      lcd.print(power);
      lcd.setCursor(12, 1);
      lcd.print("Watt");
    }
    else {
      power = power * 100;
      lcd.setCursor(7, 1);
      lcd.print(power);
      lcd.setCursor(12, 1);
      lcd.print("mw");
    }
    delay(1000);
    lcd.clear();
    power = 0;
  }
  else {
    lcd.print("Choose mode");
    delay(2000);
    lcd.clear();
  }
}
