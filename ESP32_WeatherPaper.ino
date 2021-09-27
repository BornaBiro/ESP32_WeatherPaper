#include "Inkplate.h"
#include "driver/rtc_io.h"
#include "Neucha_Regular17pt7b.h"
#include "RobotoCondensed_Regular6pt7b.h"
#include "icons.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "TSC2046E_Inkplate.h"
#include "sys/time.h"

#include "structs.h"

//.ttf to .h: https://rop.nl/truetype2gfx/
//Open Source fonts https://fonts.google.com/
#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL &RobotoCondensed_Regular6pt7b

Inkplate display(INKPLATE_1BIT);
TSC2046E_Inkplate ts;
SPIClass *mySpi = NULL;

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

timeval tm;
const timeval* tmPtr = &tm;
uint64_t timeToWake;
int timeOffset;

const char ssid[] = "Biro_WLAN";
const char pass[] = "CaVex250_H2sH11";

const char DOW[7][4] = {"NED", "PON", "UTO", "SRI", "CET", "PET", "SUB"};
const char* oznakeVjetar[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
const char* strErr = {"ERR!"};

StaticJsonDocument<2000> doc;
const size_t capacity2 = 40 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(40) + 89 * JSON_OBJECT_SIZE(1) + 41 * JSON_OBJECT_SIZE(2) + 40 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 31 * JSON_OBJECT_SIZE(7) + 10 * JSON_OBJECT_SIZE(8) + 40 * JSON_OBJECT_SIZE(9) + 9360;
char opis[70];
int icon = 1;
float temperature, windSpeed;
int hum, pres, windDir;
const char* desc;
uint32_t sunrise, sunset;
uint8_t noInternet = 0;
uint8_t modeSelect = 0;
uint8_t forcedRef = 0;
uint8_t selectedDay = 0;

int timezone;

unsigned long time2, time3;
void setup() {
  display.begin();
  mySpi = display.getSPIptr();
  mySpi->begin(14, 12, 13, 15);
  ts.begin(mySpi, &display, 13, 14);
  ts.calibrate(800, 3420, 3553, 317, 0, 799, 0, 599);

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
  display.setTextWrap(false);
  display.setFont(DISPLAY_FONT);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 24);
  int n = WiFi.scanNetworks();
  if (n > 6) n = 6;
  display.clearDisplay();
  display.setCursor(0, 24);
  display.println("ESP32 WeatherPaper\nTrazenje WiFi Mreza...");
  display.display();
  for (int i = 0; i < n; i++) {
    display.setCursor(70, 100 + (i * 35));
    display.print(WiFi.SSID(i));
    display.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? ' ' : '*');
    display.print(' ');
    display.print(WiFi.RSSI(i), DEC);
  }
  display.partialUpdate();
  display.setCursor(70, 550);
  display.print(F("Spajanje na "));
  display.print(ssid);
  display.partialUpdate(false, true);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    display.print('.');
    display.partialUpdate(false, true);
  }
  display.print("spojeno! Pricekajte...");
  display.partialUpdate();

  readSensor(&sensor);
  readWeather();
  if (readNTP(&tm.tv_sec))
  {
    settimeofday(tmPtr, NULL);
    tNow = epochToHuman(tm.tv_sec);
  }
  gettimeofday(&tm, NULL);
  timeToWake = 1200 + tm.tv_sec;
  refresh();
  esp_wifi_stop();
  btStop();
}

void loop() {
  int tsX, tsY;
  display.digitalWriteMCP(15, HIGH);
  rtc_gpio_isolate(GPIO_NUM_12);
  gettimeofday(&tm, NULL);
  Serial.println(timeToWake, DEC);
  Serial.println(tm.tv_sec, DEC);
  Serial.println("-------");
  Serial.flush();
  esp_sleep_enable_timer_wakeup((timeToWake - tm.tv_sec) * 1000000ULL); //20 minuta
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
  esp_light_sleep_start();

  if (ts.available(&tsX, &tsY))
  {
    switch (modeSelect)
    {
      case 0:
        if (tsY > 410 && tsX > 0 && tsX < 800)
        {
          selectedDay = tsX / 160;
          drawDays(selectedDay, true);
          modeSelect = 1;
        }

        if (tsX > 25 && tsX < 75 && tsY > 75 && tsY < 125)
        {
          selectedDay = 0;
          drawDays(selectedDay, true);
          modeSelect = 1;
        }

        if (tsY > 0 && tsY < 30 && tsX > 770 && tsX < 800)
        {
          display.setFont(DISPLAY_FONT);
          display.setCursor(610, 35);
          writeInfoBox(350, "Osvjez. podataka, pricekajte");
          display.partialUpdate();
          forcedRef = 1;
        }
        break;

      case 1:
        if (tsY > 500 && tsX > 700 && tsX < 800)
        {
          refresh();
          modeSelect = 0;
          selectedDay = 0;
        }
        if (tsX > 0 && tsX < 75 && tsY > 0 && tsY < 50 && selectedDay > 0)
        {
          selectedDay--;
          drawDays(selectedDay, false);
          modeSelect = 1;
        }
        if (tsX > 730 && tsX < 800 && tsY > 0 && tsY < 50 && selectedDay < 4)
        {
          selectedDay++;
          drawDays(selectedDay, false);
          modeSelect = 1;
        }

        if (tsY > 80 && tsY < 180)
        {
          display.fillRect(198, 198, 404, 304, BLACK);
          display.fillRect(200, 200, 400, 300, WHITE);
          int selectedHour = tsX / 100;
          int _offset = (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
          int _yPos = 240;
          const int _fontScale = 2;
          const int _fontYSize = 14 * _fontScale;
          if (selectedHour <= prognozaDani[selectedDay + _offset].nData)
          {
            display.setFont(DISPLAY_FONT_SMALL);
            display.setTextSize(_fontScale);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Opis: ");
            changeLetters(prognozaDani[selectedDay + _offset].description[selectedHour]);
            display.print(prognozaDani[selectedDay + _offset].description[selectedHour]);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Temp.: ");
            display.print(prognozaDani[selectedDay + _offset].tempMin[selectedHour], 1);
            display.print("[Min] / ");
            display.print(prognozaDani[selectedDay + _offset].temp[selectedHour], 1);
            display.print(" / ");
            display.print(prognozaDani[selectedDay + _offset].tempMax[selectedHour], 1);
            display.print("[Max] C");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Tlak zraka: ");
            display.print(prognozaDani[selectedDay + _offset].pressure[selectedHour], 1);
            display.print("hPa");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Brzina vjetra: ");
            display.print(prognozaDani[selectedDay + _offset].windSpeed[selectedHour], 1);
            display.print("m/s");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Smjer vjetra: ");
            display.print(prognozaDani[selectedDay + _offset].windDir[selectedHour], 1);
            display.print(" stupnjeva / ");
            display.print(oznakeVjetar[int((prognozaDani[selectedDay + _offset].windDir[selectedHour] / 22.5) + .5) % 16]);

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Naoblaka: ");
            display.print(prognozaDani[selectedDay + _offset].clouds[selectedHour], 1);
            display.print('%');

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Snijeg: ");
            display.print(prognozaDani[selectedDay + _offset].snow[selectedHour], 1);
            display.print("mm");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Kisa: ");
            display.print(prognozaDani[selectedDay + _offset].rain[selectedHour], 1);
            display.print("mm");
          }
          display.setTextSize(1);
          display.partialUpdate();
        }
        break;
    }
  }

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER || forcedRef)
  {
    forcedRef = 0;
    display.setFont(DISPLAY_FONT_SMALL);
    display.setTextSize(1);
    display.setCursor(550, 46);
    display.print("Dohvacanje novih podataka");
    esp_wifi_start();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      i++;
      if (i > 15) break;
      if (!(i % 2))
      {
        display.print('.');
        display.partialUpdate();
      }
    }

    if (i < 15) {
      readSensor(&sensor);
      readWeather();
      gettimeofday(&tm, NULL);
      tNow = epochToHuman(tm.tv_sec);
      timeToWake = 1200 + tm.tv_sec;
      refresh();
    }
    esp_wifi_stop();
    btStop();
  }
}

void writeInfoBox(int y, char* c) {
  int16_t  x1, y1;
  uint16_t w, h;
  int x;
  display.setTextSize(2);
  display.getTextBounds(c, 0, y, &x1, &y1, &w, &h);
  x = (800 - w) / 2;
  display.getTextBounds(c, x, y, &x1, &y1, &w, &h);
  display.fillRect(x1 - 3, y1 - 3, w + 6, h + 6, WHITE);
  display.drawRect(x1 - 2, y1 - 2, w + 4, h + 4, BLACK);
  display.drawRect(x1 - 1, y1 - 1, w + 2, h + 2, BLACK);
  display.setCursor(x, y);
  display.print(c);
  display.setTextSize(1);
}

void readSensor(struct sensorData *_s) {
  bme.takeForcedMeasurement();
  _s->temp = bme.readTemperature();
  _s->humidity = bme.readHumidity();
  _s->pressure = bme.readPressure() / 100.0F;
}

bool readNTP(time_t *_epoch) {
  IPAddress ntpIp;
  const char* NTPServer = "hr.pool.ntp.org";
  uint16_t ntpPort = 123;
  uint8_t ntpPacket[48];

  udp.begin(8888);
  if (!WiFi.hostByName(NTPServer, ntpIp)) return 0;
  
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
  if (udp.parsePacket())
  {
    udp.read(ntpPacket, 48);
    uint32_t unix = ntpPacket[40] << 24 | ntpPacket[41] << 16 | ntpPacket[42] << 8 | ntpPacket[43];
    *_epoch = unix - 2208988800UL + timezone;
    return true;
  }
  return false;
}
void readWeather() {
  int i;

  typedef struct wforecast {
    float temp, tempMax, tempMin, pressure, pressureSea, pressureGnd, windSpeed, windDir;
    int humidity;
    char description[30], icon[4];
    long epoch;
    int cond;
    int clouds;
    float snow;
    float rain;
  };
  wforecast prognoza[40];
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/weather?&lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657"))) {   //Belisce
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
      strncpy(opis, weather_0["description"] | strErr, sizeof(opis));
      strncpy(wIcon, weather_0["icon"] | strErr, sizeof(wIcon));
      icon = atoi(wIcon);
      changeLetters(opis);
      temperature = main["temp"];
      hum = main["humidity"];
      pres = main["pressure"];
      desc = weather_0["description"];
      windSpeed = doc["wind"]["speed"];
      windDir = doc["wind"]["deg"];
      timezone = doc["timezone"];
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
      JsonObject list_wind = listDate["wind"];
      for (i = 0; i < 40; i++) {
        listDate = list[i];
        list_main = listDate["main"];
        list_weather = listDate["weather"][0];
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
        prognoza[i].clouds = listDate["clouds"]["all"];
        prognoza[i].snow = listDate["snow"]["3h"];
        prognoza[i].rain = listDate["rain"]["3h"];
        strncpy(prognoza[i].icon, list_weather["icon"] | strErr, 3);
        strncpy(prognoza[i].description, list_weather["description"] | strErr, 19);
        prognoza[i].windSpeed = list_wind["speed"];
        prognoza[i].windDir = list_wind["deg"];
      }
      i = 0;
      while (epochToHuman(prognoza[i].epoch).tm_hour != 0)
      {
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
            prognozaDani[i].clouds[j] = prognoza[i * 8 + j].clouds;
            prognozaDani[i].snow[j] = prognoza[i * 8 + j].snow;
            prognozaDani[i].rain[j] = prognoza[i * 8 + j].rain;
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
          prognozaDani[0].clouds[i] = prognoza[i].clouds;
          prognozaDani[0].snow[i] = prognoza[i].snow;
          prognozaDani[0].rain[0] = prognoza[i].rain;
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
            prognozaDani[i + 1].clouds[j] = prognoza[z].clouds;
            prognozaDani[i + 1].snow[j] = prognoza[z].snow;
            prognozaDani[i + 1].rain[j] = prognoza[z].rain;
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
          prognozaDani[5].clouds[k] = prognoza[i].cond;
          prognozaDani[5].snow[k] = prognoza[i].snow;
          prognozaDani[5].rain[k] = prognoza[i].rain;
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
  display.setFont(DISPLAY_FONT_SMALL);
  display.setTextSize(2);
  display.setCursor(1, 70);
  display.print(opis);
  display.setCursor(1, 20);
  display.print(F("Belisce"));
  display.setTextSize(1);
  display.setCursor(1, 46);
  display.print(F("Powered by openweathermaps.org"));
  display.drawBitmap(770, 0, refIcon, 30, 30, BLACK);

  sprintf(tmp, "%1d:%02d %d/%d/%04d %3s", tNow.tm_hour, tNow.tm_min, tNow.tm_mday, tNow.tm_mon + 1, tNow.tm_year + 1900, DOW[tNow.tm_wday]);

  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 796, 3, BLACK);
  display.setCursor(267, 35);
  display.print(tmp);

  display.drawBitmap(20, 85, weatherIcon(icon), 50, 50, BLACK);
  display.setTextSize(2);
  display.setCursor(80, 130);
  display.print(temperature, 0);
  display.setCursor(display.getCursorX() + 5, 110);
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
  display.drawBitmap(300, 140, iconTlak, 40, 40, BLACK);
  display.setCursor(350, 170);
  display.print(sensor.pressure, 1);
  display.drawBitmap(300, 190, iconVlaga, 40, 40, BLACK);
  display.setCursor(350, 220);
  display.print(sensor.humidity, 1);
  display.drawBitmap(300, 240, iconTemp, 40, 40, BLACK);
  display.setCursor(350, 270);
  display.print(sensor.temp, 1);

  int k = (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
  int xOffset, xOffsetText;
  for (int i = 1; i < 6; i++) {
    xOffset = i * 160;
    xOffsetText = ((i - 1) * 160) + 20;
    display.fillRect(xOffset, 410, 3, 180, BLACK);
    display.drawBitmap(xOffset - 105, 420, weatherIcon(atoi(prognozaDani[i - 1 + k].icon[4])), 50, 50, BLACK);
    display.setTextSize(1);
    display.setFont(DISPLAY_FONT_SMALL);
    display.setCursor(xOffsetText + 10, 482);
    changeLetters(prognozaDani[i - 1 + k].description[4]);
    display.print(prognozaDani[i - 1 + k].description[4]);
    display.setTextSize(1);
    display.setCursor(xOffsetText + 40, 415);
    display.print(epochToHuman(prognozaDani[i - 1 + k].epoch[0]).tm_mday);
    display.print('.');
    display.print(epochToHuman(prognozaDani[i - 1 + k].epoch[0]).tm_mon + 1);
    display.setCursor(xOffsetText - 10, 575);
    display.print(dani[i - 1 + k].pressure, 1);
    display.print("hPa  ");
    display.print(dani[i - 1 + k].humidity, 1);
    display.print('%');
    display.setCursor(xOffsetText - 10, 588);
    display.print(dani[i - 1 + k].windSpeed, 1);
    display.print("m/s ");
    display.print(oznakeVjetar[int((dani[i - 1 + k].windDir / 22.5) + .5) % 16]);
    display.print(' ');
    display.print(dani[i - 1 + k].windSpeedMax, 1);
    display.print("m/s");
    display.setFont(DISPLAY_FONT);
    display.setCursor(xOffsetText, 520);
    display.print(dani[i - 1 + k].tempMax, 1);
    display.setCursor(display.getCursorX() + 5, 510);
    display.print("o");
    display.setCursor(xOffsetText, 555);
    display.print(dani[i - 1 + k].tempMin, 1);
    display.setCursor(display.getCursorX() + 5, 545);
    display.print('o');
    display.setCursor(xOffsetText + 20, 400);
    display.print(DOW[epochToHuman(prognozaDani[i - 1 + k].epoch[0]).tm_wday]);
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
  display.display();
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

struct tm epochToHuman(time_t _t)
{
  struct tm *_timePtr;
  struct tm _time;
  _timePtr = localtime(&_t);
  memcpy(&_time, _timePtr, sizeof(_time));
  return _time;
}

void drawDays(uint8_t n, bool fullUpdate) {
  char dayHours[8];
  n += (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
  display.clearDisplay();
  display.setFont(DISPLAY_FONT_SMALL);
  display.setTextSize(2);
  for (int i = 0; i < prognozaDani[n].nData; i++) {
    dayHours[i] = epochToHuman(prognozaDani[n].epoch[i]).tm_hour;
    display.drawBitmap(i * 95 + 45, 100, weatherIcon(atoi(prognozaDani[n].icon[i])), 50, 50, BLACK);
    display.setCursor(i * 95 + 30, 170);
    display.print(dayHours[i], DEC);
    display.print(":00h");
  }
  display.setTextSize(1);
  drawGraph(80, 250, dayHours, prognozaDani[n].temp, "h", "C", 250, 130, prognozaDani[n].nData, 1, 7);
  drawGraph(480, 250, dayHours, prognozaDani[n].pressure, "h", "hPa", 250, 130, prognozaDani[n].nData, 1, 7);
  drawGraph(80, 420, dayHours, prognozaDani[n].humidity, "h", "%", 250, 130, prognozaDani[n].nData, 0, 7);
  drawGraph(480, 420, dayHours, prognozaDani[n].windSpeed, "h", "m/s", 250, 130, prognozaDani[n].nData, 1, 7);
  if (n > 1) display.fillTriangle(50, 10, 50, 40, 20, 25, BLACK);
  if (n < 5) display.fillTriangle(750, 10, 750, 40, 780, 25, BLACK);
  display.fillTriangle(770, 570, 790, 570, 780, 580, BLACK);

  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 796, 3, BLACK);
  display.setCursor(200, 35);
  display.print("Vremenska prognoza za ");
  display.print(epochToHuman(prognozaDani[n].epoch[0]).tm_mday, DEC);
  display.print('.');
  display.print(epochToHuman(prognozaDani[n].epoch[0]).tm_mon + 1, DEC);
  display.print(".,");
  display.print(DOW[epochToHuman(prognozaDani[n].epoch[0]).tm_wday]);
  if (fullUpdate) {
    display.display();
  } else {
    display.partialUpdate();
  }
}
void drawGraph(int x, int y, char *dx, float *dy, char *tx, char *ty, int sizex, int sizey, char n, char decimal, char yelements) {
  char digits = 0;
  float maxValueX = dx[0], minValueX = dx[0];
  float maxValueY = dy[0], minValueY = dy[0];
  int stepX;
  float stepY;
  int stepYGraph;
  for (int i = 0; i < n; i++) {
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
