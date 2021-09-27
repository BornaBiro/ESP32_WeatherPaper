// Struct used for stroring data from sensor (indoor unit)
struct sensorData{
  float temp = 0;
  float humidity = 0;
  float pressure = 0;
  float co2 = 0;
  float voc = 0;
  uint32_t timeStamp = 0;
}sensor;

// Contains hour, minutes, seconds, ...
struct tm tNow;

typedef struct vremenskaProg {
  float temp[8], tempMax[8], tempMin[8], pressure[8], pressureSea[8], pressureGnd[8], windSpeed[8], windDir[8], humidity[8];
  int id[8];
  int clouds[8];
  float snow[8];
  float rain[8];
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
weatherDay dani[6];
