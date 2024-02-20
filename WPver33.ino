// write data to file each day and restore at startup
// Read Time From NTP At Startup Then set internal RTC with Time From NTP
// Then Code Takes Its Time From RTC
// all button to run motor from pin 14


#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Arduino_ESP32_OTA.h>
#include "time.h"
#include <string>
#include <iostream>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <SPIFFS.h>
#include "FS.h"
#include <SPI.h>

const char* root_ca = \
/*DigiCert TLS Hybrid ECC SHA384 2020 CA1*/
"-----BEGIN CERTIFICATE-----\n" \
"MIIEFzCCAv+gAwIBAgIQB/LzXIeod6967+lHmTUlvTANBgkqhkiG9w0BAQwFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0yMTA0MTQwMDAwMDBaFw0zMTA0MTMyMzU5NTlaMFYxCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxMDAuBgNVBAMTJ0RpZ2lDZXJ0IFRMUyBI\n" \
"eWJyaWQgRUNDIFNIQTM4NCAyMDIwIENBMTB2MBAGByqGSM49AgEGBSuBBAAiA2IA\n" \
"BMEbxppbmNmkKaDp1AS12+umsmxVwP/tmMZJLwYnUcu/cMEFesOxnYeJuq20ExfJ\n" \
"qLSDyLiQ0cx0NTY8g3KwtdD3ImnI8YDEe0CPz2iHJlw5ifFNkU3aiYvkA8ND5b8v\n" \
"c6OCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFAq8CCkXjKU5\n" \
"bXoOzjPHLrPt+8N6MB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA4G\n" \
"A1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwdgYI\n" \
"KwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\n" \
"b20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdp\n" \
"Q2VydEdsb2JhbFJvb3RDQS5jcnQwQgYDVR0fBDswOTA3oDWgM4YxaHR0cDovL2Ny\n" \
"bDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNybDA9BgNVHSAE\n" \
"NjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwBAgEwCAYGZ4EMAQICMAgG\n" \
"BmeBDAECAzANBgkqhkiG9w0BAQwFAAOCAQEAR1mBf9QbH7Bx9phdGLqYR5iwfnYr\n" \
"6v8ai6wms0KNMeZK6BnQ79oU59cUkqGS8qcuLa/7Hfb7U7CKP/zYFgrpsC62pQsY\n" \
"kDUmotr2qLcy/JUjS8ZFucTP5Hzu5sn4kL1y45nDHQsFfGqXbbKrAjbYwrwsAZI/\n" \
"BKOLdRHHuSm8EdCGupK8JvllyDfNJvaGEwwEqonleLHBTnm8dqMLUeTF0J5q/hos\n" \
"Vq4GNiejcxwIfZMy0MJEGdqN9A57HSgDKwmKdsp33Id6rHtSJlWncg+d0ohP/rEh\n" \
"xRqhqjn1VtvChMQ1H3Dau0bwhr9kAMQ+959GG50jBbl9s08PqUU643QwmA==\n" \
"-----END CERTIFICATE-----\n"
/*DigiCert TLS RSA SHA256 2020 CA1*/
"-----BEGIN CERTIFICATE-----\n" \
"MIIEvjCCA6agAwIBAgIQBtjZBNVYQ0b2ii+nVCJ+xDANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0yMTA0MTQwMDAwMDBaFw0zMTA0MTMyMzU5NTlaME8xCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxKTAnBgNVBAMTIERpZ2lDZXJ0IFRMUyBS\n" \
"U0EgU0hBMjU2IDIwMjAgQ0ExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n" \
"AQEAwUuzZUdwvN1PWNvsnO3DZuUfMRNUrUpmRh8sCuxkB+Uu3Ny5CiDt3+PE0J6a\n" \
"qXodgojlEVbbHp9YwlHnLDQNLtKS4VbL8Xlfs7uHyiUDe5pSQWYQYE9XE0nw6Ddn\n" \
"g9/n00tnTCJRpt8OmRDtV1F0JuJ9x8piLhMbfyOIJVNvwTRYAIuE//i+p1hJInuW\n" \
"raKImxW8oHzf6VGo1bDtN+I2tIJLYrVJmuzHZ9bjPvXj1hJeRPG/cUJ9WIQDgLGB\n" \
"Afr5yjK7tI4nhyfFK3TUqNaX3sNk+crOU6JWvHgXjkkDKa77SU+kFbnO8lwZV21r\n" \
"eacroicgE7XQPUDTITAHk+qZ9QIDAQABo4IBgjCCAX4wEgYDVR0TAQH/BAgwBgEB\n" \
"/wIBADAdBgNVHQ4EFgQUt2ui6qiqhIx56rTaD5iyxZV2ufQwHwYDVR0jBBgwFoAU\n" \
"A95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQG\n" \
"CCsGAQUFBwMBBggrBgEFBQcDAjB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGG\n" \
"GGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2Nh\n" \
"Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNydDBCBgNV\n" \
"HR8EOzA5MDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRH\n" \
"bG9iYWxSb290Q0EuY3JsMD0GA1UdIAQ2MDQwCwYJYIZIAYb9bAIBMAcGBWeBDAEB\n" \
"MAgGBmeBDAECATAIBgZngQwBAgIwCAYGZ4EMAQIDMA0GCSqGSIb3DQEBCwUAA4IB\n" \
"AQCAMs5eC91uWg0Kr+HWhMvAjvqFcO3aXbMM9yt1QP6FCvrzMXi3cEsaiVi6gL3z\n" \
"ax3pfs8LulicWdSQ0/1s/dCYbbdxglvPbQtaCdB73sRD2Cqk3p5BJl+7j5nL3a7h\n" \
"qG+fh/50tx8bIKuxT8b1Z11dmzzp/2n3YWzW2fP9NsarA4h20ksudYbj/NhVfSbC\n" \
"EXffPgK2fPOre3qGNm+499iTcc+G33Mw+nur7SpZyEKEOxEXGlLzyQ4UfaJbcme6\n" \
"ce1XR2bFuAJKZTRei9AqPCCcUZlM51Ke92sRKw2Sfh3oius2FkOH6ipjv3U/697E\n" \
"A7sKPPcw7+uvTPyLNhBzPvOk\n" \
"-----END CERTIFICATE-----\n";


// #define ServerVersion "1.0"
String  webpage = "";
bool    SPIFFS_present = false;

#ifndef STASSID                        // either use an external .h file containing STASSID and STAPSK
                                    // or

#define STASSID "AMGAD"
#define STAPSK  "onTESTtwo"
//#define STASSID "Recvi"
//#define STAPSK  "W$@rPC!yz"
#endif
#define test 14
const char* ssid = STASSID;
const char* password = STAPSK;

// Set your Static IP address
//IPAddress local_IP(192, 168, 30, 120);
// Set your Gateway IP address
IPAddress gateway(192, 168, 30, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

Arduino_ESP32_OTA ota;
Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

const char* ntpServer = "pool.ntp.org";
const long gmtOffsetInSeconds = 7200;
const int daylightOffsetInSeconds = 7200;

const char* fileNameFormat = "/%02d%04d.txt";

unsigned long previousMillis = 0;  // Variable to store the previous time
const unsigned long interval = 3600000;  // Interval of one hour in milliseconds


bool wifiConnected = false;
bool dataSaved = false;
bool restr = false;

String table1 = "<table><tr><th>Date</th><th>Runtime 1</th><th>Runtime 2</th><th>Runtime 3</th><th>Separate</th><th>Total 1</th><th>Total 2</th><th>Total 3</th><th>Total Separate</th></tr>";

int sum;
int sumd;

String currentFileName;
bool ledOn    = false;
bool StairON    = false;
float onTime  = 0;
float onTimeStair = 0;
float tempTime = 0;
float Ttime  = 0;
float Mtime  = 0;
int Stair_Hour_ON = 18;
int Stair_Hour_OFF = 6;
int Stair_min_ON = 30;
int Stair_min_OFF = 30;
int CHECKPOINT_time = 0;
int CHECKPOINT_min = 0;
int Stair_PIN = 33;
String message = "";
struct tm timeinfo;
bool datasa = false;
bool allowed_hour = false;
bool OVERRIDES = false;
bool MOverride = false;
bool notfi = false;
bool notfi1 = false;
bool notfi2 = false;

WebServer server(6060);

// this code works fine and calculate total time of each led on
//const uint8_t b1 = 4;  // Button ////////
//const uint8_t b2 = 22;  /////////
//const uint8_t b3 = 15;
const uint8_t b4 = 16; // MOTOR INPUT?OUTPUT
//const uint8_t r1 = 21; // Output relay
const uint8_t r2 = 19; // stair relay
const uint8_t r3 = 18; // mOverride
const uint8_t r4 = 23; // MOTOR LED
const uint8_t commonStatePin = 5; // RUNTIME_LED
const int LEDPINS[] = {5, 21, 23, 33};
const int nLEDS = sizeof(LEDPINS) / sizeof(LEDPINS[0]);
bool initialLEDS[nLEDS];


void SaveState(){
  for (int x=0; x< nLEDS; x++){
    initialLEDS[x] = digitalRead(LEDPINS[x]);
  }
}
void LoadState(){
  for (int x=0; x< nLEDS; x++){
    digitalWrite(LEDPINS[x], initialLEDS[x]);
  }
}
void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  //  Serial.print(".");
  //}
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED){
  Serial.println("WiFi connected");
  Serial.print  ("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to WiFi");
  //wifiConnected = true;
  }
}

void connectToWiFiLOOP() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print(".");
  //}
  for (int x=0; x< nLEDS; x++){
        digitalWrite(LEDPINS[x], HIGH);
        delay(100);
        digitalWrite(LEDPINS[x], LOW);
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED){
  Serial.println("WiFi connected");
  Serial.print  ("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to WiFi");
  //wifiConnected = true;
  }
}


void listFiles() {
  File root = SPIFFS.open("/");

  String highestDateFileName;
  unsigned long highestDate = 0;
  int rMonth = timeinfo.tm_mon ;
  int rYear = timeinfo.tm_year + 1900;
  unsigned long cd = rYear * 100 + rMonth;
  Serial.println(rMonth);
  Serial.println(rYear);

  File file = root.openNextFile();

  while (file) {
    String fileName = file.name();
    Serial.println(fileName);

    // Extract month and year from the filename
    int month, year;
    if (sscanf(fileName.c_str(), "%02d%04d.txt", &month, &year) == 2) {
      Serial.println(month);
      Serial.println(year);

      unsigned long currentDate = year * 100 + month;

      // Compare with the highest date so far
      if (currentDate > highestDate) {
        highestDate = currentDate;
        highestDateFileName = fileName;
      }
      Serial.println("DD: " + String(currentDate) + " " + String(highestDate));
    }

    

    file = root.openNextFile();
  }

  root.close();

  // Set the current file name to the one with the highest date
  if (cd > highestDate){
    highestDateFileName = String(rMonth) + String(rYear) +".txt";
    Serial.println("Changed To Higher Date");
  }
  currentFileName = highestDateFileName;
  Serial.println("Current File: " + currentFileName);
}



void restart()
{
saveCHECKPOINT(1);
delay(3000);
ESP.restart();
}

void restartWEB()
{
//saveDataToFile();
server.stop();
delay(1000);
server.begin();
}
void saveCHECKPOINT (int notf) {
  File file = SPIFFS.open("/CHECKPOINT", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  String dataLine = String(timeinfo.tm_mday) + ",";
  int time = Ttime;
  dataLine += String(time)+",";
  dataLine += String(CHECKPOINT_time)+",";
  dataLine += String(CHECKPOINT_min)+",";
  if (file.println(dataLine)) {
    Serial.println("[CHECKPOINT] Data written successfully");
  } else {
    Serial.println("[CHECKPOINT] Failed to write data to file");
  }
  file.close();
  saveDataToFile();
  if (notf){notfit(7);}
}

void saveDataToFile() {
  //char fileName[20];
  //snprintf(fileName, sizeof(fileName), fileNameFormat, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  //File file = SPIFFS.open(fileName, FILE_APPEND);
  File file = SPIFFS.open("/" + currentFileName, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  String dataLine = String(timeinfo.tm_mday) + "-" + String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_year + 1900) + ",";
  int time = Ttime;
  int time1 = Mtime;
  dataLine += String(time1)+",";
  dataLine += String(time)+",";
  dataLine += String(Stair_Hour_ON)+",";
  dataLine += String(Stair_Hour_OFF)+",";  
  dataLine += String(Stair_min_ON)+",";
  dataLine += String(Stair_min_OFF)+",";
  dataLine += String(Stair_PIN);
  if (file.println(dataLine)) {
    Serial.println("Data written successfully");
  } else {
    Serial.println("Failed to write data to file");
  }
  file.close();
  }

std::vector<String> lastcells() {
  Serial.println("Last Cells FUNC");
  String recentFileName = currentFileName;
  static bool dataLoaded = false;
  static std::vector<String> lines; // Store all lines from the file

  // Load data from file on startup
  if (!dataLoaded) {
    // Serial.print("Load data from file on startup ");
    //listDir("/", recentFileName);
    Serial.print("Recent File Name: ");
    Serial.println(recentFileName);

    if (!recentFileName.isEmpty()) {
      Serial.print("Recent File: ");
      Serial.println(recentFileName);

      // Open the recent file for reading
      File recentFileHandle = SPIFFS.open("/" + recentFileName, "r");
      if (recentFileHandle) {
        // Read all lines from the file
        while (recentFileHandle.available()) {
          
          String line = recentFileHandle.readStringUntil('\n');
          lines.push_back(line);
        }
        recentFileHandle.close();
        dataLoaded = true;
      } else {
        Serial.println("Failed to open recent file for reading.");
      }
    } else {
      Serial.println("No recent files found.");
    }
  }

  // Extract the last line and the last 4 values
  std::vector<String> lastValues;
  if (!lines.empty()) {
    String lastLine = lines.back();
//    Serial.print("Last Line: ");
//    Serial.println(lastLine);

    // Extract the values from the last line
    std::vector<String> values;
    int startIndex = 0;
    int endIndex;
    while ((endIndex = lastLine.indexOf(',', startIndex)) != -1) {
      values.push_back(lastLine.substring(startIndex, endIndex));
      startIndex = endIndex + 1;
    }
    values.push_back(lastLine.substring(startIndex));

    // Check if there are at least 9 values in the last line
    if (values.size() >= 8) {
      //lastValues.push_back(values[5]); // inT0
      //lastValues.push_back(values[6]); // inT1
      //lastValues.push_back(values[7]); // inT2
      //lastValues.push_back(values[8]); // sepT
      lastValues.push_back(values[1]); // Mtime
      lastValues.push_back(values[2]); // Ttime
      lastValues.push_back(values[3]); // On hour
      lastValues.push_back(values[4]); // Off hour
      lastValues.push_back(values[5]); // On min
      lastValues.push_back(values[6]); // Off min
      lastValues.push_back(values[7]); // PIN
    } else {
      Serial.println("Insufficient values in the last line.");
      Serial.println(values.size());
    }
  } else {
    Serial.println("No data available.");
  }

  return lastValues;
}


void loadCHECKPOINT() {
  Serial.println("CHECKPOINT Recover");
  static bool dataLoaded = false;
  static std::vector<String> lines;

  File F = SPIFFS.open("/CHECKPOINT", "r");
  if (F)
  {
    while (F.available()) {
          
          String line = F.readStringUntil('\n');
          lines.push_back(line);
    }
    F.close();
  }
  else {
        Serial.println("Failed to open CHECKPOINT file for reading.");
        return;
  }

  // Extract the last line and the last 4 values
  std::vector<String> lastValues;
  if (!lines.empty()) {
    String lastLine = lines.back();
//    Serial.print("Last Line: ");
//    Serial.println(lastLine);

    // Extract the values from the last line
    std::vector<String> values;
    int startIndex = 0;
    int endIndex;
    while ((endIndex = lastLine.indexOf(',', startIndex)) != -1) {
      values.push_back(lastLine.substring(startIndex, endIndex));
      startIndex = endIndex + 1;
    }
    values.push_back(lastLine.substring(startIndex));

    // Check if there are at least 9 values in the last line
    if (values.size() >= 3) {
      lastValues.push_back(values[0]); // DATE
      lastValues.push_back(values[1]); // Ttime
      lastValues.push_back(values[2]); // CHECKPOINT TIME
      lastValues.push_back(values[3]); // CHECKPOINT MIN
    } else {
      Serial.println("Insufficient values in the last line.");
      Serial.println(values.size());
      return;
    }

    if (timeinfo.tm_mday == lastValues[0].toInt()){
      // RESTORE
      Ttime += lastValues[1].toInt();
      CHECKPOINT_time = lastValues[2].toInt();
      CHECKPOINT_min = lastValues[3].toInt();
      notfit(8);
      
    }
    else{
      // make new checkpoint with TIME = 0
      CHECKPOINT_time = lastValues[2].toInt();
      CHECKPOINT_min = lastValues[3].toInt();
      saveCHECKPOINT(0);
    }

  } else {
    Serial.println("[CHECKPOINT] No data available.");
  }
}


String timeInLetters(int t) {
    int h, m, s;

    h = t / 3600;  // hours
    t %= 3600;     // remaining seconds after extracting hours
    m = t / 60;    // minutes
    s = t % 60;    // remaining seconds after extracting minutes

    String tss = "";

    if (h > 0) {
        tss += String(h) + " hour ";
    }

    if (m > 0) {
        tss += String(m) + " min ";
    }

    if (s > 0) {
        tss += String(s) + " s ";
    }

    if (tss == "") {
        tss = "0 s";  // if the time is zero, represent it as 0 seconds
    }

    Serial.println(tss);
    return tss;
}


void handleRoot() {

  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>Relay Status</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<meta http-equiv='refresh' content='2'>\n"
                "<link rel='icon' href='data:,'>\n"                      // avoid favicon request, might not work for Safari
                "<script src='j.js'></script>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif}*{box-sizing:border-box}.table{width:100%;border-collapse:collapse}.table td,.table th{padding:12px 15px;border:1px solid #ddd;text-align:center;font-size:25px;font-weight:bold;color:black;}.table th{background-color:#46494d;color:#fff}.table1{width:100%;border-collapse:collapse}.table1 td,.table1 th{padding:12px 15px;border:1px solid #ddd;text-align:center;font-size:18px;font-weight:bold;color:black;}.table1 th{background-color:#970000;color:#fff}.switch {position: relative;display: inline-block;width: 60px;height: 34px;}.switch input {opacity: 0;width: 0;height: 0;}.slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;}.slider:before {position: absolute;content: '';height: 26px;width: 26px;left: 4px;bottom: 4px;background-color: white;-webkit-transition: .4s;transition: .4s;}input:checked + .slider {background-color: #2196F3;}input:focus + .slider {box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before {-webkit-transform: translateX(26px);-ms-transform: translateX(26px);transform: translateX(26px);}.slider.round {border-radius: 34px;}.slider.round:before {border-radius: 50%;}"
                "</style>\n"
                "</head>\n"
                "<body>\n"
                "<table class='table'>\n"
                "<thead>\n"
                "<th>Floor</th>\n"
                "<th>Status</th>\n"
                "<th>Control</th>\n"
                "<th>Total H/M</th>\n"
                "<th>Total KW/M</th>\n"
                "<th>KW Today</th>\n"
                "</thead>\n"
                "<tbody>\n";

  //  this is the common state row  MAIN
  String color = digitalRead(commonStatePin) == LOW ? "red" : "green";
  html += "<tr class='colomn4'>";
  html += "<td data-label='Floor'>Main</td>";
  html += "<td data-label='Status'>";
  html += "<div style='display:inline-block;margin:10px;width:200px;height:70px;border:1px solid black;background-color:" + color + ";'><p style='text-align:center;font-size:30px;font-weight:bold;color:black;'>" + String(digitalRead(commonStatePin) == LOW ? "OFF" : "ON") + "</p>\n";
  html += "</div></td>";
  html += "<td data-label='Control'><a href = 'cmd?cmd=reset' target='i'>Reset M</a></td>";  // dirty call as href
  html += "<td data-label='Total H/M'><a href = 'cmdd?cmdd=resetd' target='i'>Reset D</a></td>";  // dirty call as href
  html += "</td>";
  html += "<td data-label='Total KW/M'>" + String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900) + "</td>";
  html += "<td data-label='KW Today'> " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec) + "</td>";

  // this is the separate output MOTOR
  color = ledOn == 0 ? "red" : "green";
  int time1 = Mtime;
  int time = Ttime;
  html += "<tr class='colomn4'>";
  html += "<td data-label='Floor'>Motor</td>";
  html += "<td data-label='Status'>";
  html += "<div style='display:inline-block;margin:10px;width:200px;height:70px;border:1px solid black;background-color:" + color + ";'><p style='text-align:center;font-size:25px;font-weight:bold;color:black;'>" + String(timeInLetters(time1) ) + "</p>\n";
  html += "</div></td>";
  html += "<td data-label='Control'>";
  html += "<label class='switch'>";     // The Switch
    html += "<input type='checkbox' ";
     if (MOverride == false)
      html += "value='checked' checked='checked' ";
    html += "onclick='firecb(this.id)' id='resbutton";
    html += "'>";
    html += "<span class='slider round'></span>";
    html += "</label>";
    html += "</td>";



  html += "<td data-label='Total H/M'>" + String(time1 / 3600) + " H</td>";
  html += "<td data-label='Total KW/M'>" + String(time1 * 0.92 / 3600.00) + " KW</td>";
  html += "<td data-label='KW Today'>" + String(time / 60) + "Min - " + String(time * 0.92 / 3600.00) + "KW</td>";

  // a row with Totals
  color = digitalRead(b4) == HIGH ? "red" : "green";
  sum = time1;
  sumd = time1;
  html += "<tr class='colomn4'>";
  html += "<td data-label='Floor'>Totals</td>";
  html += "<td data-label='Status'>";
  html += timeInLetters(time1);
  html += "</td>";

  html += "<td data-label='Control'>";
  html += "<div style='display:inline-block;margin:10px;width:200px;height:70px;border:1px solid black;background-color:" + color + ";'><p style='text-align:center;font-size:30px;font-weight:bold;color:black;'>" + "Motor "+ (digitalRead(b4) == LOW ? "Run" : "Stop") + "</p>\n";
  html += "</div></td>";


  html += "<td data-label='Total H/M'>  ";
  html += sum / 3600 ;
  html += " H</td>";
  html += "<td data-label='Total KW/M'> ";
  html += sum * 0.92 / 3600.00;
  html += " KW</td>";
  html += "<td data-label='KW Today'> ";
  html += sumd * 0.92 / 3600.00;
  html += " KW/Day</td>";
  html += "</tr>";
//motor timer
  String cor = "style='background-color: red; color: black;'";
  if (ledOn){cor = "style='background-color: green; color: white;'";}
  html += "<tr>";
  html += "<td ";
  html += cor;
  html += " colspan='6'><br>";
  html += "Total Time Today: ";
  html += timeInLetters(time);
  html += "<br><br></td></tr>";
// STAIRS State
  String corr = "style='background-color: red; color: black;'";
  if (digitalRead(Stair_PIN) == HIGH){corr = "style='background-color: green; color: white;'";}
  html += "<tr>";
  html += "<td ";
  html += corr;
  html += " colspan='6'><br>";
  html += "Stairs LED";
  html += "<br><br></td></tr>";

  html += "</tbody>\n";
  html += "</table>\n";
  html += "<iframe name='i' style='display:none'></iframe>\n";               // dirty hack to keep
  html += "</div>\n";
  html += "<br><br>\n";
  for (int i = 0; i < 15; i++){
  html += "&nbsp;";  }
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmdr?cmdr=rest';\" >RESTART</button>";  // dirty call as href
  html += "<br><br>\n";
  for (int i = 0; i < 30; i++){
  html += "&nbsp;";  }
  // html += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
  //html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmdl?cmdl=moto';\" >RUN MOTOR</button>";  // dirty call as href
  //html += "<br><br>\n";
  //for (int i = 0; i < 45; i++){
  //html += "&nbsp;";  }
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmdw?cmdw=data';\" >Dialy Report</button>";  // dirty call as href
  html += "<br><br>\n";
  for (int i = 0; i < 60; i++){
  html += "&nbsp;";  }
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmdf?cmdf=files';\" >FILES</button>";  // dirty call as href
  html += "<br><br>\n";
  for (int i = 0; i < 70; i++){
  html += "&nbsp;";  }
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'moto';\" >MOTOR TIME</button>";  // dirty call as href
  html += "<br><br>\n";
  for (int i = 0; i < 85; i++){
  html += "&nbsp;";  }
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'timer';\" >Stairs Control</button>";  // dirty call as href
  html += "&nbsp;";
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'update';\" >OTA UPDATE</button>";  // dirty call as href
  html += "</body>\n</html>";
  server.send(200, "text/html", html);
}

void handleData()
{

  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>Dialy Report</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<meta http-equiv='refresh' content='5'>\n"
                "<link rel='icon' href='data:,'>\n"                    // avoid favicon request, might not work for Safari
                "<script src='j.js'></script>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif}*{box-sizing:border-box}.table{width:100%;border-collapse:collapse}.table td,.table th{padding:12px 15px;border:1px solid #ddd;text-align:center;font-size:25px;font-weight:bold;color:black;}.table th{background-color:#46494d;color:#fff}.table1{width:100%;border-collapse:collapse}.table1 td,.table1 th{padding:12px 15px;border:1px solid #ddd;text-align:center;font-size:18px;font-weight:bold;color:black;}.table1 th{background-color:#970000;color:#fff}.switch{position:relative;display:inline-block;width:60px;height:34px}.switch input{opacity:0;width:0;height:0}.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ff0000;-webkit-transition:.4s;transition:.4s}.slider:before{position:absolute;content:"";height:26px;width:26px;left:4px;bottom:4px;background-color:#fff;-webkit-transition:.4s;transition:.4s}input:checked+.slider{background-color:#2196f3}input:focus+.slider{box-shadow:0 0 1px #2196f3}input:checked+.slider:before{-webkit-transform:translateX(26px);-ms-transform:translateX(26px);transform:translateX(26px)}.slider.round{border-radius:34px}.slider.round:before{border-radius:50%}"
                "</style>\n"
                "</head>\n"
                "<body>\n"
                "<table class='table1'>\n"
                "<thead>\n"
                "<th>Date</th>\n"
                "<th>MONTH-TIME</th>\n"
                "<th>DAY-TIME</th>\n"
                "<th>Motor-KW</th>\n"
                "</thead>\n"
                "<tbody>\n";

  html += "<br><br>\n";
  html += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = '/';\" >HOME</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";  // dirty call as href
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmds?cmds=save';\" >SAVE DATA</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";  // dirty call as href
  html += "<button style='text-align:center;font-size:30px;font-weight:bold;color:black;' onclick = \"window.location.href = 'cmdf?cmdf=files';\" >FILES</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";  // dirty call as href
  html += "<br><br>\n";

 listFiles();
  String recentFileName = currentFileName;
  //listDir("/", recentFileName);
  // Serial.print("Recent File Name: ");
  // Serial.println(recentFileName);
  if (!recentFileName.isEmpty()) {
    // Serial.print("Recent File: ");
    // Serial.println(recentFileName);
  static bool dataup = false;
  static bool dataLoaded = false;
  static std::vector<String> lines; // Store all lines from the file
  // Load data from file on startup
/*
  if (!dataLoaded) {
    // Serial.print("Load data from file on startup ");
    listDir("/", recentFileName);
    // Serial.print("Recent File Name: ");
    // Serial.println(recentFileName);

    if (!recentFileName.isEmpty()) {
      // Serial.print("Recent File: ");
      // Serial.println(recentFileName);

      // Open the recent file for reading
      File recentFileHandle = SPIFFS.open(recentFileName, "r");
      if (recentFileHandle) {
        // Read all lines from the file
        while (recentFileHandle.available()) {
          String line = recentFileHandle.readStringUntil('\n');
          lines.push_back(line);
        }

        recentFileHandle.close();
        dataLoaded = true;
      } else {
        Serial.println("Failed to open recent file for reading.");
      }
    } else {
      Serial.println("No recent files found.");
    }
  }
*/
  // Update the data from file daily at 23:57
  // if (timeinfo.tm_hour == 23 && timeinfo.tm_min == 57 && !dataup ) {
    //Serial.print("Updated now DATA ");
    //listDir("/", recentFileName);
    // Serial.print("Recent File Name: ");
    // Serial.println(recentFileName);

    if (!recentFileName.isEmpty()) {
      // Serial.print("Recent File: ");
      // Serial.println(recentFileName);

      // Open the recent file for reading
      File recentFileHandle = SPIFFS.open("/" + recentFileName, "r");
      if (recentFileHandle) {
        // Read all lines from the file
        lines.clear(); // Clear the previous lines
        while (recentFileHandle.available()) {
          String line = recentFileHandle.readStringUntil('\n');
          lines.push_back(line);
        }

        recentFileHandle.close();
      //  dataup = true;
      } else {
        Serial.println("Failed to open recent file for reading.");
      }
    } else {
      Serial.println("No recent files found.");
    }
 // }

  // Display the data from the file on startup and throughout the day
  //Serial.println("OLD DATA");
  for (const auto& line : lines) {
    // Extract the values from the line
    std::vector<String> values;
    int startIndex = 0;
    int endIndex;
    while ((endIndex = line.indexOf(',', startIndex)) != -1) {
      values.push_back(line.substring(startIndex, endIndex));
      startIndex = endIndex + 1;
    }
    // Serial.print(endIndex);
    // Serial.print("Start Index");
    // Serial.print(startIndex);
    // Serial.print("Values Sizes");
    // Serial.print(values.size());
    // Serial.print(values[values.size()-1]);
    values.push_back(line.substring(startIndex));

          // Display the extracted values
                        
                         html += "<tr class='colomn1'>";
                         html += "<td data-label='Date'>" + values[0] + " </td>";
                         html += "<td data-label='MONTH-TIME'>" + String(timeInLetters(values[1].toInt())) + " </td>";
                         Serial.println("Values Sizes");
                         Serial.print(values.size()); 
                         html += "<td data-label='DAY-TIME'>" + String(timeInLetters(values[2].toInt())) + " </td>";
                         html += "<td data-label='Motor-KW'>" + String(values[1].toInt() * 0.92/60 ) + " </td>";

  }
 }

  html += "</tr>";
  html += "</tbody>\n";
  html += "</table>\n";

  html += "</body>\n</html>";



  server.send(200, "text/html", html);
}



void handleCommand() {
  // receive command and handle accordingly
  // Serial.println("D310 handleCommand");
  for (uint8_t i = 0; i < server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print(": ");
    Serial.println(server.arg(i));
  }

  if (server.argName(0) == "cmd" && server.arg(0) == "reset") {            // the parameter which was sent to this server
    Serial.println("D318 reset");
    Ttime = 0;
    Mtime = 0;
  }
  else if (server.argName(0) == "cmdd" && server.arg(0) == "resetd") {            // the parameter which was sent to this server
    Serial.println("D275 reset");
    Ttime = 0; // reset all today's runtime counter
          // reset separate today's runtime counter
    }

  else if (server.argName(0) == "cmdl" && server.arg(0) == "moto") {            // the parameter which was sent to this server
    Serial.println("D285 Motor On");
    //digitalWrite(test, HIGH);      // run motor //TODO
    }

  else if (server.argName(0) == "cmdr" && server.arg(0) == "rest") {            // the parameter which was sent to this server
    Serial.println("D295 Restart");
    restr = true;      // Restart
    }

  else if (server.argName(0) == "cmdf" && server.arg(0) == "files") {            // the parameter which was sent to this server
    Serial.println("D288 Files Menu");
    SPIFFS_dir();
    }

  else if (server.argName(0) == "cmdw" && server.arg(0) == "data") {            // the parameter which was sent to this server
    Serial.println("D289 Daily Report");
    handleData();
    }

  else if (server.argName(0) == "cmds" && server.arg(0) == "save") {            // the parameter which was sent to this server
    Serial.println("D298 Daily Report");
    saveDataToFile();
    }

  else if (server.arg(0) == "resbutton" && server.argName(1) == "v") {            // a checkbox sends a value
    byte i = server.arg(0).substring(2).toInt();      // we get a "cb2"
    Serial.print("D334 cb i:"); Serial.println(i);
    if (!MOverride)
        {                      // we get a string not an integer
        //pinMode(b4, INPUT);
        MOverride = true;
        Serial.print("D355 separate.deactivate ");
        }
      else
        {
        //pinMode(b4 , INPUT_PULLUP);
        MOverride = false;
        Serial.print("D355 separate.activate(); ");
        }


  }


  server.send(204);                      // this page doesn't send back content --> 204
}

void handleJs() {
  // Output: JavaScript

  String message;
  message += F("function firecb(c){\n"                                 // c caller (mostly the id/self)
               " var request=new XMLHttpRequest();\n"
               " var v=0;\n"                                           // if value is initialisied with 0 we get ...
               " if (document.getElementById(c).checked) v=1;\n"       // ... 1 or 0 if checkbox is checked (better than true or false)
               " request.open('GET','cmd?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmdd?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmdl?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmdr?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmdf?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmdw?cb='+c+'&v='+v,true);\n"
               " request.open('GET','cmds?cb='+c+'&v='+v,true);\n"
               " request.send(null);\n"
               "}\n");

  server.send(200, "text/javascript", message);
}

void handle204() {
  server.send(204);                     // this page doesn't send back content --> 204
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void checkRuntimeReset() {
  int currentDay = timeinfo.tm_mday;
  int currentMonth = timeinfo.tm_mon + 1;  // Month is 0-based, so adding 1 to get the actual month
  int currentYear = timeinfo.tm_year + 1900;  // Year is years since 1900, so adding 1900 to get the actual year

  static int previousDay = -1; // remember the last day of reset;
  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && previousDay != timeinfo.tm_mday) {
    previousDay = timeinfo.tm_mday;
    Serial.println("D378 Daily reset of runtime");
    //for (auto &i : indicator) i.resetToday();

    //separate.resetToday(); // to be checked if needed
    notfit(6);
    Ttime = 0;
  }
}

//=============New Web Pages  ==========
void handleSPIFFSPage() {

#define ServerVersion "1.0"
String  webpage = "";
bool    SPIFFS_present = false;

#include "FS.h"
#include <SPI.h>

}

// All supporting functions from here...

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) DownloadFile(server.arg(0));
  }
  else SelectInput("Enter filename to download","download","download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DownloadFile(String filename){
  if (SPIFFS_present) {
    File download = SPIFFS.open("/"+filename,  "r");
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download");
  } else ReportSPIFFSNotPresent();
}

void File_Upload(){
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>");
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
  webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html",webpage);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File UploadFile;
void handleFileUpload(){ // upload a new file to the Filing system
  HTTPUpload& uploadfile = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SPIFFS.remove(filename);                  // Remove a previous version, otherwise data is appended the file again
    UploadFile = SPIFFS.open(filename, "w");  // Open the file for writing in SPIFFS (create it, if doesn't exist)
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  }
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          // If the file was successfully created
    {
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>");
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>";
      append_page_footer();
      server.send(200,"text/html",webpage);
    }
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SPIFFS_dir(){
  if (SPIFFS_present) {
    File root = SPIFFS.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<h3 class='rcorners_m'>ESP32 Directory Contents</h3><br>");
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   // Stop is needed because no content length was sent
  } else ReportSPIFFSNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printDirectory(const char * dirname, uint8_t levels){
  File root = SPIFFS.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td></tr>";
    }
    file = root.openNextFile();
  }
  file.close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Stream(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("stream")) SPIFFS_file_stream(server.arg(0));
  }
  else SelectInput("Enter a File to Stream","stream","stream");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SPIFFS_file_stream(String filename) {
  if (SPIFFS_present) {
    File dataFile = SPIFFS.open("/"+filename,  "r"); // Now read data from SPIFFS Card
    if (dataFile) {
      if (dataFile.available()) { // If data is available and present
        String dataType = "application/octet-stream";
        if (server.streamFile(dataFile, dataType) != dataFile.size()) {Serial.print(F("Sent less data than expected!")); }
      }
      dataFile.close(); // close the file:
    } else ReportFileNotPresent("Cstream");
  } else ReportSPIFFSNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("delete")) SPIFFS_file_delete(server.arg(0));
  }
  else SelectInput("Select a File to Delete","delete","delete");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SPIFFS_file_delete(String filename) { // Delete the file
  if (SPIFFS_present) {
    SendHTML_Header();
    File dataFile = SPIFFS.open("/"+filename, "r"); // Now read data from SPIFFS Card
    if (dataFile)
    {
      if (SPIFFS.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>";
        webpage += F("<a href='/delete'>[Back]</a><br><br>");
      }
      else
      {
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='delete'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSPIFFSNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>";
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportSPIFFSNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SPIFFS Card present</h3>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportCouldNotCreateFile(String target){
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

//===============End of new web pages


//======== Contents of CSS.h====================
void append_page_header() {
  webpage  = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>File Server</title>"); // NOTE: 1em = 16px
  webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
  webpage += F("<style>");
  webpage += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#F7F2Fd;}");
  webpage += F("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#dcade6;font-size:1em;}");
  webpage += F("li{float:left;border-radius:0.375em;border-right:0.06em solid #bbb;}last-child {border-right:none;font-size:85%}");
  webpage += F("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%}");
  webpage += F("li a:hover{background-color:#EAE3EA;border-radius:0.375em;font-size:85%}");
  webpage += F("section {font-size:0.88em;}");
  webpage += F("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#558ED5;}");
  webpage += F("h2{color:orange;font-size:1.0em;}");
  webpage += F("h3{font-size:0.8em;}");
  webpage += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}");
  webpage += F("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}");
  webpage += F("tr:nth-child(odd) {background-color:#eeeeee;}");
  webpage += F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}");
  webpage += F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}");
  webpage += F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}");
  webpage += F(".column{float:left;width:50%;height:45%;}");
  webpage += F(".row:after{content:'';display:table;clear:both;}");
  webpage += F("*{box-sizing:border-box;}");
  webpage += F("footer{background-color:#eedfff; text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
  webpage += F("button{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:130%;}");
  webpage += F(".buttons {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:80%;}");
  webpage += F(".buttonsm{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
  webpage += F(".buttonm {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:70%;}");
  webpage += F(".buttonw {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:40%;color:white;font-size:70%;}");
  webpage += F("a{font-size:75%;}");
  webpage += F("p{font-size:75%;}");
  webpage += F("</style></head><body><h1>File Server "); webpage += String(ServerVersion) + "</h1>";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  webpage += F("<ul>");
  webpage += F("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
  webpage += F("<li><a href='/download'>Download</a></li>");
  webpage += F("<li><a href='/upload'>Upload</a></li>");
  webpage += F("<li><a href='/stream'>Stream</a></li>");
  webpage += F("<li><a href='/delete'>Delete</a></li>");
  webpage += F("<li><a href='/dir'>Directory</a></li>");
  webpage += F("</ul>");
  webpage += "<footer>&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
  webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
  webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x70>>1))+"</footer>";
  webpage += F("</body></html>");
}
//======== End of CSS.h====================

void api() {
  Serial.print("NEW REQ!!");
  String data = server.argName(0);

  int time = Ttime;
  String ts = timeInLetters(time);
  String tsM = timeInLetters(Mtime);
  bool stair = false;
  if (digitalRead(Stair_PIN) == HIGH){stair = true;}

  if (data == "status"){
    //return json with current time
    String json = "{\"OVERRIDEmotor\":" + String(MOverride ? "true" : "false") + ",\"motor\":" +
              String(ledOn ? "true" : "false") + ",\"stairs\":" + String(stair ? "true" : "false") +
              ",\"time\":\"" + ts + "\",\"Mtime\":\"" + tsM + "\",\"Son\":" + String(Stair_Hour_ON) +
              ",\"Soff\":" + String(Stair_Hour_OFF) + ",\"Spin\":" + String(Stair_PIN) + "}";
    server.send(200, "application/json", json);
  }

}


void showTime() {
  //Serial.print("NEW REQ!!");
  // less than 60 use secs
  // more than 60 use mins
  // more than 3600 use hours
  int time = Ttime;

  String ts = timeInLetters(time);
  String tsM = timeInLetters(Mtime);

  //Serial.println(ts);
  //Serial.println(t);
  String color = "background-color: red; color: black;";
  if (ledOn){color = "background-color: green; color: white;";}

  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>Motor Timer</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<meta http-equiv='refresh' content='1'>\n"
                "<link rel='icon' href='data:,'>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif;font-size:40px;font-size: 50px;font-weight: bold;font-family: monospace;}.red{backgroud-color:red;}.green{background-color:green;}"
                "</style>\n"
                "</head>\n"
                "<body ";

  html += "style='" + color + "'>\n";
  html += "<p>Total Time Today: ";
  html += ts;
  html += "</p>\n";
  html += "<p>Total Time In Secs: ";
  html += Ttime;
  html += "s</p>\n";

  html += "<p>Time this month: ";
  html += tsM;
  html += "</p>\n";

  html += "</body>\n";

  server.send(200, "text/html", html);
}


void updateESP() {
  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>ESP OTA</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<link rel='icon' href='data:,'>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif;font-size:40px;font-size: 50px;font-weight: bold;font-family: monospace; background-color:red;}"
                "</style>\n"
                "</head>\n"
                "<body><form action='/updateURL' method='post'><label for='url'>OTA File URL:</label><input type='url' id='url' name='url' required='true'><br><button type='submit'>UPDATE</button></form></body>";

  server.send(200, "text/html", html);
}

void timer() {
  //Serial.print("NEW REQ!!");

  String color = "background-color: red; color: black;";
  String clr = "red";
  if (OVERRIDES){clr = "blue";}
  if (digitalRead(Stair_PIN) == HIGH){color = "background-color: green; color: white;";}

  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>Stair Timer</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<meta http-equiv='refresh' content='1'>\n"
                "<link rel='icon' href='data:,'>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif;font-size: 30px;font-weight: bold;font-family: monospace;}.red{backgroud-color:red;}.green{background-color:green;}"
                "</style>\n"
                "</head>\n"
                "<body ";

  html += "style='" + color + "'>\n";
  if (message != "") {html += "<h1>" + message + "</h1>";}
  html += "<p>On Hour: ";
  html += String(Stair_Hour_ON);
  html += ":";
  html += String(Stair_min_ON);
  html += "</p>\n";
  html += "<p>Off Hour: ";
  html += String(Stair_Hour_OFF);
  html += ":";
  html += String(Stair_min_OFF);
  html += "</p>\n";
  html += "<p>Control PIN ";
  html += Stair_PIN;
  html += "</p>\n";
  html += "<p>TIME: " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
  html += "</p>\n";
  html += "<button style='margin-top: 10px;padding: 8px 15px;background-color: #4CAF50;color: white;border: none;border-radius: 4px;cursor: pointer;font-size: 16px;' onclick = \"window.location.href = 'Setup_timer';\" >Setup</button>";
  html += "<br><br>";
  html += "<button style='margin-top: 10px;padding: 8px 15px;background-color: " + clr + ";color: black;border: none;border-radius: 4px;cursor: pointer;font-size: 16px;border: 3px solid black;' onclick = \"window.location.href = 'OVERRIDE_STAIR';\" >OVERRIDE OFF</button>";
  html += "<br><br>";
  html += "<a href='/'>Home</a>";
  html += "</body>\n";

  message = "";
  server.send(200, "text/html", html);
}

void Setup_timer() {
  //Serial.print("NEW REQ!!");

  String color = "background-color: red; color: black;";
  if (digitalRead(Stair_PIN) == HIGH){color = "background-color: green; color: white;";}
//background: linear-gradient(90deg, #efd5ff 0%, #515ada 100%);
  String html = "<!doctype html>\n"
                "<html lang='en'>\n"
                "<head>\n"
                "<title>Stair Timer</title>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<link rel='icon' href='data:,'>\n"
                "<style>\n"
                "body{margin:0;padding:20px;font-family:sans-serif;font-size: 30px;font-weight: bold;font-family: monospace;}.red{backgroud-color:red;}.green{background-color:green;}"
                "</style>\n"
                "</head>\n"
                "<body ";

  html += "style='" + color + "'>\n";
  html += "<label for='time'>Select On Hour:</label><input type='time' id='time' onblur='convertTime()'><p id='result'></p>";
  html += "<br><br>";
  html += "<label for='time2'>Select Off Hour:</label><input type='time' id='time2' onblur='convertTime2()'><p id='result2'></p>";
  html += "<br><br>";
  html += "<label for='PIN'>Control PIN:</label><select id='PIN'><option value='33'>33</option><option value='34'>34</option><option value='35'>35</option></select>";
  html += "<br><br>";
  html += "<label for='time3'>Select Checkpoint:</label><input type='time' id='time3' onblur='convertTime3()'><p id='result3'></p>";
  html += "<br><br>";
  html += "<p>TIME: " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
  html += "</p>\n";
  html += "<br><br>";
  html += "<button style='margin-top: 10px;padding: 8px 15px;background-color: #4CAF50;color: white;border: none;border-radius: 4px;cursor: pointer;font-size: 16px;' onclick='save()'>Save</button>";
  html += "<script>";
  html += "function convertTime() {const selectedTime = document.getElementById('time').value; const [hours, minutes] = selectedTime.split(':'); let convertedHour = parseInt(hours, 10); const selectedAmPm = selectedTime.slice(-2).toUpperCase(); if (selectedAmPm === 'PM' && convertedHour !== 12) {convertedHour += 12;} else if (selectedAmPm === 'AM' && convertedHour === 12) {convertedHour = 0;} const convertedTime = String(convertedHour).padStart(2, '0'); const convertedmin = String(minutes).padStart(2, '0'); document.getElementById('result').innerText = convertedTime +':'+ convertedmin; document.getElementById('time').style.display = 'none'; document.getElementById('result').style.display = 'block';}";
  html += "function convertTime2() {const selectedTime = document.getElementById('time2').value; const [hours, minutes] = selectedTime.split(':'); let convertedHour = parseInt(hours, 10); const selectedAmPm = selectedTime.slice(-2).toUpperCase(); if (selectedAmPm === 'PM' && convertedHour !== 12) {convertedHour += 12;} else if (selectedAmPm === 'AM' && convertedHour === 12) {convertedHour = 0;} const convertedTime = String(convertedHour).padStart(2, '0'); const convertedmin = String(minutes).padStart(2, '0'); document.getElementById('result2').innerText = convertedTime +':'+ convertedmin; document.getElementById('time2').style.display = 'none'; document.getElementById('result2').style.display = 'block';}";
  html += "function convertTime3() {const selectedTime = document.getElementById('time3').value; const [hours, minutes] = selectedTime.split(':'); let convertedHour = parseInt(hours, 10); const selectedAmPm = selectedTime.slice(-2).toUpperCase(); if (selectedAmPm === 'PM' && convertedHour !== 12) {convertedHour += 12;} else if (selectedAmPm === 'AM' && convertedHour === 12) {convertedHour = 0;} const convertedTime = String(convertedHour).padStart(2, '0'); document.getElementById('result3').innerText = convertedTime + ':' + minutes; document.getElementById('time3').style.display = 'none'; document.getElementById('result3').style.display = 'block';}";
  html += "function save() {let [h1, h3] = document.getElementById('result').innerText.split(':'); let [h2, h4] = document.getElementById('result2').innerText.split(':'); let h5 = document.getElementById('result3').innerText; let PIN = document.getElementById('PIN').value; let url = '/Stimer?' + 'h1=' + h1 + '&h2=' + h2 + '&h3=' + h3 + '&h4=' + h4 +  '&h5=' + h5 + '&PIN=' + PIN; if (h1=='0' | h2=='0' | h3=='0') {alert('Please Select time');} else {window.location.href = window.location.origin + url;}}";
  html += "</script>";
  html += "</body>\n";

  server.send(200, "text/html", html);
}

void Stimer() {

  Stair_Hour_ON = atoi(server.arg(0).c_str());
  Stair_Hour_OFF = atoi(server.arg(1).c_str());
  Stair_min_ON = atoi(server.arg(2).c_str());
  Stair_min_OFF = atoi(server.arg(3).c_str());
  String inputString = server.arg(4).c_str();
  int indz = inputString.indexOf(':');
  CHECKPOINT_time = inputString.substring(0, indz).toInt();
  CHECKPOINT_min = inputString.substring(indz + 1).toInt();

  Stair_PIN = atoi(server.arg(5).c_str());
  // Alarm.alarmRepeat(Stair_Hour_OFF,Stair_min_OFF,0,MorningAlarm);  // 9:00am every day
  // Alarm.alarmRepeat(Stair_Hour_ON,Stair_min_ON,0,EveningAlarm);  // 19:00 -> 7:00pm every day
  saveDataToFile();
  saveCHECKPOINT(1);

  message = "SAVED SUCCESFULLY";
  server.sendHeader("Location", "/timer");
  server.send(303);

}

void  updateESPURL() {
  const char *urll = server.arg("url").c_str();
  Serial.print("URLLLLL:");
  Serial.print(urll);
  espOTA(urll);
}

void OVERST() {
  if (OVERRIDES) {OVERRIDES = false;}
  else {OVERRIDES = true;}

  if (digitalRead(Stair_PIN) == HIGH){digitalWrite(Stair_PIN, LOW); digitalWrite(r2, LOW);}
  else{digitalWrite(Stair_PIN, HIGH); digitalWrite(r2, HIGH);}

  message = "OVERRIDE SET";
  server.sendHeader("Location", "/timer");
  server.send(303);
}

void notfit(int m) {
  int time = Ttime;
  String tt = String(timeInLetters(time)); //.replace(" ", "%20");
  String msg = "Starting up.......";
  if (m == 1){msg = "Motor%20Running%20more%20than%2030mins,%0A%0ATotal%20Runtime%20Today:%20%20" + tt;}
  if (m == 2){msg = "Motor%20Running%20more%20than%2040mins,%0A%0ATotal%20Runtime%20Today:%20%20" + tt;}
  if (m == 3){msg = "Motor%20Running%20more%20than%201%20Hour,%0A%0ATotal%20Runtime%20Today:%20%20" + tt;}
  if (m == 4){msg = "Stairs LED OFF";}
  if (m == 5){msg = "Stairs LED ON";}
  if (m == 6){msg = "Daily%20Report%0A%0ADate:%20" + String(timeinfo.tm_mday - 1) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900) + "%0ATotal%20Run%20Time:%20" + tt;}
  if (m == 7){msg = "Saved%20Checkpoint%0A%0ADate:%20" + String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900) + "%0ATime%20Saved:%20" + tt;}
  if (m == 8){msg = "Loaded%20Time:%20" + tt;}

  String url = "http://tqwee.pythonanywhere.com/msg?msgt=";
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url + msg);
  int code = http.GET();
  Serial.println(code);

}

void espOTA(const char *url)
{
  Serial.println("Starting download to flash ...");
  Serial.println(url);
  ota.setCACert(root_ca);
  int const ota_download = ota.download(url);
  if (ota_download <= 0)
  {
    Serial.print  ("Arduino_ESP_OTA::download failed with error code ");
    Serial.println(ota_download);
    message = "Arduino_ESP_OTA::download failed with error code";
    server.send(303, "text/text", message);
    return;
  }
  Serial.print  (ota_download);
  Serial.println(" bytes stored.");


  Serial.println("Verify update integrity and apply ...");
  if ((ota_err = ota.update()) != Arduino_ESP32_OTA::Error::None)
  {
    Serial.print  ("ota.update() failed with error code ");
    Serial.println((int)ota_err);
    message = "ota.update() failed with error code " + String((int)ota_err);
    server.send(303, "text/text", message);
    return;
  }

  Serial.println("Performing a reset after which the bootloader will start the new firmware.");
  server.send(200, "text/text", "SUCCESS");
  delay(1000); /* Make sure the serial message gets out before the reset. */
  ota.reset();
}

void setup() {
  int value = 0;
  pinMode(test, OUTPUT);
  digitalWrite(test,LOW);
  Serial.begin(115200);
  // rtc.setTime(00, 00, 00, 8, 5, 2040);  // 1th Jan 2040 01:00:00
  while (!Serial);        // wait for the serial monitor to open
  pinMode(Stair_PIN, OUTPUT);
  pinMode(commonStatePin, OUTPUT);
  pinMode(b4, INPUT_PULLUP);
  pinMode(32, INPUT_PULLUP);
  pinMode(r4, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(5, OUTPUT);


  connectToWiFi();
  //notfit();
  while (WiFi.status() != WL_CONNECTED) {
    for (int x=0; x< nLEDS; x++){
        digitalWrite(LEDPINS[x], HIGH);
        delay(100);
        digitalWrite(LEDPINS[x], LOW);
    }
    Serial.print(".");
  }


  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, ntpServer, gmtOffsetInSeconds, daylightOffsetInSeconds);

  // Set ESP32 internal RTC from NTP
  timeClient.begin();
  timeClient.update();

  // Wait for NTP response
  while (!timeClient.update()) {
    Serial.println("Waiting for NTP response...");
    for (int x=0; x< nLEDS; x++){
        digitalWrite(LEDPINS[x], HIGH);
        delay(100);
        digitalWrite(LEDPINS[x], LOW);
    }
  }

  // Get the epoch time from NTP
  unsigned long epochTime = timeClient.getEpochTime();

  
  // Set the ESP32's internal RTC 
  struct timeval tv;
  tv.tv_sec = epochTime;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  // Set the time in TimeLib library
  //setTime(epochTime);

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time from internal RTC");
    return;
  }

// Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }



  //for (auto &i : indicator) i.begin();
  //separate.begin();

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisation failed...");
    SPIFFS_present = false;
  }
  else
  {
    Serial.println(F("SPIFFS initialised... file access enabled..."));
    SPIFFS_present = true;
  }


  // the webserver
  // Set up the server routes
  server.on("/", handleRoot);
  server.on("/api", api);
  server.on("/moto", showTime);
  server.on("/timer", timer);
  server.on("/Setup_timer", Setup_timer);
  server.on("/Stimer", Stimer);
  server.on("/OVERRIDE_STAIR", OVERST);
  server.on("/favicon.ico", handle204);
  server.on("/j.js",  handleJs);                 // javscript
  server.on("/cmd", handleCommand);              // process commands
  server.on("/cmdd", handleCommand);              // process commands
  server.on("/cmdl", handleCommand);              // process commands
  server.on("/cmdr", handleCommand);              // process commands
  server.on("/cmdf", handleCommand);              // process commands
  server.on("/cmdw", handleCommand);              // process commands
  server.on("/cmds", handleCommand);              // process commands
  server.onNotFound(handleNotFound);

  // server.on("/spiffs", HTTP_GET, [](){ server.sendHeader("Connection", "close"); handleSPIFFSPage(); });
  ///////////////////////////// Server Request commands
  server.on("/download", File_Download);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
  server.on("/stream",   File_Stream);
  server.on("/delete",   File_Delete);
  server.on("/dir",      SPIFFS_dir);
  server.on("/update",   updateESP);
  server.on("/updateURL",  HTTP_POST, updateESPURL);
    ///////////////////////////// End of Request commands

  server.begin();

  Serial.println("HTTP server started");


  Serial.print("Current time From Internal RTC: ");
  Serial.print(timeinfo.tm_year + 1900);
  Serial.print("-");
  Serial.print(timeinfo.tm_mon + 1);
  Serial.print("-");
  Serial.print(timeinfo.tm_mday);
  Serial.print(" ");
  Serial.print(timeinfo.tm_hour);
  Serial.print(":");
  Serial.print(timeinfo.tm_min);
  Serial.print(":");
  Serial.print(timeinfo.tm_sec);
  Serial.println();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Print available space
  Serial.println("Total space: " + String(SPIFFS.totalBytes()));
  Serial.println("Used space: " + String(SPIFFS.usedBytes()));


// Call lastcells() to get the last 4 values
  listFiles();
  std::vector<String> lastValues = lastcells();

  if (!lastValues.empty()) {
    Mtime = lastValues[0].toInt();
    Stair_Hour_ON = std::min(lastValues[2].toInt(), lastValues[3].toInt());
    Stair_Hour_OFF = std::max(lastValues[2].toInt(), lastValues[3].toInt());
    Stair_min_ON = lastValues[4].toInt();
    Stair_min_OFF = lastValues[5].toInt();
    Stair_PIN = lastValues[6].toInt();
  } else {
    Serial.println("No last values available.");
  }

  loadCHECKPOINT();

  Serial.println("Initializing OTA storage");
  if ((ota_err = ota.begin()) != Arduino_ESP32_OTA::Error::None)
  {
    Serial.print("Arduino_ESP_OTA::begin() failed with error code ");
    Serial.println((int)ota_err);
    return;
  }

  for (int x=0; x< nLEDS; x++){
        digitalWrite(LEDPINS[x], LOW);
  }

  digitalWrite(5, HIGH);
  digitalWrite(r3, LOW);
  digitalWrite(r2, LOW);
  notfit(0);


}


void loop() {
// update time every one sec


  if (millis() % 1000 == 0){
    if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time from internal RTC");
    return;
  }



  if ((timeinfo.tm_hour == Stair_Hour_ON && timeinfo.tm_min == Stair_min_ON && timeinfo.tm_sec == 1) && !OVERRIDES) {
      if (digitalRead(Stair_PIN) == LOW){
        digitalWrite(Stair_PIN, HIGH);
        digitalWrite(r2, HIGH);
        notfit(5);
      }
  }
  else if ((timeinfo.tm_hour == Stair_Hour_OFF && timeinfo.tm_min == Stair_min_OFF && timeinfo.tm_sec == 1) && !OVERRIDES){
      if (digitalRead(Stair_PIN) == HIGH){
        digitalWrite(Stair_PIN, LOW);
        digitalWrite(r2, LOW);
        notfit(4);
        Serial.println("OFFF");
      }

  }

  if (datasa){datasa=false;}

  if (timeinfo.tm_hour == CHECKPOINT_time && timeinfo.tm_min == CHECKPOINT_min && timeinfo.tm_sec == 2 ) {
    saveCHECKPOINT(1);
  }

  }

  //r4 b4
  if (digitalRead(b4) == 0){
    digitalWrite(r4, HIGH);
  }
  else{
    digitalWrite(r4, LOW);
  }

  if (MOverride == 0){
    digitalWrite(r3, LOW);
  }
  else{
    digitalWrite(r3, HIGH);
  }


  //Main motor run time
  if (digitalRead(r4) == HIGH && !ledOn){
    ledOn = true;
    onTime = millis();
    tempTime = 0;
    notfi = false;
    notfi1 = false;
    notfi2 = false;
  }
  else if (digitalRead(r4) == LOW && ledOn)
  {
    ledOn = false;
    Ttime -= tempTime;
    Mtime -= tempTime;
    tempTime = (millis() - onTime)/1000;
    Ttime += tempTime;
    Mtime += tempTime;
  }
  else if (digitalRead(r4) == HIGH && ledOn)
  {
    Ttime -= tempTime;
    Mtime -= tempTime;
    tempTime = (millis() - onTime)/1000;
    Ttime += tempTime;
    Mtime += tempTime;
    // telegram notfi if more than 30 min on
    if (tempTime >= (30*60) && !notfi){
      Serial.print("Reporting to telegram");
      //web http
      notfit(1);
      notfi = true;
    }
    else if (tempTime >= (40*60) && !notfi1)
    {
      notfit(2);
      notfi1 = true;

    }
    else if (tempTime >= (60*60) && !notfi2)
    {
      notfit(3);
      notfi2 = true;

    }
  }


  if (digitalRead(32) == 0){
     restartWEB();
     Serial.print("Restart WEB");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost connection to WiFi. Reconnecting...");
    SaveState(); //Save Pins State before animation
    connectToWiFiLOOP();
    LoadState();
    restartWEB();
  }

  //separate.update();


  server.handleClient();   // loop to handle incoming client requests



  if (timeinfo.tm_hour == 23 && timeinfo.tm_min == 59 && timeinfo.tm_sec == 1 && !datasa) {
      saveDataToFile();
      datasa=true;

  }

  checkRuntimeReset();


  if ( restr == true ) restart() ;

}