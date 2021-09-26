#include <SPI.h>
#include "TSC2046E.h"

SPIClass *mySpi = new SPIClass(HSPI);
TSC2046E ts;
void setup() {
  Serial.begin(115200);
  mySpi->begin(14, 12, 13, 15);
  ts.begin(mySpi, 15, 19);
  ts.calibrate(802, 3400, 3566, 379, 0, 799, 0, 599);
}

void loop() {
  char temp[50];
  int x, y;
  if (ts.available(&x, &y))
  {
    sprintf(temp, "X:%4d Y:%4d", x, y);
    Serial.println(temp);
    delay(50); 
  }
}
