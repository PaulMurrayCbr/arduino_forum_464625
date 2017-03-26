#include <Wire.h>
#include <LiquidCrystal.h>
#include <Stepper.h>


//////////////// PINOUT ///////////////////

const byte buttonPin = 2;
const byte potPin = A0;

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
Stepper stepper(48, 10, 11, 12, 13);

//////////////// STATE ///////////////////

enum State {
  SET, RUN, DONE
} state = SET;

const int MAX_MIN = 60 * 3; // three hours
const int MIN_MIN = 5; // five minutes
const int TARGET_STEPS = 183;
int target_min = 1;
uint32_t start_ms;

int currentSteps;
int currentTimeRemaining_sec;
int currentProgress_thousandths;


void setup()
{
  lcd.begin(16, 2);

  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);

  setup_set_lcd();
}

void loop()
{
  switch (state) {
    case SET:
      if (digitalRead(buttonPin) == LOW) {
        start_ms = millis();
        currentSteps = 0;
        currentTimeRemaining_sec = target_min * 60;
        setup_run_lcd();
        state = RUN;
      }
      else {
        int v = map(analogRead(potPin), 0, 1023, MIN_MIN, MAX_MIN);
        if (v != target_min) {
          target_min = v;
          draw_set_lcd();
        }
      }
      break;

    case RUN:
      {
        float pct = (float)( millis() - start_ms) / (float)(target_min * 60L * 1000L);
        if (pct >= 1.0) {
          while (currentSteps < TARGET_STEPS) {
            stepper.step(1);
            currentSteps++;
          }
          state = DONE;
          setup_done_lcd();
        }
        else {
          int targetSteps = TARGET_STEPS * pct;
          int targetProgress_thousanths = pct * 1000;
          int targetTimeRemaining_sec = ((float)target_min - (target_min * pct)) * 60;

          while (currentSteps < targetSteps) {
            stepper.step(1);
            currentSteps++;
          }

          if (targetProgress_thousanths != currentProgress_thousandths ||
              targetTimeRemaining_sec != currentTimeRemaining_sec) {
            currentProgress_thousandths = targetProgress_thousanths;
            currentTimeRemaining_sec = targetTimeRemaining_sec;
            draw_run_lcd();
          }
        }
      }
      break;

    case DONE:
      break;
  }
}

void setup_set_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TARGET");
  draw_set_lcd();
}

void draw_set_lcd() {
  lcd.setCursor(8, 0);
  writeTimeSec(target_min * 60);
}

void setup_run_lcd() {
  lcd.clear();
  draw_run_lcd();
}

void draw_run_lcd() {
  lcd.setCursor(0, 0);
  writeTimeSec(currentTimeRemaining_sec);
  lcd.setCursor(0, 1);
  writeThousandths(currentProgress_thousandths);
}

void setup_done_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DONE!");
  draw_done_lcd();
}

void draw_done_lcd() {
}

void writeTimeSec(int t) {
  lcd.write('0' + ((t / 3600) % 10));
  lcd.write(':');
  lcd.write('0' + ((t / 600) % 6));
  lcd.write('0' + ((t / 60) % 10));
  lcd.write(':');
  lcd.write('0' + ((t / 10) % 6));
  lcd.write('0' + ((t / 1) % 10));
}

void writeThousandths(int t) {
  lcd.write(t < 1000 ? ' ' : '1' );
  lcd.write(t < 100 ? ' ' : '0' + ((t / 100) % 10));
  lcd.write('0' + ((t / 10) % 10));
  lcd.write('.');
  lcd.write('0' + ((t / 10) % 10));
  lcd.write('%');

}

