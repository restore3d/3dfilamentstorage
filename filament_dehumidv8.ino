#include <Nextion.h>
#include <SoftwareSerial.h>
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

SoftwareSerial HMISerial(10,11);
Weather sensor;

uint32_t next = 0;

float humidity;
String tempC;
String tempF;
float desiredHumidity = 20;
bool trigger;

String color1;
String type1;
String month1;
String day1;

String color2;
String type2;
String month2;
String day2;

int changeSetting;
int colorCount1 = 0;
int typeCount1 = 0;
int monthCount1 = 0;
int dayCount1 = 0;

int colorCount2 = 0;
int typeCount2 = 0;
int monthCount2 = 0;
int dayCount2 = 0;

#define RELAYPIN 5
NexButton bOn = NexButton(0,4, "bOn");
NexButton bOff = NexButton(0,5, "bOff");

NexText tTempC = NexText(1, 7, "tTempC");
NexText tTempF = NexText(1, 8, "tTempF");
NexText tHum = NexText(1, 11, "tHum");
NexText tHumSlider = NexText(2, 0, "tHumSlider");
NexSlider h0 = NexSlider(2, 8, "h0");
NexButton bReset = NexButton(1, 13, "bReset");

NexButton bColor1 = NexButton(3, 10, "bColor1");
NexButton bType1 = NexButton(3, 11, "bType1");
NexButton bMonth1 = NexButton(3, 12, "bMonth1");
NexButton bDay1 = NexButton(3, 13, "bDay1");
NexButton bTop1 = NexButton(3, 8, "bTop1");
NexButton bDown1 = NexButton(3, 9, "bDown1");

NexButton bColor2 = NexButton(4, 9, "bColor2");
NexButton bType2 = NexButton(4, 10, "bType2");
NexButton bMonth2 = NexButton(4, 11, "bMonth2");
NexButton bDay2 = NexButton(4, 12, "bDay2");
NexButton bTop2 = NexButton(4, 7, "bTop2");
NexButton bDown2 = NexButton(4, 8, "bDown2");

NexButton b01 = NexButton(0, 2, "b01");
NexButton b10 = NexButton(1, 3, "b10");
NexButton b12 = NexButton(1, 2, "b12");
NexButton b21 = NexButton(2, 3, "b21");
NexButton b23 = NexButton(2, 2, "b23");
NexButton b32 = NexButton(3, 6, "b32");
NexButton b34 = NexButton(3, 7, "b34");
NexButton b43 = NexButton(4, 6, "b43");


NexTouch *nex_listen_list[] = {
    &bOn,
    &bOff,
    &h0,
    &b01,
    &b12,
    &b21,
    &b23,
    &b32,
    &b34,
    &b43,
    &bReset,
    &bColor1,
    &bType1,
    &bMonth1,
    &bDay1,
    &bDown1,
    &bTop1,
    &bColor2,
    &bType2,
    &bMonth2,
    &bDay2,
    &bDown2,
    &bTop2,
    NULL
};


char *filamentColors[] = {"Red", "Orange", "Yellow", 
  "Green", "Blue", "Cyan", "Purple", "Pink", "Black", "Gray", "White",
  "Brown", "Gold", "Silver", "Bronze", "Multi", "Clear", "Other"};

char *filamentTypes[] = {"ABS", "PLA", "PVA", "PET", "TPE", "TPU", "PC", "Other"};

char *months[] = {"Jan", "Feb", "Mar", 
  "Apr", "May", "Jun", "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec"};

char *days[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
    "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};


void bOnPopCallback(void *ptr) {
  trigger = true;
  humidityControl();
}


void bOffPopCallback(void *ptr) {
  trigger = false;
  digitalWrite(RELAYPIN, LOW);
}

void bResetPopCallback(void *ptr) {
  updateWeather();
}

void page1PopCallback(void *ptr) {
  updateWeather();
}

void page2PopCallback(void *ptr) {
  tHumSlider.setText(String(desiredHumidity).c_str());
  h0.setValue(desiredHumidity);
}

void page3PopCallback(void *ptr) {
  bColor1.setText(color1.c_str());
  bType1.setText(type1.c_str());
  bMonth1.setText(month1.c_str());
  bDay1.setText(day1.c_str());
}


void page4PopCallback(void *ptr) {
  bColor2.setText(color2.c_str());
  bType2.setText(type2.c_str());
  bMonth2.setText(month2.c_str());
  bDay2.setText(day2.c_str());
}

void h0PopCallback(void *ptr) {
  uint32_t number = 0;
  char temp[10] = {0};
  // change text with the current slider value
  h0.getValue(&number);
  utoa(number, temp, 10);
  desiredHumidity = number;
  tHumSlider.setText(temp);
}

void bColor1PopCallback(void *ptr) {
  changeSetting = 0;
}

void bType1PopCallback(void *ptr) {
  changeSetting = 1;
}

void bMonth1PopCallback(void *ptr) {
  changeSetting = 2;
}

void bDay1PopCallback(void *ptr) {
  changeSetting = 3;
}

void bColor2PopCallback(void *ptr) {
  changeSetting = 0;
}

void bType2PopCallback(void *ptr) {
  changeSetting = 1;
}

void bMonth2PopCallback(void *ptr) {
  changeSetting = 2;
}

void bDay2PopCallback(void *ptr) {
  changeSetting = 3;
}

void bDown1PopCallback(void *ptr) {
  if (changeSetting == 0) {
    color1 = filamentColors[colorCount1];
    bColor1.setText(color1.c_str());
    colorCount1++;
    if (colorCount1 > 17) {
      colorCount1 = 0;
    }
  } else if (changeSetting == 1) {
    type1 = filamentTypes[typeCount1];
    bType1.setText(type1.c_str());
    typeCount1++;
    if (typeCount1 > 7) {
      typeCount1 = 0;
    }
  } else if (changeSetting == 2) {
    month1 = months[monthCount1];
    bMonth1.setText(month1.c_str());
    monthCount1++;
    if (monthCount1 > 11) {
      monthCount1 = 0;
    }
  } else if (changeSetting == 3) {
    day1 = days[dayCount1];
    bDay1.setText(day1.c_str());
    dayCount1++;
    if (dayCount1 > 30) {
      dayCount1 = 0;
    }
  }
}

void bTop1PopCallback(void *ptr) {
  //0 is for color
  if (changeSetting == 0) {
    colorCount1--;  
    if (colorCount1 < 0) {
        colorCount1 = 0;                                                     
    }
    color1 = filamentColors[colorCount1];
    bColor1.setText(color1.c_str());
  } else if (changeSetting == 1){
    typeCount1--; 
    if (typeCount1 < 0) {
        typeCount1 = 0;                                                     
    }
    type1 = filamentTypes[typeCount1];
    bType1.setText(type1.c_str());
  } else if (changeSetting == 2){
    monthCount1--; 
    if (monthCount1 < 0) {
        monthCount1 = 0;                                                     
    }
    month1 = months[monthCount1];
    bMonth1.setText(month1.c_str());
  } else if (changeSetting == 3) {
    dayCount1--;
    if (dayCount1 < 0) {
      dayCount1 = 0;
    }
    day1 = days[dayCount1];
    bDay1.setText(day1.c_str());
  }
}

void bDown2PopCallback(void *ptr) {
  if (changeSetting == 0) {
    color2 = filamentColors[colorCount2];
    bColor2.setText(color2.c_str());
    colorCount2++;
    if (colorCount2 > 27) {
      colorCount2 = 0;
    }
  } else if (changeSetting == 1) {
    type2 = filamentTypes[typeCount2];
    bType2.setText(type2.c_str());
    typeCount2++;
    if (typeCount2 > 7) {
      typeCount2 = 0;
    }
  } else if (changeSetting == 2) {
    month2 = months[monthCount2];
    bMonth2.setText(month2.c_str());
    monthCount2++;
    if (monthCount2 > 22) {
      monthCount2 = 0;
    }
  } else if (changeSetting == 3) {
    day2 = days[dayCount2];
    bDay2.setText(day2.c_str());
    dayCount2++;
    if (dayCount2 > 30) {
      dayCount2 = 0;
    }
  }
}

void bTop2PopCallback(void *ptr) {
  //0 is for color
  if (changeSetting == 0) {
    colorCount2--;  
    if (colorCount2 < 0) {
        colorCount2 = 0;                                                     
    }
    color2 = filamentColors[colorCount2];
    bColor2.setText(color2.c_str());
  } else if (changeSetting == 1){
    typeCount2--; 
    if (typeCount2 < 0) {
        typeCount2 = 0;                                                     
    }
    type2 = filamentTypes[typeCount2];
    bType2.setText(type2.c_str());
  } else if (changeSetting == 2){
    monthCount2--; 
    if (monthCount2 < 0) {
        monthCount2 = 0;                                                     
    }
    month2 = months[monthCount2];
    bMonth2.setText(month2.c_str());
  } else if (changeSetting == 3) {
    dayCount2--;
    if (dayCount2 < 0) {
      dayCount2 = 0;
    }
    day2 = days[dayCount2];
    bDay2.setText(day2.c_str());
  }
}

//Retrieves the humidity, temperatures and updates the page 2 of the display with the values
void updateWeather() {
  //humidity = sensor.getRH();
  humidity = sensor.getRH();
  tempC = String(sensor.getTemp());
  tempF = String(sensor.getTempF());
  tTempC.setText(tempC.c_str());
  tTempF.setText(tempF.c_str());
  tHum.setText(String(humidity).c_str());
  
}

//Humidity control.  Set a desired humidity via the slider and the device will trigger relay. 
//Programmed to have a 0.5 range from the desired to save power
void humidityControl() {
  if (humidity >= desiredHumidity + 0.5) {
     digitalWrite(RELAYPIN, HIGH);
  } else if (humidity <= desiredHumidity - 0.5){
    digitalWrite(RELAYPIN, LOW);
  } else {
    digitalWrite(RELAYPIN, HIGH);
  }
}

void setup() {
  // put your setup code here, to run once:
  HMISerial.begin(9600);
  Serial.begin(9600);   // open serial over USB at 9600 baud

  //Initialize the I2C sensors and ping them
  sensor.begin();  
  // You might need to change NexConfig.h file in your ITEADLIB_Arduino_Nextion folder
  // Set the baudrate which is for debug and communicate with Nextion screen
  nexInit();

  // Register the pop event callback function of the components
  bOn.attachPop(bOnPopCallback, &bOn);
  bOff.attachPop(bOffPopCallback, &bOff);
  b01.attachPop(page1PopCallback, &b01);
  b21.attachPop(page1PopCallback, &b21);
  b12.attachPop(page2PopCallback, &b12);
  b32.attachPop(page2PopCallback, &b32);
  b23.attachPop(page3PopCallback, &b23);
  b43.attachPop(page3PopCallback, &b43);
  b34.attachPop(page4PopCallback, &b34);
  h0.attachPop(h0PopCallback, &h0);
  bColor1.attachPop(bColor1PopCallback, &bColor1);
  bType1.attachPop(bType1PopCallback, &bType1);
  bMonth1.attachPop(bMonth1PopCallback, &bMonth1);
  bDay1.attachPop(bDay1PopCallback, &bDay1);
  bDown1.attachPop(bDown1PopCallback, &bDown1);
  bTop1.attachPop(bTop1PopCallback, &bTop1);
  bColor2.attachPop(bColor2PopCallback, &bColor2);
  bType2.attachPop(bType2PopCallback, &bType2);
  bMonth2.attachPop(bMonth2PopCallback, &bMonth2);
  bDay2.attachPop(bDay2PopCallback, &bDay2);
  bDown2.attachPop(bDown2PopCallback, &bDown2);
  bTop2.attachPop(bTop2PopCallback, &bTop2);
  bReset.attachPop(bResetPopCallback, &bReset);
  pinMode(RELAYPIN, OUTPUT);  
  next = millis();
}

void loop() {
  
  nexLoop(nex_listen_list);
  if (millis() >= next){
    next = millis() + 500;
      if (trigger) {
      humidityControl();
    }
  } 
}
