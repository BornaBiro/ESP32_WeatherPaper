// Includes for communication and periph.
#include <Wire.h>
#include <SPI.h>
#include "driver/rtc_io.h"
#include "timeAndDate.h"

// WiFi and Internet includes
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiUdp.h>
#include "Weather.h"

// Display includes (driver, icons, fonts, etc)
#include "Inkplate.h"
#include "GUI.h"
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
#include "EEPROM.h"
#include "PCF85063.h"
#include "communication.h"
#include "RF24_Inkplate.h"

//.ttf to .h: https://rop.nl/truetype2gfx/
//Open Source fonts https://fonts.google.com/
#define DISPLAY_FONT &Neucha_Regular17pt7b
#define DISPLAY_FONT_SMALL &RobotoCondensed_Regular6pt7b

#define NEW_INKPLATE

// Objects / constructors
Inkplate display(INKPLATE_1BIT);
TSC2046E_Inkplate ts;
SPIClass *mySpi = NULL;
Adafruit_BME280 bme;
OWMWeather owm;
GUI gui;
pcf85063 rtc;
communication rf;
RF24_Inkplate radio(14, 15, 2000000);

// Contains hour, minutes, seconds, ...
struct tm tNow;
//timeval tm;
//const timeval* tmPtr = &tm;
time_t myTime;
time_t timeToWake;
int timeOffset;

const char ssid[] = "Biro_WLAN";
const char pass[] = "CaVex250_H2sH11";

uint8_t modeSelect = 0;
uint8_t forcedRef = 0;
uint8_t selectedDay = 0;
uint8_t selectedGraph = 0;

struct sensorData sensor;
struct currentWeatherHandle currentWeather;
struct forecastListHandle forecastList;
struct forecastDisplayHandle forecastDisplay[7];
struct oneCallApiHandle oneCallApi;
struct syncStructHandle syncStruct;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  display.begin();

  #ifdef NEW_INKPLATE
  // Set output mode of MCP INT pin
  display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, 0, 0, 1);

  // Set touchscreen controller INT pin
  display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 13, INPUT_PULLUP);
  display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 13, FALLING);

  // Set default pin state on touchscreen controller CS pin
  display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 10, OUTPUT);
  display.digitalWriteInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 10, HIGH);

  // Set default state of nRF24l01 pins
  display.digitalWriteInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 14, HIGH);
  display.digitalWriteInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 15, HIGH);

  // Set RTC INT PIN
  display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 12, INPUT_PULLUP);
  display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 12, FALLING);

  // Clear all INT flags on MCP
  display.getINTstateInternal(MCP23017_INT_ADDR, display.mcpRegsInt);
  #endif

  gui.init(&display);
  rf.init(&radio, &display, &rtc);
  rtc.RTCinit();

  mySpi = display.getSPIptr();
  mySpi->begin(14, 12, 13, 15);
  #ifdef NEW_INKPLATE
  ts.begin(mySpi, &display, 10, 13);
  #elif
  ts.begin(mySpi, &display, 13, 14);
  #endif
  ts.calibrate(800, 3420, 3553, 317, 0, 799, 0, 599);

  if (!radio.begin(mySpi, &display))
  {
    Serial.println("Radio init ERROR!");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setHostname("WeatherPaper");
  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X16, // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X16, // humidity
                  Adafruit_BME280::FILTER_X16   );
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
  updateWeatherData();
  if (!rtc.isClockSet() && readNTP(&myTime))
  {
    //settimeofday(tmPtr, NULL);
    rtc.setClock(myTime);
    //gettimeofday(&tm, NULL);
  }
  tNow = epochToHuman(rtc.getClock());

  rf.setupCommunication();
  if (rf.sync(&syncStruct))
  {
    display.clearDisplay();
    display.print("sync succ!");
    display.display();
  }

  timeToWake = 1200 + myTime;
  gui.drawMainScreen(&sensor, &currentWeather, &forecastList, forecastDisplay, &oneCallApi, &tNow);
  esp_wifi_stop();
  btStop();
}

void loop()
{
  int tsX, tsY;
  display.digitalWriteMCP(15, HIGH);
  rtc_gpio_isolate(GPIO_NUM_12);
  //gettimeofday(&tm, NULL);
  //esp_sleep_enable_timer_wakeup((timeToWake - tm.tv_sec) * 1000000ULL); //20 minuta
  rtc.setAlarm(timeToWake);
  #ifdef NEW_INKPLATE
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);
  #elif
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
  #endif
  esp_light_sleep_start();
  uint16_t intPins = display.getINTInternal(MCP23017_INT_ADDR, display.mcpRegsInt);
  display.getINTstateInternal(MCP23017_INT_ADDR, display.mcpRegsInt);
  Serial.println("Woken up");
  Serial.flush();
  if ((intPins & (1 << 13)) && ts.available(&tsX, &tsY))
  {
    Serial.println("TS detected");
    switch (modeSelect)
    {
      case 0:
        if (touchArea(tsX, tsY, 0, 410, 800, 190))
        {
          selectedDay = tsX / 160;
          drawDays(selectedDay, true);
          modeSelect = 1;
        }

        if (touchArea(tsX, tsY, 770, 0, 30, 30))
        {
          display.setFont(DISPLAY_FONT);
          display.setCursor(610, 35);
          writeInfoBox(350, "Osvjez. podataka, pricekajte");
          display.partialUpdate();
          forcedRef = 1;
        }
        break;

      case 1:
        if (touchArea(tsX, tsY, 10, 570, 30, 30))
        {
          gui.drawMainScreen(&sensor, &currentWeather, &forecastList, forecastDisplay, &oneCallApi, &tNow);
          modeSelect = 0;
          selectedDay = 0;
          selectedGraph = 0;
        }

        if (touchArea(tsX, tsY, 0, 0, 75, 75) && selectedDay > 0)
        {
          selectedDay--;
          drawDays(selectedDay, false);
          modeSelect = 1;
        }

        if (touchArea(tsX, tsY, 730, 0, 70, 50) && selectedDay < 4)
        {
          selectedDay++;
          drawDays(selectedDay, false);
          modeSelect = 1;
        }
        if (touchArea(tsX, tsY, 50, 550, 700, 150))
        {

          selectedGraph = (tsX - 50) / 70;
          drawSelectedGraph(selectedGraph, selectedDay);
          display.partialUpdate();
        }

        if (touchArea(tsX, tsY, 0, 70, 800, 70))
        {
          int selectedElement = tsX / 100;
          int _yPos = 240;
          const int _fontScale = 2;
          const int _fontYSize = 14 * _fontScale;
          if (selectedElement < (forecastList.startElement[selectedDay + 2 - forecastList.shiftDay] - forecastList.startElement[selectedDay + 1 - forecastList.shiftDay]))
          {
            int element = forecastList.startElement[selectedDay + 1 - forecastList.shiftDay] + selectedElement;
            display.fillRect(198, 198, 404, 304, BLACK);
            display.fillRect(200, 200, 400, 300, WHITE);
            display.setFont(DISPLAY_FONT_SMALL);
            display.setTextSize(_fontScale);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print(forecastList.forecast[element].weatherDesc);
            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Temp.: ");
            display.print(forecastList.forecast[element].minTemp, 1);
            display.print("[Min] / ");
            display.print(forecastList.forecast[element].temp, 1);
            display.print(" / ");
            display.print(forecastList.forecast[element].maxTemp, 1);
            display.print("[Max] C");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Tlak zraka: ");
            display.print(forecastList.forecast[element].pressureGnd);
            display.print("hPa");

            display.setCursor(220, _yPos);  _yPos += _fontYSize;
            display.print("Brzina vjetra: ");
            display.print(forecastList.forecast[element].windSpeed, 1);
            display.print('/');
            display.print(forecastList.forecast[element].windGust, 1);
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

  if ((rtc.checkAlarmFlag() && (intPins & (1 << 12))) || forcedRef)
  {
    rtc.clearAlarm();
    forcedRef = 0;
    modeSelect = 0;
    selectedDay = 0;
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
      display.print('.');
      display.partialUpdate();
    }

    readSensor(&sensor);
    if (WiFi.status() == WL_CONNECTED) updateWeatherData();
    //gettimeofday(&tm, NULL);
    myTime = rtc.getClock();
    tNow = epochToHuman(myTime);
    timeToWake = 1200 + myTime;
    rtc.setAlarm(timeToWake);
    gui.drawMainScreen(&sensor, &currentWeather, &forecastList, forecastDisplay, &oneCallApi, &tNow);
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
  //#ifndef NEW_INKPLATE  
  bme.takeForcedMeasurement();
  _s->temp = bme.readTemperature();
  _s->humidity = bme.readHumidity();
  _s->pressure = bme.readPressure() / 100.0F;
  //#endif
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

void updateWeatherData()
{
  owm.getCurrentWeather("http://api.openweathermap.org/data/2.5/weather?&lat=45.684502&lon=18.402357&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657", &currentWeather);
  owm.getForecastWeather("http://api.openweathermap.org/data/2.5/forecast?lat=45.684502&lon=18.402357&units=metric&lang=hr&APPID=b1d3e9077193732d4b5e3e2c4c036657", &forecastList, forecastDisplay);
  owm.oneCall("http://api.openweathermap.org/data/2.5/onecall?lat=45.684502&lon=18.402357&units=metric&lang=hr&exclude=current,minutely,hourly,daily&appid=b1d3e9077193732d4b5e3e2c4c036657", &oneCallApi);  //Upozorenja
  //owm.oneCall("http://api.openweathermap.org/data/2.5/onecall?lat=45.267136&lon=19.833549&units=metric&lang=hr&exclude=current,minutely,hourly,daily&appid=b1d3e9077193732d4b5e3e2c4c036657", &oneCallApi);
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
    display.drawBitmap(xPos * 95 + 45, 70, gui.weatherIcon(atoi(forecastList.forecast[i].weatherIcon)), 50, 50, BLACK);
    display.setCursor(xPos * 95 + 30, 140);
    display.print(epochToHuman(forecastList.forecast[i].timestamp).tm_hour, DEC);
    display.print(":00h");
    xPos++;
  }
  display.setTextSize(1);
  if (n > 0) display.fillTriangle(50, 10, 50, 40, 20, 25, BLACK);
  if (n < 4) display.fillTriangle(750, 10, 750, 40, 780, 25, BLACK);
  display.fillTriangle(10, 580, 30, 570, 30, 590, BLACK);

  display.setFont(DISPLAY_FONT);
  display.fillRect(4, 50, 792, 3, BLACK);
  display.setCursor(200, 35);
  display.print("Vremenska prognoza za ");
  display.print(epochToHuman(forecastList.forecast[start].timestamp).tm_mday, DEC);
  display.print('.');
  display.print(epochToHuman(forecastList.forecast[start].timestamp).tm_mon + 1, DEC);
  display.print(".,");
  display.print(DOW[epochToHuman(forecastList.forecast[start].timestamp).tm_wday]);

  for (int i = 0; i < 10; i++)
  {
    const uint8_t *iconList[] = {iconTemp, iconTlak, iconVlaga, iconVjetar, iconWindDir, iconClouds, iconVisability, iconRainDrop, iconSnowflake, iconProbability};
    display.drawBitmap((i * 70) + 50, 550, iconList[i], 40, 40, BLACK);
  }
  drawSelectedGraph(selectedGraph, n);

  if (fullUpdate)
  {
    display.display();
  }
  else
  {
    display.partialUpdate();
  }
}

void drawSelectedGraph(uint8_t _graph, uint8_t _day)
{
  uint8_t start = forecastList.startElement[-forecastList.shiftDay + _day + 1];
  uint8_t end = forecastList.startElement[-forecastList.shiftDay + _day + 2];
  uint8_t _n = end - start;
  display.fillRect(4, 540, 792, 3, BLACK);
  display.fillRect((selectedGraph * 70) + 35, 540, 70, 3, WHITE);
  display.setFont(NULL);
  display.setTextSize(2);
  display.fillRect(0, 170, 800, 360, WHITE);
  switch(_graph)
  {
    case 0:
      gui.printAlignText("Temperatura zraka [C]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].temp), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_FLOAT, GRAPHSTYLE_LINE);
      break;
    case 1:
      gui.printAlignText("Tlak zraka [hPa]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].pressureGnd), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
      break;
    case 2:
      gui.printAlignText("Relat. vlaznost zraka [%]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].humidity), _n, sizeof(struct forecastWeatherHandle), 10, DATATYPE_UINT8_T, GRAPHSTYLE_LINE, 0, 100);
      break;
    case 3:
      gui.printAlignText("Brzina vjetra [m/s]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].windSpeed), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_FLOAT, GRAPHSTYLE_LINE, 0);
      break;
    case 4:
      gui.printAlignText("Smjer vjetra [stupnjevi]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].windDir), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_UINT16_T, GRAPHSTYLE_DOT, 0, 360);
      break;
    case 5:
      gui.printAlignText("Kolicina naoblake [%]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].clouds), _n, sizeof(struct forecastWeatherHandle), 10, DATATYPE_UINT8_T, GRAPHSTYLE_COLUMN, 0, 100);
      break;
    case 6:
      gui.printAlignText("Vidljivost [m]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].visibility), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_UINT16_T, GRAPHSTYLE_LINE);
      break;
    case 7:
      gui.printAlignText("Kolicina padalina [mm]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].rain), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN, 0);
      break;
    case 8:
      gui.printAlignText("Kolicina snijega [mm]" , 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].snow), _n, sizeof(struct forecastWeatherHandle), 12, DATATYPE_FLOAT, GRAPHSTYLE_COLUMN, 0);
      break;
    case 9:
      gui.printAlignText("Vjerojatnost oborina [%]", 400, 195, ALIGMENT_CENTERBOT);
      gui.drawGraph(130, 200, 600, 300, &(forecastList.forecast[start].timestamp), &(forecastList.forecast[start].probability), _n, sizeof(struct forecastWeatherHandle), 10, DATATYPE_UINT8_T, GRAPHSTYLE_COLUMN, 0, 100);
      break;
    }
}

uint8_t touchArea(int16_t tsX, int16_t tsY, int16_t x, int16_t y, int16_t w, int16_t h)
{
  if ((tsX >= x) && (tsY >= y) && (tsX < (x + w)) && (tsY < (y + h))) return 1;
  return 0;
}

void drawGraph(int x, int y, char *dx, float *dy, char *tx, char *ty, int sizex, int sizey, char n, char decimal, char yelements)
{
  char digits = 0;
  float maxValueX = dx[0], minValueX = dx[0];
  float maxValueY = dy[0], minValueY = dy[0];
  int stepX;
  float stepY;
  int stepYGraph;
  for (int i = 0; i < n; i++)
  {
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
    y1 = gui.map2(dy[i], minValueY, maxValueY, y + sizey, y);
    y2 = gui.map2(dy[i + 1], minValueY, maxValueY, y + sizey, y);
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
