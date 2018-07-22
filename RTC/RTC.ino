#include "LedControl.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <RtNTPClient.h>

const char *ssid     = "REDACTED";
const char *password = "REDACTED";
const int timezone = -7 * 60 * 60;
const int loopDelay = 50; // This is 2x the min resolution and its set to not overwhelm the display
const int updateWithNTPClient = (30 * 60 * 1000) / loopDelay; // Update every 30 mins with NTP

WiFiUDP ntpUDP;
RtNTPClient timeClient(ntpUDP);
LedControl lc=LedControl(D8,D7,D6,1);

// Globals
int currentMinutes;
int currentHours;
int currentSecs;
int currentMillisecs;
unsigned long currentEpoc;
long lastUpdateMillis = 0;
int syncWithNTPCount = 0;

void syncTimeWithNTP() {
  // NTP has updated, so lets do some blinking to inform the user
  for(int i=0; i<8; i++){
     lc.setDigit(0,i,8,true);
  } 
  delay(100);
  lc.clearDisplay(0);
  delay(100);
        
  currentHours = timeClient.getHours();
  currentMinutes = timeClient.getMinutes();
  currentSecs = timeClient.getSeconds();
  currentEpoc = timeClient.getEpochTime();
  Serial.print("Time sync with NTP: ");
  Serial.println(timeClient.getFormattedTime());
}

void displayTest() {
 // Blink
 for(int k=0; k<4; k++){
   for(int i=0; i<8; i++){
     lc.setDigit(0,i,8,true);
   }  
   delay(100);
   lc.clearDisplay(0);
   delay(100);
 } 

 // Display 0 through E
 for(int k=0; k<15; k++){
   for(int i=0; i<8; i++){
    lc.setDigit(0,i,k,false);
  }
  delay(300);
 }
}

void displayWifiError() {
  // Block the boot process since we can't connect to the wifi or sync with ntp
  while (1) {
    lc.setChar(0,7,'1',false);
    lc.setChar(0,6,'F',false);
    lc.setChar(0,5,'1',false);
    delay(500);
    lc.clearDisplay(0);  
    delay(500);
  }
}

void printfWrapper(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Serial.printf(fmt, args);
  va_end(args);
}

void setup()
{
  // Initialize the MAX7219 based display
  lc.shutdown(0, false);  // Enable display
  lc.setIntensity(0, 15); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display
  
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  Serial.println("Startup");

  // Wait for wifi to connect
  int wifiUpTimeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print ( "." );
    if (wifiUpTimeout > 120) {
      displayWifiError();
    }
    wifiUpTimeout++;
  }

  Serial.println("Wifi connected");
  timeClient.setTimeOffset(timezone);
  timeClient.setPrintDebugFunc(&printfWrapper);

  // Wait for the ntp client to set the time
  Serial.println("NTP client start the update procedure");
  while (timeClient.forceUpdate() == false) {
    displayTest();
  }
  Serial.println("NTP time updated");
 
  syncTimeWithNTP();
}


void publishTimeToDisplay() {
  long millicurrentSecss = millis();
  currentMillisecs = int(millicurrentSecss - lastUpdateMillis);

  if (currentMillisecs >= 1000) {
    currentEpoc += 1;
    currentMillisecs = currentMillisecs % 1000;
    lastUpdateMillis = millicurrentSecss;
  }
 
  currentHours = ((currentEpoc % 86400L) / 3600);
  currentMinutes = ((currentEpoc % 3600) / 60);
  currentSecs = (currentEpoc % 60);
}

void loop()
{
    publishTimeToDisplay();

    int ms0 = int(currentMillisecs / 100) % 10;
    int ms1 = int(currentMillisecs / 10) % 10;

    int currentSecs0 = int(currentSecs) % 10;
    int currentSecs1 = int(currentSecs / 10) % 10;

    int min0 = int(currentMinutes) % 10;
    int min1 = int(currentMinutes / 10) % 10; 

    int h0 = int(currentHours) % 10;
    int h1 = int(currentHours / 10) % 10;
   
    lc.setDigit(0,7,h1,false);
    lc.setDigit(0,6,h0,true);
    lc.setDigit(0,5,min1,false);
    lc.setDigit(0,4,min0,true);
    lc.setDigit(0,3,currentSecs1,false);
    lc.setDigit(0,2,currentSecs0,true);
    lc.setDigit(0,1,ms0,false);
    lc.setDigit(0,0,ms1,false);
    
    delay(loopDelay); 
    
    timeClient.update();

    if (syncWithNTPCount > updateWithNTPClient) {
        syncTimeWithNTP();
        syncWithNTPCount = 0; 
    }
    
    syncWithNTPCount++;
}
