#include "E_INK.h"
#include <Adafruit_GFX.h>
//#include "Fonts/FreeSansOblique18pt7b.h"
#include "Neucha_Regular17pt7b.h"
//#include "Homenaje_Regular17pt7b.h"
#include "icons.h"
#include "touch.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//.ttf to .h: https://rop.nl/truetype2gfx/
//Open Source fonts https://fonts.google.com/
#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL NULL

eink display;
extern Adafruit_MCP23017 mcp;

#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

WiFiUDP udp;
WiFiClient client;
HTTPClient http;
Adafruit_BME280 bme;
struct timeval tm;
uint64_t timeToWake;

const char ssid[] = "Biro_WLAN";
const char pass[] = "CaVex250_H2sH11";
//const char ssid[] = "Golic_wlan";
//const char pass[] = "BaDromAnce12589/*-";
//const char ssid[] = "e-radionica.com";
//const char pass[] = "croduino";
//const char ssid[] = "AndroidAP12589";
//const char pass[] = "sgs7EDGE";
const char DOW[7][4] = {"NED", "PON", "UTO", "SRI", "CET", "PET", "SUB"};
//const char DOW[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char* oznakeVjetar[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
typedef struct vremenskaProg {
  float temp[8], tempMax[8], tempMin[8], pressure[8], pressureSea[8], pressureGnd[8], windSpeed[8], windDir[8], humidity[8];
  int id[8];
  char description[8][20], icon[8][4];
  long epoch[8];
  int cond[8];
  int nData;
};
vremenskaProg prognozaDani[6];

typedef struct weatherDay {
  float tempMax, tempMin, pressure, windSpeed, windDir, windSpeedMax, humidity;
  int cond;
};

int timeOffset;
weatherDay dani[6];

//const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 310;
StaticJsonDocument<2000> doc;
const size_t capacity2 = 40 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(40) + 85 * JSON_OBJECT_SIZE(1) + 41 * JSON_OBJECT_SIZE(2) + 40 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 35 * JSON_OBJECT_SIZE(7) + 45 * JSON_OBJECT_SIZE(8) + 8740;
const char* NTPServer = "hr.pool.ntp.org";
uint16_t ntpPort = 123;
IPAddress ntpIp;
uint8_t ntpPacket[48];
char opis[70];
int icon = 1;
float temperature, bmeTemp, bmeHum, bmePress, windSpeed;
int hum, pres, windDir;
const char* desc;
uint8_t sec, minu, hr;
uint32_t sunrise, sunset;
uint8_t noInternet = 0;
uint8_t modeSelect = 0;
uint8_t forcedRef = 0;

struct ntpInfo {
  uint8_t h;
  uint8_t m;
  uint8_t s;
  uint8_t d;
  uint8_t mo;
  uint16_t y;
  uint8_t dow;
} td;

unsigned long time2, time3;
void setup() {
  display.begin();
  display.clearDisplay();
  int n;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setHostname("WeatherPaper");
  Wire.begin();
  Serial.begin(115200);
  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
  initTouch();
  display.setRotation(2);
  display.setFont(DISPLAY_FONT);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 24);
  n = WiFi.scanNetworks();
  if (n > 6) n = 6;
  display.clearDisplay();
  display.setCursor(0, 24);
  display.print(F("Pronadjeno:"));
  for (int i = 0; i < n; i++) {
    display.setCursor(70, 70 + (i * 35));
    display.print(WiFi.SSID(i));
    display.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? 'O' : '*');
    display.print(' ');
    display.print(WiFi.RSSI(i), DEC);
  }
  display.einkOn();
  display.display();
  display.setCursor(70, 550);
  display.print(F("Spajanje na "));
  display.print(ssid);
  display.print("...");
  display.display();
  display.einkOff();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  display.einkOn();
  display.print("spojeno!");
  display.display();
  display.einkOff();
  udp.begin(8888);
  if (!WiFi.hostByName(NTPServer, ntpIp)) {
    noInternet = 1;
  }
  readSensor();
  readWeather();
  readNTP();
  refresh();
  esp_wifi_stop();
  btStop();
}
unsigned long time1 = 0;
void loop() {
  touchSleep();
  gettimeofday(&tm, NULL);
  esp_sleep_enable_timer_wakeup((timeToWake - tm.tv_sec) * 1000000ULL); //45 minuta
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
  esp_light_sleep_start();
  if (touchAvailable()) {
    int ts[2];
    if (readTouch(ts)) {
      //Serial.print('?');
      //Serial.print(ts[0], DEC);
      //Serial.print(' ');
      //Serial.println(ts[1], DEC);
      ts[0] = map(ts[0], 2767, 591, 0, 800);
      ts[1] = map(ts[1], 2559, 384, 0, 600);
      switch (modeSelect) {
        case 0:
          if (ts[1] > 410 && ts[0] > 0 && ts[0] < 800) {
            drawDays(ts[0] / 160);
            modeSelect = 1;
          }

          if(ts[0]>25 && ts[0] < 75 && ts[1] > 75 && ts[1] < 125) {
            drawDays(0);
            modeSelect = 1;
          }
          
          if (ts[1] > 0 && ts[1] < 30 && ts[0] > 770 && ts[0] < 800) {
            display.setFont(DISPLAY_FONT);
            display.setCursor(610, 35);
            display.print("Pricekajte...");
            WAKEUP_SET;
            VCOM_SET;
            display.einkOn();
            display.display();
            display.einkOff();
            VCOM_CLEAR;
            WAKEUP_CLEAR;
            forcedRef = 1;
          }
          break;

        case 1:
          if (ts[1] > 500 && ts[0] > 700 && ts[0] < 800) {
            refresh();
            modeSelect = 0;
          }
          break;

      }
      //Serial.print(ts[0], DEC);
      //Serial.print(' ');
      //Serial.println(ts[1], DEC);

    }
  }
  //if ((unsigned long)(millis() - time1) > 300000) {
  //  time1 = millis();
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER || forcedRef) {
    forcedRef = 0;
    esp_wifi_start();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      i++;
      if (i > 15) break;
    }

    if (i < 15) {
      readSensor();
      readWeather();
      readNTP();
      refresh();
    }
    esp_wifi_stop();
    btStop();
  }
}

void readSensor() {
  bme.takeForcedMeasurement();
  bmeTemp = bme.readTemperature();
  bmeHum = bme.readHumidity();
  bmePress = bme.readPressure() / 100.0F;
}
void readNTP() {
  memset(ntpPacket, 0, 48);
  ntpPacket[0] = B11100011; //Clock is unsync, NTP version 4, Symmetric passive
  ntpPacket[1] = 0;     // Stratum, or type of clock
  ntpPacket[2] = 60;     // Polling Interval
  ntpPacket[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpPacket[12]  = 49;
  ntpPacket[13]  = 0x4E;
  ntpPacket[14]  = 49;
  ntpPacket[15]  = 52;
  udp.beginPacket(ntpIp, 123);
  udp.write(ntpPacket, 48);
  udp.endPacket();
  delay(1500);
  if (udp.parsePacket()) {
    udp.read(ntpPacket, 48);
    uint32_t unix = ntpPacket[40] << 24 | ntpPacket[41] << 16 | ntpPacket[42] << 8 | ntpPacket[43];
    unix = unix - 2208988800UL + 7200;
    //sec = unix % 60;
    //minu = unix / 60 % 60;
    //hr = unix / 3600 % 24;
    td = epochDate(unix);
  }
}
void readWeather() {
  int i;
  typedef struct wforecast {
    float temp, tempMax, tempMin, pressure, pressureSea, pressureGnd, windSpeed, windDir;
    int humidity;
    char description[30], icon[4];
    long epoch;
    int cond;
  };
  wforecast prognoza[40];
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/weather?&lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) {   //Belisce
    //if (http.begin(client, F("http://api.openweathermap.org/data/2.5/weather?&id=3221033&units=metric&lang=en&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) {            //Hannover
    if (http.GET() > 0) {
      String APIData;
      //DynamicJsonDocument doc(capacity);
      JsonObject main, weather_0, sys;
      char wIcon[6];
      APIData = http.getString();
      http.end();
      deserializeJson(doc, APIData);
      sys = doc["sys"];
      main = doc["main"];
      weather_0 = doc["weather"][0];
      strncpy(opis, weather_0["description"], sizeof(opis));
      strncpy(wIcon, weather_0["icon"], sizeof(wIcon));
      icon = atoi(wIcon);
      changeLetters(opis);
      temperature = main["temp"];
      hum = main["humidity"];
      pres = main["pressure"];
      desc = weather_0["description"];
      windSpeed = doc["wind"]["speed"];
      windDir = doc["wind"]["deg"];
      int timezone = doc["timezone"];
      sunrise = sys["sunrise"];
      sunrise += timezone;
      sunset = sys["sunset"];
      sunset += timezone;
    } else {
      noInternet = 1;
      return;
    }
  }


  //Uhvati podatke s APIa s Interneta, te ih smjesti u polje struktura (privremeno)
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/forecast?lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) { //Belisce
    //if (http.begin(client, F("http://api.openweathermap.org/data/2.5/forecast?&id=3221033&units=metric&lang=en&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) { //Hannover
    if (http.GET() > 0) {
      String APIData;
      APIData = http.getString();
      http.end();
      DynamicJsonDocument doc2(capacity2);
      deserializeJson(doc2, APIData);
      JsonArray list = doc2["list"];
      JsonObject listDate = list[0];
      JsonObject list_main = listDate["main"];
      JsonObject list_weather = listDate["weather"][0];
      //JsonObject list_clouds = listDate["clouds"];
      JsonObject list_wind = listDate["wind"];
      for (i = 0; i < 40; i++) {
        listDate = list[i];
        list_main = listDate["main"];
        list_weather = listDate["weather"][0];
        //list_clouds = listDate["clouds"];
        list_wind = listDate["wind"];
        prognoza[i].epoch = listDate["dt"];
        prognoza[i].temp = list_main["temp"];
        prognoza[i].tempMin = list_main["temp_min"];
        prognoza[i].tempMax = list_main["temp_max"];
        prognoza[i].pressure = list_main["pressure"];
        prognoza[i].pressureSea = list_main["sea_level"];
        prognoza[i].pressureGnd = list_main["grnd_level"];
        prognoza[i].humidity = list_main["humidity"];
        prognoza[i].cond = list_weather["id"];
        strncpy(prognoza[i].icon, list_weather["icon"], 3);
        strncpy(prognoza[i].description, list_weather["description"], 19);
        prognoza[i].windSpeed = list_wind["speed"];
        prognoza[i].windDir = list_wind["deg"];
      }
      i = 0;
      while (epochDate(prognoza[i].epoch).h != 0) {
        //epochDate(prognoza[i].epoch);
        i++;
      };
      timeOffset = i;

      //Provjeri da li vremenska prognoza pocinje od pocetka dana.
      if (timeOffset == 0) {      //Ako da, postavi svaki podatak u odgovarajuće mjesto u polju podataka strukture u polji struktura (jedna struktura predstavlja jedan dan, a svaki element polja unutar te strukture razmak od 3 sata).
        for (int i = 0; i < 5; i++) {
          prognozaDani[i].nData = 8;
          for (int j = 0; j < 8; j++) {
            prognozaDani[i].temp[j] = prognoza[i * 8 + j].temp;
            prognozaDani[i].tempMax[j] = prognoza[i * 8 + j].tempMax;
            prognozaDani[i].tempMin[j] = prognoza[i * 8 + j].tempMin;
            prognozaDani[i].pressure[j] = prognoza[i * 8 + j].pressure;
            prognozaDani[i].pressureSea[j] = prognoza[i * 8 + j].pressureSea;
            prognozaDani[i].pressureGnd[j] = prognoza[i * 8 + j].pressureGnd;
            prognozaDani[i].humidity[j] = prognoza[i * 8 + j].humidity;
            prognozaDani[i].windSpeed[j] = prognoza[i * 8 + j].windSpeed;
            prognozaDani[i].windDir[j] = prognoza[i * 8 + j].windDir;
            prognozaDani[i].epoch[j] = prognoza[i * 8 + j].epoch;
            prognozaDani[i].cond[j] = prognoza[i * 8 + j].cond;
            strncpy(prognozaDani[i].icon[j], prognoza[i * 8 + j].icon, 4);
            strncpy(prognozaDani[i].description[j], prognoza[i * 8 + j].description, 20);
          }
        }
      } else {      //Ako nije, tada napravi prvo napravi rasclambu podataka za ostatak trenutnog dana, zatim za dane u sredini (za koje imamo sve podatke) i zatim onaj zadnji na kraju (za kojeg isto tako nemamo sve podatke).
        prognozaDani[0].nData = timeOffset;
        for (int i = 0; i < timeOffset; i++) {
          prognozaDani[0].temp[i] = prognoza[i].temp;
          prognozaDani[0].tempMax[i] = prognoza[i].tempMax;
          prognozaDani[0].tempMin[i] = prognoza[i].tempMin;
          prognozaDani[0].pressure[i] = prognoza[i].pressure;
          prognozaDani[0].pressureSea[i] = prognoza[i].pressureSea;
          prognozaDani[0].pressureGnd[i] = prognoza[i].pressureGnd;
          prognozaDani[0].humidity[i] = prognoza[i].humidity;
          prognozaDani[0].windSpeed[i] = prognoza[i].windSpeed;
          prognozaDani[0].windDir[i] = prognoza[i].windDir;
          prognozaDani[0].epoch[i] = prognoza[i].epoch;
          prognozaDani[0].cond[i] = prognoza[i].cond;
          strncpy(prognozaDani[0].icon[i], prognoza[i].icon, 4);
          strncpy(prognozaDani[0].description[i], prognoza[i].description, 20);
        }

        for (int i = 0; i < 4; i++) {
          prognozaDani[i + 1].nData = 8;
          int z;
          for (int j = 0; j < 8; j++) {
            z = i * 8 + j + timeOffset;
            prognozaDani[i + 1].temp[j] = prognoza[z].temp;
            prognozaDani[i + 1].tempMax[j] = prognoza[z].tempMax;
            prognozaDani[i + 1].tempMin[j] = prognoza[z].tempMin;
            prognozaDani[i + 1].pressure[j] = prognoza[z].pressure;
            prognozaDani[i + 1].pressureSea[j] = prognoza[z].pressureSea;
            prognozaDani[i + 1].pressureGnd[j] = prognoza[z].pressureGnd;
            prognozaDani[i + 1].humidity[j] = prognoza[z].humidity;
            prognozaDani[i + 1].windSpeed[j] = prognoza[z].windSpeed;
            prognozaDani[i + 1].windDir[j] = prognoza[z].windDir;
            prognozaDani[i + 1].epoch[j] = prognoza[z].epoch;
            prognozaDani[i + 1].cond[j] = prognoza[z].cond;
            strncpy(prognozaDani[i + 1].icon[j], prognoza[z].icon, 4);
            strncpy(prognozaDani[i + 1].description[j], prognoza[z].description, 20);
          }
        }

        int k = 0;
        for (int i = 32 + timeOffset; i < 40; i++) {
          prognozaDani[5].temp[k] = prognoza[i].temp;
          prognozaDani[5].tempMax[k] = prognoza[i].tempMax;
          prognozaDani[5].tempMin[k] = prognoza[i].tempMin;
          prognozaDani[5].pressure[k] = prognoza[i].pressure;
          prognozaDani[5].pressureSea[k] = prognoza[i].pressureSea;
          prognozaDani[5].pressureGnd[k] = prognoza[i].pressureGnd;
          prognozaDani[5].humidity[k] = prognoza[i].humidity;
          prognozaDani[5].windSpeed[k] = prognoza[i].windSpeed;
          prognozaDani[5].windDir[k] = prognoza[i].windDir;
          prognozaDani[5].epoch[k] = prognoza[i].epoch;
          prognozaDani[5].cond[k] = prognoza[i].cond;
          strncpy(prognozaDani[5].icon[k], prognoza[i].icon, 3);
          strncpy(prognozaDani[5].description[k], prognoza[i].description, 20);
          k++;
        }
        prognozaDani[5].nData = k;
      }
      //Sada treba pronaci prosjecne temperature, tlakove, vlagu, itd. za pojedinacni dan, kao i najmanje i najvece vrijednosti
      for (int i = 0; i < (timeOffset ? 6 : 5); i++) {
        dani[i].tempMax = prognozaDani[i].tempMax[0];
        dani[i].tempMin = prognozaDani[i].tempMin[0];
        dani[i].windSpeedMax = prognozaDani[i].windSpeed[0];
        dani[i].pressure = 0;
        dani[i].humidity = 0;
        dani[i].windSpeed = 0;
        dani[i].windDir = 0;
        dani[i].cond = 0;
        for (int j = 0; j < prognozaDani[i].nData; j++) {
          if (dani[i].tempMax < prognozaDani[i].tempMax[j]) dani[i].tempMax = prognozaDani[i].tempMax[j];
          if (dani[i].tempMin > prognozaDani[i].tempMin[j]) dani[i].tempMin = prognozaDani[i].tempMin[j];
          if (dani[i].windSpeedMax < prognozaDani[i].windSpeed[j]) dani[i].windSpeedMax = prognozaDani[i].windSpeed[j];
          dani[i].pressure += prognozaDani[i].pressure[j];
          dani[i].humidity += prognozaDani[i].humidity[j];
          dani[i].windSpeed += prognozaDani[i].windSpeed[j];
          dani[i].windDir += prognozaDani[i].windDir[j];
        }
        dani[i].pressure /= prognozaDani[i].nData;
        dani[i].humidity /= prognozaDani[i].nData;
        dani[i].windSpeed /= prognozaDani[i].nData;
        dani[i].windDir /= prognozaDani[i].nData;
        dani[i].cond = prognozaDani[i].cond[4];
      }
    } else {
      noInternet = 1;
      return;
    }
  }
  gettimeofday(&tm, NULL);
  timeToWake = 1800 + tm.tv_sec;
}

void changeLetters(char *p) {
  int webTextSize = strlen(p);
  for (int16_t i = 0; i < webTextSize; i++) {
    if (p[i] == 196 || p[i] == 197) memmove(&p[i], &p[i + 1], webTextSize - i);
    if (p[i] == 141 || p[i] == 135) p[i] = 'c';
    if (p[i] == 161) p[i] = 's';
    if (p[i] == 190) p[i] = 'z';
  }
}

void refresh() {
  char tmp[50];
  display.clearDisplay();
  display.setTextSize(1);
  display.fillRect(20, 360, 760, 3, BLACK);
  display.setFont(DISPLAY_FONT_SMALL);
  display.setCursor(4, 55);
  display.print(opis);
  display.setCursor(5, 26);
  display.print(F("Belisce"));
  display.setCursor(5, 34);
  display.print(F("Powered by openweathermaps.org"));
  display.drawBitmap(770, 0, refIcon, 30, 30, BLACK);

  sprintf(tmp, "%1d:%02d %d/%d/%4d %3s", td.h, td.m, td.d, td.mo, td.y, DOW[td.dow]);
  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 796, 3, BLACK);
  display.setCursor(267, 35);
  display.print(tmp);

  display.drawBitmap(20, 75, weatherIcon(icon), 50, 50, BLACK);
  display.setTextSize(2);
  display.setCursor(80, 120);
  display.print(temperature, 0);
  display.setCursor(display.getCursorX() + 5, 100);
  display.print('o');
  display.setTextSize(1);
  display.drawBitmap(1, 140, iconTlak, 40, 40, BLACK);
  display.setCursor(45, 170);
  display.print(pres, DEC);
  display.drawBitmap(145, 140, iconVlaga, 40, 40, BLACK);
  display.setCursor(195, 170);
  display.print(hum);
  display.drawBitmap(1, 190, iconVjetar, 40, 40, BLACK);
  display.setCursor(45, 220);
  display.print(windSpeed, 1);
  display.print(" m/s | ");
  display.print(oznakeVjetar[int((windDir / 22.5) + .5) % 16]);
  display.drawBitmap(1, 240, iconSunrise, 40, 40 , BLACK);
  sprintf(tmp, "%d:%02d", sunrise / 3600 % 24, sunrise / 60 % 60);
  display.setCursor(40, 270);
  display.print(tmp);
  display.drawBitmap(130, 240, iconSunset, 40, 40 , BLACK);
  sprintf(tmp, "%d:%02d", sunset / 3600 % 24, sunset / 60 % 60);
  display.setCursor(175, 270);
  display.print(tmp);

  display.drawBitmap(300, 70, indoorIcon, 40, 40, BLACK);
  display.setCursor(300, 170);
  display.print(bmePress, 1);
  display.print("hPa");
  display.setCursor(300, 220);
  display.print(bmeHum, 1);
  display.print('%');
  display.setCursor(300, 270);
  display.print(bmeTemp, 1);
  display.print('C');

  int k = (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
  int xOffset, xOffsetText;
  for (int i = 1; i < 6; i++) {
    xOffset = i * 160;
    xOffsetText = ((i - 1) * 160) + 20;
    display.fillRect(xOffset, 410, 3, 180, BLACK);
    display.drawBitmap(xOffset - 105, 420, weatherIcon(atoi(prognozaDani[i - 1 + k].icon[4])), 50, 50, BLACK);
    display.setTextSize(1);
    display.setFont(DISPLAY_FONT_SMALL);
    display.setCursor(xOffsetText + 10, 472);
    changeLetters(prognozaDani[i - 1 + k].description[4]);
    display.print(prognozaDani[i - 1 + k].description[4]);
    display.setTextSize(1);
    display.setCursor(xOffsetText + 40, 405);
    display.print(epochDate(prognozaDani[i - 1 + k].epoch[0]).d);
    display.print('.');
    display.print(epochDate(prognozaDani[i - 1 + k].epoch[0]).mo);
    display.setCursor(xOffsetText - 10, 565);
    //display.print("1003 hPa  57%");
    display.print(dani[i - 1 + k].pressure, 1);
    display.print("hPa  ");
    display.print(dani[i - 1 + k].humidity, 1);
    display.print('%');
    display.setCursor(xOffsetText - 10, 574);
    //display.print("5.6 m/s W");
    display.print(dani[i - 1 + k].windSpeed, 1);
    display.print("m/s ");
    display.print(oznakeVjetar[int((dani[i - 1 + k].windDir / 22.5) + .5) % 16]);
    display.print(' ');
    //display.print("   10.1 m/s W");
    display.print(dani[i - 1 + k].windSpeedMax, 1);
    display.print("m/s");
    display.setFont(DISPLAY_FONT);
    display.setCursor(xOffsetText, 520);
    //display.print("22");
    display.print(dani[i - 1 + k].tempMax, 1);
    display.setCursor(display.getCursorX() + 5, 510);
    display.print("o");
    display.setCursor(xOffsetText, 555);
    //display.print("7");
    display.print(dani[i - 1 + k].tempMin, 1);
    display.setCursor(display.getCursorX() + 5, 545);
    display.print('o');
    display.setCursor(xOffsetText + 20, 400);
    display.print(DOW[epochDate(prognozaDani[i - 1 + k].epoch[0]).dow]);
  }

  for (int i = 1; i < 3; i++) {
    display.fillRect(267 * i, 60, 3, 290, BLACK);
  }

  if (noInternet) {
    noInternet = 0;
    display.fillRect(100, 402, 600, 100, WHITE);
    display.drawRect(99, 401, 602, 102, BLACK);
    display.drawRect(98, 400, 604, 104, BLACK);
    display.setFont(DISPLAY_FONT);
    display.setTextSize(2);
    display.setCursor(170, 470);
    display.print("Nema Interneta");
  }
  WAKEUP_SET;
  VCOM_SET;
  display.einkOn();
  time2 = millis();
  display.display();
  time3 = millis();
  //delay(100);
  display.einkOff();
  VCOM_CLEAR;
  WAKEUP_CLEAR;
  Serial.println("Ref time:");
  Serial.print(time3 - time2, DEC);
}

uint8_t* weatherIcon(uint8_t i) {
  switch (i) {
    case 1:
      return (uint8_t*)icon01d;
      break;
    case 2:
      return (uint8_t*)icon02d;
      break;
    case 3:
      return (uint8_t*)icon03d;
      break;
    case 4:
      return (uint8_t*)icon04d;
      break;
    case 9:
      return (uint8_t*)icon09d;
      break;
    case 10:
      return (uint8_t*)icon10d;
      break;
    case 11:
      return (uint8_t*)icon11d;
      break;
    case 13:
      return (uint8_t*)icon13d;
      break;
    case 50:
      return (uint8_t*)icon13d;
      break;
    default:
      return (uint8_t*)icon01d;
  }
}

ntpInfo epochDate(uint32_t epoch) {
  ntpInfo timeInfo;
  static unsigned char month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  static unsigned char week_days[7] = {4, 5, 6, 0, 1, 2, 3};
  //Thu=4, Fri=5, Sat=6, Sun=0, Mon=1, Tue=2, Wed=3
  unsigned char ntp_hour, ntp_minute, ntp_second, ntp_week_day, ntp_date, ntp_month, leap_days, leap_year_ind ;
  unsigned short temp_days;
  unsigned int ntp_year, days_since_epoch, day_of_year;

  leap_days = 0;
  leap_year_ind = 0;

  ntp_second = epoch % 60;
  epoch /= 60;
  ntp_minute = epoch % 60;
  epoch /= 60;
  ntp_hour  = epoch % 24;
  epoch /= 24;

  days_since_epoch = epoch;      //number of days since epoch
  ntp_week_day = week_days[days_since_epoch % 7]; //Calculating WeekDay

  ntp_year = 1970 + (days_since_epoch / 365); // ball parking year, may not be accurate!

  int i;
  for (i = 1972; i < ntp_year; i += 4) // Calculating number of leap days since epoch/1970
    if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)) leap_days++;

  ntp_year = 1970 + ((days_since_epoch - leap_days) / 365); // Calculating accurate current year by (days_since_epoch - extra leap days)
  day_of_year = ((days_since_epoch - leap_days) % 365) + 1;


  if (((ntp_year % 4 == 0) && (ntp_year % 100 != 0)) || (ntp_year % 400 == 0))
  {
    month_days[1] = 29;   //February = 29 days for leap years
    leap_year_ind = 1;    //if current year is leap, set indicator to 1
  }
  else month_days[1] = 28; //February = 28 days for non-leap years

  temp_days = 0;

  for (ntp_month = 0 ; ntp_month <= 11 ; ntp_month++) //calculating current Month
  {
    if (day_of_year <= temp_days) break;
    temp_days = temp_days + month_days[ntp_month];
  }

  temp_days = temp_days - month_days[ntp_month - 1]; //calculating current Date
  ntp_date = day_of_year - temp_days;

  timeInfo.d = ntp_date;
  timeInfo.mo = ntp_month;
  timeInfo.y = ntp_year;
  timeInfo.dow = ntp_week_day;
  timeInfo.h = ntp_hour;
  timeInfo.m = ntp_minute;
  timeInfo.s = ntp_second;

  return timeInfo;
}

void drawDays(uint8_t n) {
  char dayHours[8];
  n += (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
  display.clearDisplay();
  display.setFont(DISPLAY_FONT_SMALL);
  for (int i = 0; i < prognozaDani[n].nData; i++) {
    dayHours[i] = epochDate(prognozaDani[n].epoch[i]).h;
    display.drawBitmap(i * 80 + 10, 100, weatherIcon(atoi(prognozaDani[n].icon[i])), 50, 50, BLACK);
    display.setCursor(i * 80 + 20, 160);
    display.print(dayHours[i], DEC);
    display.print('h');
  }
  drawGraph(80, 250, dayHours, prognozaDani[n].temp, "h", "C", 250, 130, prognozaDani[n].nData, 1, 7);
  drawGraph(480, 250, dayHours, prognozaDani[n].pressure, "h", "hPa", 250, 130, prognozaDani[n].nData, 1, 7);
  drawGraph(80, 420, dayHours, prognozaDani[n].humidity, "h", "%", 250, 130, prognozaDani[n].nData, 0, 7);
  drawGraph(480, 420, dayHours, prognozaDani[n].windSpeed, "h", "m/s", 250, 130, prognozaDani[n].nData, 1, 7);
  display.fillTriangle(770, 570, 790, 570, 780, 580, BLACK);
  display.setCursor(150, 200);
  display.setFont(DISPLAY_FONT);
  display.setTextSize(1);
  display.print("Odabran je dan ");
  display.print(epochDate(prognozaDani[n].epoch[0]).d, DEC);
  display.print('.');
  display.print(epochDate(prognozaDani[n].epoch[0]).mo, DEC);
  display.print(".,");
  display.print(DOW[epochDate(prognozaDani[n].epoch[0]).dow]);
  display.einkOn();
  display.display();
  display.einkOff();
}
void drawGraph(int x, int y, char *dx, float *dy, char *tx, char *ty, int sizex, int sizey, char n, char decimal, char yelements) {
  char digits = 0;
  float maxValueX = dx[0], minValueX = dx[0];
  float maxValueY = dy[0], minValueY = dy[0];
  int stepX;
  float stepY;
  int stepYGraph;
  for (int i = 0; i < n; i++) {
    //if (maxValueX < dx[i]) maxValueX = dx[i];
    //if (minValueX > dx[i]) minValueX = dx[i];
    if (maxValueY < dy[i]) maxValueY = dy[i];
    if (minValueY > dy[i]) minValueY = dy[i];
  }
  stepY = (maxValueY - minValueY) / (yelements + 1);
  digits = (int) log10(maxValueY) + 1 + (decimal != 0 ? 1 + decimal : 0);
  display.fillRect(x + (digits * 6), y + sizey, sizex, 2, BLACK);
  display.fillRect(x + (digits * 6), y, 2, sizey, BLACK);
  stepX = sizex / n;
  display.setFont(NULL);
  display.setTextSize(1);
  //display.setCursor(x, y);
  //display.print(maxValueY, decimal);
  //display.setCursor(x, y + sizey);
  //display.print(minValueY, decimal);
  //minValueY -= 5;
  //maxValueY += 5;
  int x1, y1, x2, y2;
  for (int i = 0; i < n - 1; i++) {
    x1 = i * stepX + x + (digits * 6);
    x2 = (i + 1) * stepX + x + (digits * 6);
    y1 = map2(dy[i], minValueY, maxValueY, y + sizey, y);
    y2 = map2(dy[i + 1], minValueY, maxValueY, y + sizey, y);
    display.drawLine(x1, y1, x2, y2, BLACK);
    display.fillCircle(x1, y1, 2, BLACK);
  }
  display.fillCircle(x2, y2, 2, BLACK);

  stepYGraph = sizey / (yelements + 1);
  for (int i = 0; i < yelements + 2; i++) {
    for (int j = 0; j < n; j++) {
      if (i == 0) {
        display.setCursor(x + (digits * 6) + (stepX * j), y + sizey + 9);
        display.print(dx[j], DEC);
      }
      x2 = j * stepX + x + (digits * 6);
      display.fillCircle(x2, y + (sizey) - (stepYGraph * i), 1, BLACK);
    }
    display.setCursor(x, y + (sizey) - (stepYGraph * i) - 3);
    display.print(minValueY + (stepY * i), decimal);
  }
  display.setCursor(x + (digits * 6) + (stepX * n + 1), y + sizey + 9);
  display.print(tx);
  display.setCursor(x, y - 10);
  display.print(ty);
}

double map2(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
