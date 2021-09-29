// Includes for communication and periph.
#include <Wire.h>
#include <SPI.h>
#include "driver/rtc_io.h"

// WiFi and Internet includes
#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// Display includes (driver, icons, fonts, etc)
#include "Inkplate.h"
#include "Neucha_Regular17pt7b.h"
#include "RobotoCondensed_Regular6pt7b.h"
#include "icons.h"
#include "TSC2046E_Inkplate.h"

// Sensor includes
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Other includes
#include "sys/time.h"
#include "structs.h"

//.ttf to .h: https://rop.nl/truetype2gfx/
//Open Source fonts https://fonts.google.com/
#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL &RobotoCondensed_Regular6pt7b

// Objects / constructors
Inkplate display(INKPLATE_1BIT);
TSC2046E_Inkplate ts;
SPIClass *mySpi = NULL;
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

uint8_t modeSelect = 0;
uint8_t forcedRef = 0;
uint8_t selectedDay = 0;

void setup()
{
  display.begin();
  Wire.begin();
  Serial.begin(115200);

  mySpi = display.getSPIptr();
  mySpi->begin(14, 12, 13, 15);
  ts.begin(mySpi, &display, 13, 14);
  ts.calibrate(800, 3420, 3553, 317, 0, 799, 0, 599);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setHostname("WeatherPaper");
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
  for (int i = 0; i < n; i++)
  {
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
  while (WiFi.status() != WL_CONNECTED)
  {
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

void loop()
{
  int tsX, tsY;
  display.digitalWriteMCP(15, HIGH);
  rtc_gpio_isolate(GPIO_NUM_12);
  gettimeofday(&tm, NULL);
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
          int selectedElement = tsX / 100;
          int _offset = (timeOffset > 0 && timeOffset < 4) ? 1 : 0;
          int _yPos = 240;
          const int _fontScale = 2;
          const int _fontYSize = 14 * _fontScale;
          if (selectedElement <= (forecastList.startElement[selectedDay + 2 - forecastList.shiftDay] - forecastList.startElement[selectedDay + 1 - forecastList.shiftDay]))
          {
            int element = forecastList.startElement[selectedDay + 1 - forecastList.shiftDay] + selectedElement;
            Serial.println(element);
            display.fillRect(198, 198, 404, 304, BLACK);
            display.fillRect(200, 200, 400, 300, WHITE);
            display.setFont(DISPLAY_FONT_SMALL);
            display.setTextSize(_fontScale);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            // changeLetters(prognozaDani[selectedDay + _offset].description[selectedElement]);
            // display.print(prognozaDani[selectedDay + _offset].description[selectedElement]);
            display.print(forecastList.forecast[element].weatherDesc);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Temp.: ");
            display.print(prognozaDani[selectedDay + _offset].tempMin[selectedElement], 1);
            display.print("[Min] / ");
            display.print(forecastList.forecast[element].temp, 1);
            display.print(" / ");
            display.print(prognozaDani[selectedDay + _offset].tempMax[selectedElement], 1);
            display.print("[Max] C");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Tlak zraka: ");
            display.print(forecastList.forecast[element].pressureGnd);
            display.print("hPa");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Brzina vjetra: ");
            display.print(forecastList.forecast[element].windSpeed, 1);
            display.print("m/s");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Smjer vjetra: ");
            display.print(oznakeVjetar[int((forecastList.forecast[element].windDir / 22.5) + .5) % 16]);
            display.print('[');
            display.print(forecastList.forecast[element].windDir);
            display.print(" st.]");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Naoblaka: ");
            display.print(forecastList.forecast[element].clouds);
            display.print('%');

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Snijeg: ");
            display.print(forecastList.forecast[element].snow, 1);
            display.print("mm");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Kisa: ");
            display.print(forecastList.forecast[element].rain, 1);
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
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      i++;
      if (i > 15) break;
      if (!(i % 2))
      {
        display.print('.');
        display.partialUpdate();
      }
    }

    if (i < 15)
    {
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

void writeInfoBox(int y, char* c)
{
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

void readSensor(struct sensorData *_s)
{
  bme.takeForcedMeasurement();
  _s->temp = bme.readTemperature();
  _s->humidity = bme.readHumidity();
  _s->pressure = bme.readPressure() / 100.0F;
}

bool readNTP(time_t *_epoch)
{
  IPAddress ntpIp;
  WiFiUDP udp;
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
    *_epoch = unix - 2208988800UL + currentWeather.timezone;
    return true;
  }
  return false;
}
void readWeather()
{
  WiFiClient client;
  HTTPClient http;
  DynamicJsonDocument doc(30000);

  http.useHTTP10(true);
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/weather?&lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657")))
  {
    if (http.GET() > 0)
    {
      deserializeJson(doc, http.getStream());
      if (doc["cod"] == 200)
      {
        strncpy(currentWeather.weatherIcon, doc["weather"][0]["icon"], sizeof(currentWeather.weatherIcon) - 1);
        changeLetters(strncpy(currentWeather.weatherDesc, doc["weather"][0]["description"], sizeof(currentWeather.weatherDesc) - 1));
        changeLetters(strncpy(currentWeather.city, doc["name"], sizeof(currentWeather.city) - 1));
        currentWeather.temp = doc["main"]["temp"];
        currentWeather.feelsLike = doc["main"]["feels_like"];
        currentWeather.humidity = doc["main"]["humidity"];
        currentWeather.pressure = doc["main"]["pressure"];
        currentWeather.pressureGnd = doc["main"]["grnd_level"];
        currentWeather.windSpeed = doc["wind"]["speed"];
        currentWeather.windGust = doc["wind"]["gust"];
        currentWeather.windDir = doc["wind"]["deg"];
        currentWeather.visibility = doc["visibility"];
        currentWeather.clouds = doc["clouds"]["all"];
        currentWeather.rain = doc["rain"]["1h"];
        currentWeather.snow = doc["snow"]["1h"];
        currentWeather.timezone = doc["timezone"];
        currentWeather.timestamp = doc["dt"];
        currentWeather.sunrise = doc["sys"]["sunrise"];
        currentWeather.sunrise += currentWeather.timezone;
        currentWeather.sunset = doc["sys"]["sunset"];
        currentWeather.sunset += currentWeather.timezone;
      }
    }
  }
  http.end();

  // https://api.openweathermap.org/data/2.5/onecall?lat=45.68&lon=18.41&units=metric&lang=hr&exclude=current,minutely,daily&appid=b1d3e9077193732d4b5e3e2c4c036657 Za dnevnu i za upozorenja
  
  if (http.begin(client, F("http://api.openweathermap.org/data/2.5/forecast?lat=45.68&lon=18.41&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657")))
  {
    if (http.GET() > 0)
    {
      doc.clear();
      doc.garbageCollect();
      deserializeJson(doc, http.getStream());
      if (atoi(doc["cod"]) == 200)
      {
        forecastList.numberOfData = doc["cnt"];
        for (int i = 0; i < forecastList.numberOfData; i++)
        {
          changeLetters(strncpy(forecastList.forecast[i].weatherDesc, doc["list"][i]["weather"][0]["description"], sizeof(forecastList.forecast[i].weatherDesc) - 1));
          strncpy(forecastList.forecast[i].weatherIcon, doc["list"][i]["weather"][0]["icon"], sizeof(forecastList.forecast[i].weatherIcon) - 1);
          forecastList.forecast[i].clouds = doc["list"][i]["clouds"]["all"];
          forecastList.forecast[i].humidity = doc["list"][i]["main"]["humidity"];
          forecastList.forecast[i].probability = (float)(doc["list"][i]["pop"]) * 100;
          forecastList.forecast[i].weatherId = doc["list"][i]["weather"][0]["id"];
          forecastList.forecast[i].pressureGnd = doc["list"][i]["main"]["grnd_level"];
          forecastList.forecast[i].visibility = doc["list"][i]["visibility"];
          forecastList.forecast[i].pressure = doc["list"][i]["main"]["pressure"];
          forecastList.forecast[i].windDir = doc["list"][i]["wind"]["deg"];
          forecastList.forecast[i].temp = doc["list"][i]["main"]["temp"];
          forecastList.forecast[i].feelsLike = doc["list"][i]["main"]["feels_like"];
          forecastList.forecast[i].windSpeed = doc["list"][i]["wind"]["speed"];
          forecastList.forecast[i].windGust = doc["list"][i]["wind"]["gust"];
          forecastList.forecast[i].rain = doc["list"][i]["rain"]["3h"];
          forecastList.forecast[i].snow = doc["list"][i]["snow"]["3h"];
          forecastList.forecast[i].timestamp = doc["list"][i]["dt"];
        }

        // Group data into "days"
        // Skip first element
        int n = 1;
        forecastList.startElement[0] = 0;
        for (int i = 1; i < forecastList.numberOfData; i++)
        {
          if (epochToHuman(forecastList.forecast[i].timestamp).tm_hour == 0)
          {
            forecastList.startElement[n] = i;
            n++;
          }
        }
        forecastList.startElement[n] = forecastList.numberOfData;

        // Calculate avg and max data for forecast display
        memset(forecastDisplay, 0, sizeof(forecastDisplay));
        for (int i = 0; i < 6; i++)
        {
          int nElements = forecastList.startElement[i + 1] - forecastList.startElement[i] + 1;
          forecastDisplay[i].maxTemp = forecastList.forecast[forecastList.startElement[i]].temp;
          forecastDisplay[i].minTemp = forecastList.forecast[forecastList.startElement[i]].temp;
          forecastDisplay[i].maxWindSpeed = forecastList.forecast[forecastList.startElement[i]].windGust;
          for (int j = forecastList.startElement[i]; j < forecastList.startElement[i + 1]; j++)
          {
            forecastDisplay[i].avgPressure += forecastList.forecast[j].pressureGnd;
            forecastDisplay[i].avgHumidity += forecastList.forecast[j].humidity;
            forecastDisplay[i].avgWindSpeed += forecastList.forecast[j].windSpeed;
            if (forecastDisplay[i].maxTemp < forecastList.forecast[j].temp) forecastDisplay[i].maxTemp = forecastList.forecast[j].temp;
            if (forecastDisplay[i].minTemp > forecastList.forecast[j].temp) forecastDisplay[i].minTemp = forecastList.forecast[j].temp;
            if (forecastDisplay[i].maxWindSpeed < forecastList.forecast[j].windGust) forecastDisplay[i].maxWindSpeed = forecastList.forecast[j].windGust;
            if (epochToHuman(forecastList.forecast[j].timestamp).tm_hour == 15)
            {
              forecastDisplay[i].weatherIcon = forecastList.forecast[j].weatherIcon;
              forecastDisplay[i].weatherDesc = forecastList.forecast[j].weatherDesc;
            }
          }
          forecastDisplay[i].avgPressure /= nElements;
          forecastDisplay[i].avgHumidity /= nElements;
          forecastDisplay[i].avgWindSpeed /= nElements;
        }
        forecastList.shiftDay = 1;
        if (forecastList.startElement[1] - forecastList.startElement[0] < 3) forecastList.shiftDay = 0;
      }
    }
    http.end();
  }
}

void changeLetters(char *p)
{
  int webTextSize = strlen(p);
  for (int16_t i = 0; i < webTextSize; i++)
  {
    if (p[i] == 196 || p[i] == 197) memmove(&p[i], &p[i + 1], webTextSize - i);
    if (p[i] == 141 || p[i] == 135) p[i] = 'c';
    if (p[i] == 161) p[i] = 's';
    if (p[i] == 190) p[i] = 'z';
  }
}

void refresh()
{
  char tmp[50];
  display.clearDisplay();
  display.setTextSize(1);
  display.fillRect(20, 360, 760, 3, BLACK);
  display.setFont(DISPLAY_FONT_SMALL);
  display.setTextSize(2);
  display.setCursor(1, 70);
  display.print(currentWeather.weatherDesc);
  display.setCursor(1, 20);
  display.print(currentWeather.city);
  display.setTextSize(1);
  display.setCursor(1, 46);
  display.print(F("Powered by openweathermaps.org"));
  display.drawBitmap(770, 0, refIcon, 30, 30, BLACK);

  sprintf(tmp, "%1d:%02d %d/%d/%04d %3s", tNow.tm_hour, tNow.tm_min, tNow.tm_mday, tNow.tm_mon + 1, tNow.tm_year + 1900, DOW[tNow.tm_wday]);

  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 796, 3, BLACK);
  display.setCursor(267, 35);
  display.print(tmp);

  display.drawBitmap(20, 85, weatherIcon(atoi(currentWeather.weatherIcon)), 50, 50, BLACK);
  display.setTextSize(2);
  display.setCursor(80, 130);
  display.print(currentWeather.temp, 0);
  display.setCursor(display.getCursorX() + 5, 110);
  display.print('o');
  display.setTextSize(1);
  display.drawBitmap(1, 140, iconTlak, 40, 40, BLACK);
  display.setCursor(45, 170);
  display.print(currentWeather.pressureGnd, DEC);
  display.drawBitmap(145, 140, iconVlaga, 40, 40, BLACK);
  display.setCursor(195, 170);
  display.print(currentWeather.humidity);
  display.drawBitmap(1, 190, iconVjetar, 40, 40, BLACK);
  display.setCursor(45, 220);
  display.print(currentWeather.windSpeed, 1);
  display.print(" m/s | ");
  display.print(oznakeVjetar[int((currentWeather.windDir / 22.5) + .5) % 16]);
  display.drawBitmap(1, 240, iconSunrise, 40, 40 , BLACK);
  sprintf(tmp, "%d:%02d", epochToHuman(currentWeather.sunrise).tm_hour, epochToHuman(currentWeather.sunrise).tm_min);
  display.setCursor(40, 270);
  display.print(tmp);
  display.drawBitmap(130, 240, iconSunset, 40, 40 , BLACK);
  sprintf(tmp, "%d:%02d", epochToHuman(currentWeather.sunset).tm_hour, epochToHuman(currentWeather.sunset).tm_min);
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

  int xOffset, xOffsetText;
  for (int i = 1; i < 6; i++) 
  {
    xOffset = i * 160;
    xOffsetText = ((i - 1) * 160) + 20;
    display.fillRect(xOffset, 410, 3, 180, BLACK);
    display.drawBitmap(xOffset - 105, 420, weatherIcon(atoi(forecastDisplay[i - forecastList.shiftDay].weatherIcon)), 50, 50, BLACK);
    display.setTextSize(1);
    display.setFont(DISPLAY_FONT_SMALL);
    display.setCursor(xOffsetText + 10, 482);
    display.print(forecastDisplay[i - forecastList.shiftDay].weatherDesc);
    display.setCursor(xOffsetText + 40, 415);
    display.print(epochToHuman(forecastList.forecast[forecastList.startElement[i - forecastList.shiftDay]].timestamp).tm_mday);
    display.print('.');
    display.print(epochToHuman(forecastList.forecast[forecastList.startElement[i - forecastList.shiftDay]].timestamp).tm_mon + 1);
    display.setCursor(xOffsetText - 10, 575);
    display.print(forecastDisplay[i - forecastList.shiftDay].avgPressure);
    display.print("hPa  ");
    display.print(forecastDisplay[i - forecastList.shiftDay].avgHumidity);
    display.print('%');
    display.setCursor(xOffsetText - 10, 588);
    display.print(forecastDisplay[i - forecastList.shiftDay].avgWindSpeed, 1);
    display.print("m/s ");
    // display.print(oznakeVjetar[int((dani[i - 1 + k].windDir / 22.5) + .5) % 16]);
    display.print(' ');
    display.print(forecastDisplay[i - forecastList.shiftDay].maxWindSpeed, 1);
    display.print("m/s");
    display.setFont(DISPLAY_FONT);
    display.setCursor(xOffsetText, 520);
    display.print(forecastDisplay[i - forecastList.shiftDay].maxTemp);
    display.setCursor(display.getCursorX() + 5, 510);
    display.print("o");
    display.setCursor(xOffsetText, 555);
    display.print(forecastDisplay[i - forecastList.shiftDay].minTemp);
    display.setCursor(display.getCursorX() + 5, 545);
    display.print('o');
    display.setCursor(xOffsetText + 20, 400);
    display.print(DOW[epochToHuman(forecastList.forecast[forecastList.startElement[i - forecastList.shiftDay]].timestamp).tm_wday]);
  }

  for (int i = 1; i < 3; i++)
  {
    display.fillRect(267 * i, 60, 3, 290, BLACK);
  }

  display.display();
}

uint8_t* weatherIcon(uint8_t i)
{
  switch (i)
  {
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

void drawDays(uint8_t n, bool fullUpdate)
{
  display.clearDisplay();
  display.setFont(DISPLAY_FONT_SMALL);
  display.setTextSize(2);
  uint8_t xPos = 0;
  uint8_t start = forecastList.startElement[-forecastList.shiftDay + n + 1];
  uint8_t end = forecastList.startElement[-forecastList.shiftDay + n + 2];
  for (int i = start; i < end; i++)
  {
    display.drawBitmap(xPos * 95 + 45, 100, weatherIcon(atoi(forecastList.forecast[i].weatherIcon)), 50, 50, BLACK);
    display.setCursor(xPos * 95 + 30, 170);
    display.print(epochToHuman(forecastList.forecast[i].timestamp).tm_hour, DEC);
    display.print(":00h");
    xPos++;
  }
  display.setTextSize(1);
  // drawGraph(80, 250, dayHours, prognozaDani[n].temp, "h", "C", 250, 130, prognozaDani[n].nData, 1, 7);
  // drawGraph(480, 250, dayHours, prognozaDani[n].pressure, "h", "hPa", 250, 130, prognozaDani[n].nData, 1, 7);
  // drawGraph(80, 420, dayHours, prognozaDani[n].humidity, "h", "%", 250, 130, prognozaDani[n].nData, 0, 7);
  // drawGraph(480, 420, dayHours, prognozaDani[n].windSpeed, "h", "m/s", 250, 130, prognozaDani[n].nData, 1, 7);
  if (n > 0) display.fillTriangle(50, 10, 50, 40, 20, 25, BLACK);
  if (n < 4) display.fillTriangle(750, 10, 750, 40, 780, 25, BLACK);
  display.fillTriangle(770, 570, 790, 570, 780, 580, BLACK);

  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 796, 3, BLACK);
  display.setCursor(200, 35);
  display.print("Vremenska prognoza za ");
  display.print(epochToHuman(forecastList.forecast[start].timestamp).tm_mday, DEC);
  display.print('.');
  display.print(epochToHuman(forecastList.forecast[start].timestamp).tm_mon + 1, DEC);
  display.print(".,");
  display.print(DOW[epochToHuman(forecastList.forecast[start].timestamp).tm_wday]);
   if (fullUpdate)
   {
     display.display();
   }
   else
   {
     display.partialUpdate();
  }
}

void drawGraph(int x, int y, char *dx, float *dy, char *tx, char *ty, int sizex, int sizey, char n, char decimal, char yelements)
{
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
  for (int i = 0; i < n - 1; i++)
  {
    x1 = i * stepX + x + (digits * 6);
    x2 = (i + 1) * stepX + x + (digits * 6);
    y1 = map2(dy[i], minValueY, maxValueY, y + sizey, y);
    y2 = map2(dy[i + 1], minValueY, maxValueY, y + sizey, y);
    display.drawLine(x1, y1, x2, y2, BLACK);
    display.fillCircle(x1, y1, 2, BLACK);
  }
  display.fillCircle(x2, y2, 2, BLACK);

  stepYGraph = sizey / (yelements + 1);
  for (int i = 0; i < yelements + 2; i++)
  {
    for (int j = 0; j < n; j++)
    {
      if (i == 0)
      {
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

double map2(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
