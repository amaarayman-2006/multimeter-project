#include <LiquidCrystal_I2C.h>

#define VOLT_IN       A0
#define OHM           A2
#define VOLT_DETECT   2
#define CURRENT_DETECT 4
#define OHM_DETECT    7
#define CURRENT_IN    A3
#define POWER_DETECT  5

// --- Calibration / scaling constants ---
const float VOLT_SCALE  = 20.0;   // 95K + 5K divider
const float VOLT_OFFSET = 0.2;    // Zero offset (calibrate with input shorted)
const float SHUNT_OHM   = 22.0;   // Current shunt
const float OHM_REF     = 10.0;   // 10K reference resistor (Kohms)

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(VOLT_DETECT,   INPUT);
  pinMode(CURRENT_DETECT, INPUT);
  pinMode(OHM_DETECT,    INPUT);
  pinMode(POWER_DETECT,  INPUT);
}

/**
 * Read an analog pin and return the averaged ADC value (0-1023).
 * 'samples' should be a power of 2 if you later want to use bit-shifts.
 */
float readAverageADC(uint8_t pin, uint16_t samples) {
  // Dummy read: let the ADC S/H capacitor settle after a channel switch
  analogRead(pin);
  delayMicroseconds(150);

  uint32_t sum = 0;
  for (uint16_t i = 0; i < samples; i++) {
    sum += analogRead(pin);
  }
  return sum / (float)samples;
}

void loop() {
  int p_on = digitalRead(POWER_DETECT);
  int v_on = digitalRead(VOLT_DETECT);
  int c_on = digitalRead(CURRENT_DETECT);
  int o_on = digitalRead(OHM_DETECT);

  if (v_on == HIGH && c_on == LOW && o_on == LOW && p_on == LOW) {
    runVoltmeter();
  }
  else if (v_on == LOW && c_on == HIGH && o_on == LOW && p_on == LOW) {
    runAmmeter();
  }
  else if (v_on == LOW && c_on == LOW && o_on == HIGH && p_on == LOW) {
    runOhmmeter();
  }
  else if (v_on == LOW && c_on == LOW && o_on == LOW && p_on == HIGH) {
    runWattmeter();
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Choose mode     ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(500);
    lcd.clear();
  }
}

/* ------------------------------------------------------------------ */
/*  VOLTMETER                                                          */
/* ------------------------------------------------------------------ */
void runVoltmeter() {
  // 500 samples ~ 50 ms. That spans ~2.5 cycles of 50 Hz mains hum,
  // which naturally averages out power-supply ripple.
  float avgADC = readAverageADC(VOLT_IN, 500);
  float pinV   = avgADC * (5.0 / 1023.0);
  float volt   = pinV * VOLT_SCALE - VOLT_OFFSET;
  if (volt < 0.0) volt = 0.0;

  lcd.setCursor(0, 0);
  lcd.print("Voltmeter       ");
  lcd.setCursor(0, 1);

  if (volt >= 1.0) {
    lcd.print("V = ");
    lcd.print(volt, 2);
    lcd.print(" V   ");
  } else {
    lcd.print("V = ");
    lcd.print(volt * 1000.0, 1);
    lcd.print(" mV  ");
  }

  delay(400);
  lcd.clear();
}

/* ------------------------------------------------------------------ */
/*  AMMETER                                                            */
/* ------------------------------------------------------------------ */
void runAmmeter() {
  float avgADC = readAverageADC(CURRENT_IN, 500);
  float pinV   = avgADC * (5.0 / 1023.0);
  float Iamp   = pinV / SHUNT_OHM;
  if (Iamp < 0.0) Iamp = 0.0;

  lcd.setCursor(0, 0);
  lcd.print("Ammeter         ");
  lcd.setCursor(0, 1);

  if (Iamp >= 1.0) {
    lcd.print("I = ");
    lcd.print(Iamp, 3);
    lcd.print(" A   ");
  } else {
    lcd.print("I = ");
    lcd.print(Iamp * 1000.0, 1);
    lcd.print(" mA  ");
  }

  delay(400);
  lcd.clear();
}

/* ------------------------------------------------------------------ */
/*  OHMMETER                                                           */
/* ------------------------------------------------------------------ */
void runOhmmeter() {
  float avgADC = readAverageADC(OHM, 500);
  float Vr     = avgADC * (5.0 / 1023.0);

  // Avoid divide-by-zero if leads are open (Vr ~ 5 V)
  float r = 0.0;
  if (Vr < 4.98) {
    r = (OHM_REF * Vr) / (5.0 - Vr);
  } else {
    r = 999.99; // Over-range
  }
  if (r < 0.0) r = 0.0;

  lcd.setCursor(0, 0);
  lcd.print("Ohmmeter        ");
  lcd.setCursor(0, 1);

  if (r >= 1.0) {
    lcd.print("R = ");
    lcd.print(r, 2);
    lcd.print(" Kohm ");
  } else {
    lcd.print("R = ");
    lcd.print(r * 1000.0, 1);
    lcd.print(" ohm  ");
  }

  delay(400);
  lcd.clear();
}

/* ------------------------------------------------------------------ */
/*  WATTMETER                                                          */
/* ------------------------------------------------------------------ */
void runWattmeter() {
  // We need both voltage and current.
  // Re-sample each rather than re-using stale globals.
  float vADC = readAverageADC(VOLT_IN, 300);
  float volt = vADC * (5.0 / 1023.0) * VOLT_SCALE - VOLT_OFFSET;
  if (volt < 0.0) volt = 0.0;

  float cADC = readAverageADC(CURRENT_IN, 300);
  float Iamp = (cADC * (5.0 / 1023.0)) / SHUNT_OHM;
  if (Iamp < 0.0) Iamp = 0.0;

  float power = volt * Iamp;

  lcd.setCursor(0, 0);
  lcd.print("Wattmeter       ");
  lcd.setCursor(0, 1);

  if (power >= 1.0) {
    lcd.print("P = ");
    lcd.print(power, 3);
    lcd.print(" W   ");
  } else {
    lcd.print("P = ");
    lcd.print(power * 1000.0, 1);
    lcd.print(" mW  ");
  }

  delay(400);
  lcd.clear();
}
