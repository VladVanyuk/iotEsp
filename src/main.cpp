#include <Arduino.h>
#include "LittleFS.h"

#include "WiFiManager.h"
#include "webServer.h"
#include "updater.h"
#include "fetch.h"
#include "configManager.h"
#include "dashboard.h"
#include "timeSync.h"
#include <TZ.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <Fonts/FreeSerifBold9pt7b.h>


struct task
{
    unsigned long rate;
    unsigned long previous;
};

task taskA = {.rate = 60000, .previous = 0};
task taskO= {.rate = 600000, .previous = 0};

#define BTN_PIN D5
bool flag_press = false;

IRAM_ATTR void btn_isr()
{

  flag_press = true;
  //oled_active_flag = true;
}

void btn_setup()
{
  pinMode(BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), btn_isr, RISING);
}


#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C
#define OLED_RESET -1

bool oled_active_flag= false;

// uint32_t oled_timer;
const uint32_t off_seconds = 30000;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

void print_oled()
{
    //display.invertDisplay(true);
    Serial.println("OLED print started");

    display.setTextSize(1);
    display.setCursor(34, 0);
    display.println("IhorYarish");
    display.display();

    //display.setFont(&FreeSerifBold9pt7b);
    display.setCursor(0, 21);
    display.println(" Guru of coding");
    display.display();
}

void setup_OLED()
{
    Serial.println("OLED setup started");
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
    }
    
    display.clearDisplay();

    display.setTextColor(WHITE);
    //display.invertDisplay(true);

    print_oled();
    // DRAW_RECT_AND_UPDATE();
   // oled_timer = millis();

    oled_active_flag = true;
}

void loop_OLED()
{
    if (taskO.previous == 0 || (millis() - taskO.previous > taskO.rate))
    {
        Serial.println("OLED task");
        taskO.previous = millis();

        if (oled_active_flag == true && flag_press == false)
        {
            Serial.println("OLED off");
            //display.clearDisplay();
            oled_active_flag = false;
        }

        if (flag_press)
        {
            Serial.println("btn was pressed -> OLED on");
            flag_press = false;
            //print_oled();
            oled_active_flag = true;
        }
    }
}

#define DRAW_RECT_AND_UPDATE() \
  display.drawRect(1, 1, 126, 62, WHITE); \
  display.display(); \


void saveCallback() {
    Serial.println("EEPROM saved"); 
}

void setup()
{
    Serial.begin(115200);
    pinMode(D4, OUTPUT);
    digitalWrite(D4, LOW); //on

    btn_setup();
    setup_OLED();

    LittleFS.begin();
    GUI.begin();
    configManager.begin();
    configManager.setConfigSaveCallback(saveCallback);
    WiFiManager.begin(configManager.data.projectName);
    timeSync.begin(TZ_Europe_Kiev);

    timeSync.waitForSyncResult(10000);

    if (timeSync.isSynced())
    {
        time_t now = time(nullptr);
        Serial.print(PSTR("Current time in Amsterdam: "));
        Serial.print(asctime(localtime(&now)));
    }
    else 
    {
        Serial.println("Timeout while receiving the time");
    }

     digitalWrite(D4, HIGH); //off
   // dash.begin(500);
}

void loop()
{
    //software interrupts
    WiFiManager.loop();
    updater.loop();
    configManager.loop();
   // dash.loop();

    loop_OLED();
    //task A
    if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
    {
        taskA.previous = millis();

        // configManager.data.dummyInt++;
        //save the newest values in the EEPROM
        configManager.save();        
    }

    
}
