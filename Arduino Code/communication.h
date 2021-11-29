#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "RF24_Inkplate.h"
#include "structs.h"
#include "PCF85063.h"

#define WAKEUP_INTERVAL         10
#define COMMUNICATION_INDDOR    0
#define COMMUNICATION_OUTDDOR   1

class communication
{
    public:
    communication();
    void init(RF24_Inkplate * _rf, Inkplate *_dPtr, pcf85063 *_rtcPtr);
    void setupCommunication();
    uint8_t sync(struct syncStructHandle *_s);
    uint8_t getData(struct syncStructHandle *_s, struct measruementHandle *_h);
    uint8_t saveDataToSD(struct sensorData *_s, struct measruementHandle* _d);
    int16_t getNumberOfEntries(time_t _epoch, uint8_t _indoor);
    uint8_t getIndoorDataFromSD(time_t _epoch, int16_t _n, struct sensorData* _d);
    uint8_t getOutdoorDataFromSD(time_t _epoch, int16_t _n, struct measruementHandle* _d);
    void getFileNameFromEpoch(char *_s, time_t _epoch);
    void getFolderPath(char *_s, time_t _epoch, uint8_t _indoor);

    struct syncStructHandle makeSyncStruct();
    time_t newWakeupTime(time_t _current);

    private:
    RF24_Inkplate *_myRf;
    Inkplate *_d;
    pcf85063 *_rtc;
    SdFat *_sd;

    const char *_folderPath = "/ESP32WeatherPaper/%s/%04d/";
    const char *_folderIndoor = "Indoor";
    const char *_folderOutdoor = "Outdoor";
    const char *_filenameStr = "%02d_%02d_%04d.csv";
    const char *_outdoorDataHeader = "No.;EPOCH;DATE[DD.MM.YYYY.];TIME[HH:MM:SS];Battery[V];Temp[C];TempSoil[C];Humidity[%];Pressure[hPa];Light[lux];UV[index];Solar energy[J/cm^2];Solar Enegry[W/^2];Wind speed[m/s];Wind direction[deg];Rain[mm]";
    const char *_outdoorDataEntry = "%d.;%ld;%02d.%02d.%04d.;%02d:%02d:%02d;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%.6f;%d;%.6f;";
    const char *_outdoorDataEntrySD = "%d.;%ld;%d.%d.%d.;%d:%d:%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%d;%f;";
    const char *_indoorDataHeader = "No.;EPOCH;DATE[DD.MM.YYYY.];TIME[HH:MM:SS];Battery[V];Temp[C];Humidity[%];Pressure[hPa];eCO2[ppm];TVOC[ppb];RAW H2;RAW Ethanol";
    const char *_indoorDataEntry = "%d.;%ld;%02d.%02d.%04d.;%02d:%02d:%02d;%.6f;%.6f;%.6f;%.6f;%d;%d;%d;%d";
    const char *_indoorDataEntrySD = "%d.;%ld;%d.%d.%d.;%d:%d:%d;%f;%f;%f;%f;%d;%d;%d;%d";
    uint64_t addr[2] = {0x65646F4E31, 0x65646F4E32};
};

#endif
