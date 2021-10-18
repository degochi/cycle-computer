// the speed meter(cycle computer) for m5stack
// auther : degochi 
// 
#include <M5Stack.h>
#include <Preferences.h>
#include <FS.h>
#include <SPIFFS.h>

#define BUTTONA 39
#define BUTTONB 38
#define BUTTONC 37
#define sencesw 5

#define BUTTON_OFF 1

//variables declaretion
int vcnt1, vcnt2, velocity; //to calculate timer counter, period, velocity
int tire;                   //diameter of the tire (mm)

//timer interrupt variable.
volatile unsigned long usecCount = 0;
hw_timer_t *interrupptTimer = NULL;
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

//non volatile memory setting
Preferences preferences;

//timer setting
void IRAM_ATTR usecTimer()
{
  portENTER_CRITICAL_ISR(&mutex);
  usecCount += 5;
  portEXIT_CRITICAL_ISR(&mutex);
}

void setup()
{
//initialize the M5Stack object
  M5.begin();

//initialize non-volatile memory setting
  preferences.begin("nv-mem",false);
  tire = preferences.getInt("value",254);
  preferences.end();

//GPIO setting
  pinMode(BUTTONA, INPUT);
  pinMode(BUTTONB, INPUT);
  pinMode(BUTTONC, INPUT);
  pinMode(sencesw, INPUT_PULLUP);

// Lcd display setup
  M5.Lcd.fillScreen(BLACK);

//  M5.Lcd.setTextFont(10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);

  //interrupt timer setting
  //timerBegin is count per 100 microsec.
  interrupptTimer = timerBegin(0, 80, true);
  //interrupt method setting
  timerAttachInterrupt(interrupptTimer, &usecTimer, true);
  //interrupt timing setting.
  timerAlarmWrite(interrupptTimer, 5, true);
  timerAlarmDisable(interrupptTimer);

//initialize variables for speed calculation
  vcnt1=0;
  vcnt2=0;
  velocity=0;

//Time Count  Start
  timerAlarmEnable(interrupptTimer);

}

void loop()
{
//draw the setting menu to display
  M5.Lcd.setCursor(0, 200);
  M5.Lcd.printf("Setting");

  if (digitalRead(BUTTONA) != BUTTON_OFF)
  {
    delay(1);
    if (digitalRead(BUTTONA) != BUTTON_OFF){
      M5.Lcd.fillRect(0, 200, 130, 40, BLACK);
      M5.Lcd.setCursor(0, 210);
      M5.Lcd.printf("    +    -   OK");
      delay(200);
      for(;;){
        M5.Lcd.fillRect(175, 180, 60, 25, BLACK);
        M5.Lcd.setCursor(0, 180);
        M5.Lcd.printf("tire size:%03d mm\n",tire);
        if (digitalRead(BUTTONA) != BUTTON_OFF){
           tire=tire+1;
           delay(200);
        }
        if (digitalRead(BUTTONB) != BUTTON_OFF){
           tire=tire-1;
           delay(200);
        }
        if (digitalRead(BUTTONC) != BUTTON_OFF){
           M5.Lcd.fillScreen(BLACK);
           M5.Lcd.setCursor(0, 200);
           M5.Lcd.printf("Setting");
        
        //non-voratile memory setting
           preferences.begin("nv-mem",false);
           preferences.putInt("value",tire);
           preferences.end();
           break;
        }
      }
    }
  }

//Speed sencing
  if (digitalRead(sencesw) != BUTTON_OFF)
  {
    delay(1);
    if (digitalRead(sencesw) != BUTTON_OFF){
      vcnt2 = vcnt1;
      vcnt1 = usecCount;
    }
    for (;;){
      if (digitalRead(sencesw) == BUTTON_OFF){
        break;
      }
    }
  }

//velocity calculation
  velocity = tire*314*36/(vcnt1-vcnt2+1);

//error , no sencer input 1sec
  if ((velocity >100)|((usecCount-vcnt1)>1000000)){
     velocity = 0;
  }

//refresh display every 50ms 
  if(((int)((usecCount % 1000000) / 1000))<=50){

//draw velocity value to display
     M5.Lcd.fillRect(105, 100, 55, 25, BLACK);
     M5.Lcd.setCursor(0, 100);
     M5.Lcd.printf("Speed: %02d km/h\n",velocity);
  }
}
