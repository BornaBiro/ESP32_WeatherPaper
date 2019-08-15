#include "E_INK.h"
//#include "E_INK_noMCP.h"
#include <Adafruit_GFX.h>
#include "Fonts/FreeSansOblique18pt7b.h"
#include "icons.h"
#include "touch.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

eink display;

#include "WiFi.h"
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

WiFiMulti wifim;
WiFiUDP udp;
WiFiClient client;
HTTPClient http;
Adafruit_BME280 bme;

const char ssid[] = "Biro_WLAN";
const char pass[] = "CaVex250_H2sH11";
//const char ssid[] = "e-radionica.com";
//const char pass[] = "croduino";
//const char ssid[] = "AndroidAP12589";
//const char pass[] = "CaVex250_H2sH11";
const char DOW[7][4] = {"NED", "PON", "UTO", "SRI", "CET", "PET", "SUB"};
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

const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 310;
const size_t capacity2 = 40 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(40) + 85 * JSON_OBJECT_SIZE(1) + 41 * JSON_OBJECT_SIZE(2) + 40 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 35 * JSON_OBJECT_SIZE(7) + 45 * JSON_OBJECT_SIZE(8) + 8740;
const char* NTPServer = "hr.pool.ntp.org";
uint16_t ntpPort = 123;
IPAddress ntpIp;
uint8_t ntpPacket[48];
char opis[70];
int icon = 1;
float temperature, bmeTemp, bmeHum, bmePress, windSpeed;
int hum, pres;
const char* desc;
uint8_t sec, minu, hr;
uint32_t sunrise, sunset;
uint8_t noInternet = 0;
uint8_t modeSelect = 0;

class ntpInfo {
  public:
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
  display.display();
  int n;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  //WiFi.setHostname("WeatherPaper");
  Wire.begin();
  Serial.begin(115200);
  bme.begin(0x76);
  //initTouch();
  display.setRotation(2);
  display.setFont(&FreeSansOblique18pt7b);
  display.setTextColor(BLACK, WHITE);
  //display.einkOn();
  display.setCursor(0, 24);
  n = WiFi.scanNetworks();
  //display.clean(0);
  //display.clean(1);
  //display.clean(0);
  //dummyData();
  //delay(4000);
  //display.einkOff();
  //do {
  //  delay(100);
  //} while (true);
  //display.drawBitmap(0, 0, wifi, 64, 50, BLACK);
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
  //display.clean(0);
  //display.clean(1);
  //display.clean(0);
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
  //display.clean(0);
  //display.clean(1);
  //display.clean(0);
  refresh();
  //btStop();
  //WiFi.mode(WIFI_OFF);
}
unsigned long time1 = 0;
void loop() {
/*
  if (touchAvailable()) {
    int ts[2];
    if (readTouch(ts)) {
      //Serial.print('?');
      //Serial.print(ts[0], DEC);
      //Serial.print(' ');
      //Serial.println(ts[1], DEC);
      ts[0] = map(ts[0], 416, 3050, 0, 800);
      ts[1] = map(ts[1], 320, 2670, 0, 600);
      switch (modeSelect) {
        case 0:
          if (ts[1] > 410 && ts[0] > 0 && ts[0] < 800) {
            drawDays(ts[0] / 160);
            modeSelect = 1;
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
*/
  if ((unsigned long)(millis() - time1) > 300000) {
    time1 = millis();
    wifim.addAP(ssid, pass);
    while (wifim.run() != WL_CONNECTED) {
      delay(250);
    }
    readSensor();
    readWeather();
    readNTP();
    btStop();
    WiFi.disconnect();
    delay(10);
    WiFi.mode(WIFI_OFF);
    delay(1);
    refresh();
  }
}
/*
  void refreshOld() {
  char tmp[60];
  sprintf(tmp, "Osvjezeno %2d:%02d:%02d", hr, minu, sec);
  display.clearDisplay();
  display.setFont(&FreeSansOblique18pt7b);
  display.drawBitmap(20, 15, pressure, 50, 50, 1);
  display.setCursor(90, 50);
  display.print(pres, DEC);
  display.print(" hPa");
  display.setCursor(300, 50);
  display.print(opis);
  display.drawBitmap(40, 75, temp, 14, 50, 1);
  display.setCursor(90, 110);
  display.print(temperature, 1);
  display.print(" C");
  display.drawBitmap(30, 135, vlaga, 31, 50, 1);
  display.setCursor(90, 170);
  display.print(hum, DEC);
  display.print(" %");
  display.drawBitmap(15, 195, vjetar, 57, 50, 1);
  display.setCursor(90, 230);
  display.print(windSpeed, 1);
  display.print(" m/s");
  display.drawBitmap(730, 15, wifi, 64, 50, 1);
  display.drawCircle(satX, satY, 150, BLACK);
  int x0 = 140 * cos(((sec * 6.0) / 180 * PI) - PI / 2) + satX;
  int y0 = 140 * sin(((sec * 6.0) / 180 * PI) - PI / 2) + satY;
  display.drawLine(satX, satY, x0, y0, BLACK);
  x0 = 120 * cos(((minu * 6.0) / 180 * PI) - PI / 2) + satX;
  y0 = 120 * sin(((minu * 6.0) / 180 * PI) - PI / 2) + satY;
  int x1 = 4 * cos((minu * 6.0) / 180 * PI) + satX;
  int y1 = 4 * sin((minu * 6.0) / 180 * PI) + satY;
  int x2 = -4 * cos((minu * 6.0) / 180 * PI) + satX;
  int y2 = -4 * sin((minu * 6.0) / 180 * PI) + satY;
  display.fillTriangle(x0, y0, x1, y1, x2, y2, BLACK);
  x0 = 90 * cos((((hr * 30.0) + (minu / 2)) / 180 * PI) - PI / 2) + satX;
  y0 = 90 * sin((((hr * 30.0) + (minu / 2)) / 180 * PI) - PI / 2) + satY;
  x1 = 8 * cos(((hr * 30.0) + (minu / 2)) / 180 * PI) + satX;
  y1 = 8 * sin(((hr * 30.0) + (minu / 2)) / 180 * PI) + satY;
  x2 = -8 * cos(((hr * 30.0) + (minu / 2)) / 180 * PI) + satX;
  y2 = -8 * sin(((hr * 30.0) + (minu / 2)) / 180 * PI) + satY;
  display.fillTriangle(x0, y0, x1, y1, x2, y2, BLACK);
  display.drawBitmap(600, 300, sunriseIcon, 50, 50, BLACK);
  display.drawBitmap(600, 360, sunsetIcon, 51, 50, BLACK);
  display.setCursor(660, 350);
  sprintf(tmp, "%d:%02d", sunrise / 3600 % 24, sunrise / 60 % 60);
  display.print(tmp);
  display.setCursor(660, 410);
  sprintf(tmp, "%d:%02d", sunset / 3600 % 24, sunset / 60 % 60);
  display.print(tmp);
  sprintf(tmp, "Zadnje ocitanje: %d:%02d:%02d", hr, minu, sec);
  display.setCursor(0, 590);
  display.print(tmp);
  display.fillRect(10, 550, 780, 4, BLACK);
  display.drawBitmap(500, 565, spot, 20, 30, BLACK);
  display.setCursor(540, 590);
  display.print("Belisce");
  display.drawPixel(40, 550, BLACK);
  display.setFont();
  display.setTextSize(1);
  display.setCursor(50, 260);
  display.print("Powered by openweathermaps.com");
  display.drawBitmap(100, 480, icon13d, 50, 50, BLACK);
  display.einkOn();
  display.display();
  display.einkOff();
  }
*/

void readSensor() {
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
    epochDate(unix);
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
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/weather?&lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) {
    if (http.GET() > 0) {
      String APIData;
      DynamicJsonDocument doc(capacity);
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
      sunrise = sys["sunrise"];
      sunrise += 7200;
      sunset = sys["sunset"];
      sunset += 7200;
    } else {
      noInternet = 1;
      return;
    }
  }


  //Uhvati podatke s APIa s Interneta, te ih smjesti u polje struktura (privremeno)
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/forecast?lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) {
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
      do {
        epochDate(prognoza[i].epoch);
        i++;
      } while (td.h != 0);
      timeOffset = i - 1;

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
  display.setFont();
  display.setCursor(4, 55);
  display.print(opis);

  sprintf(tmp, "%1d:%02d %d/%d/%4d %3s", td.h, td.m, td.d, td.mo, td.y, DOW[td.dow]);
  display.setFont(&FreeSansOblique18pt7b);
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
  display.print(" m/s");
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
    epochDate(prognozaDani[i - 1 + k].epoch[0]);
    display.fillRect(xOffset, 410, 3, 180, BLACK);
    display.drawBitmap(xOffset - 105, 420, weatherIcon(atoi(prognozaDani[i - 1 + k].icon[4])), 50, 50, BLACK);
    display.setTextSize(1);
    display.setFont();
    display.setCursor(xOffsetText + 10, 472);
    changeLetters(prognozaDani[i - 1 + k].description[4]);
    display.print(prognozaDani[i - 1 + k].description[4]);
    display.setTextSize(1);
    display.setCursor(xOffsetText + 40, 405);
    display.print(td.d);
    display.print('.');
    display.print(td.mo);
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
    display.setFont(&FreeSansOblique18pt7b);
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
    display.print(DOW[td.dow]);
  }

  for (int i = 1; i < 3; i++) {
    display.fillRect(267 * i, 60, 3, 290, BLACK);
  }

  if (noInternet) {
    noInternet = 0;
    display.fillRect(100, 402, 600, 100, WHITE);
    display.drawRect(99, 401, 602, 102, BLACK);
    display.drawRect(98, 400, 604, 104, BLACK);
    display.setFont(&FreeSansOblique18pt7b);
    display.setTextSize(2);
    display.setCursor(170, 470);
    display.print("Nema Interneta");
  }
  display.einkOn();
  time2 = millis();
  display.display();
  time3 = millis();
  delay(100);
  display.einkOff();
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

void epochDate(uint32_t epoch) {
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

  td.d = ntp_date;
  td.mo = ntp_month;
  td.y = ntp_year;
  td.dow = ntp_week_day;
  td.h = ntp_hour;
  td.m = ntp_minute;
  td.s = ntp_second;
}

void drawDays(uint8_t n) {
  n += (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
  display.clearDisplay();
  display.setFont();
  for (int i = 0; i < prognozaDani[n].nData; i++) {
    epochDate(prognozaDani[n].epoch[i]);
    display.drawBitmap(i * 80 + 10, 100, weatherIcon(atoi(prognozaDani[n].icon[i])), 50, 50, BLACK);
    display.setCursor(i * 80 + 20, 160);
    display.print(td.h);
    display.print('h');
  }
  drawGraph(80, 250, prognozaDani[n].temp, 0, 0, 250, 130, prognozaDani[n].nData);
  drawGraph(480, 250, prognozaDani[n].pressure, 0, 0, 250, 130, prognozaDani[n].nData);
  drawGraph(80, 400, prognozaDani[n].humidity, 0, 0, 250, 130, prognozaDani[n].nData);
  drawGraph(480, 400, prognozaDani[n].windSpeed, 0, 0, 250, 130, prognozaDani[n].nData);
  epochDate(prognozaDani[n].epoch[0]);
  display.fillTriangle(770, 570, 790, 570, 780, 580, BLACK);
  display.setCursor(150, 200);
  display.setFont(&FreeSansOblique18pt7b);
  display.setTextSize(1);
  display.print("Odabran je dan ");
  display.print(td.d, DEC);
  display.print('.');
  display.print(td.mo, DEC);
  display.einkOn();
  display.display();
  display.einkOff();
}

void drawGraph(int x, int y, float *dy, char tx, char ty, int sizex, int sizey, char n) {
  display.fillRect(x, y, 2, sizey, BLACK);
  //float maxValueX = dx[0], minValueX = dx[0];
  float maxValueY = dy[0], minValueY = dy[0];
  int stepX;
  display.fillRect(x, y + sizey, sizex, 2, BLACK);
  for (int i = 0; i < n; i++) {
    //if (maxValueX < dx[i]) maxValueX = dx[i];
    //if (minValueX > dx[i]) minValueX = dx[i];
    if (maxValueY < dy[i]) maxValueY = dy[i];
    if (minValueY > dy[i]) minValueY = dy[i];
  }
  stepX = sizex / n;
  display.setFont();
  display.setTextSize(1);
  display.setCursor(x + sizex, y);
  display.print(maxValueY, 1);
  display.setCursor(x + sizex, y + sizey);
  display.print(minValueY, 1);
  minValueY -= 5;
  maxValueY += 5;
  int x1, y1, x2, y2;
  for (int i = 0; i < n - 1; i++) {
    x1 = i * stepX + x;
    x2 = (i + 1) * stepX + x;
    y1 = map(dy[i], minValueY, maxValueY, y + sizey, y);
    y2 = map(dy[i + 1], minValueY, maxValueY, y + sizey, y);
    display.drawLine(x1, y1, x2, y2, BLACK);
    display.fillCircle(x2, y2, 2, BLACK);
  }

}
