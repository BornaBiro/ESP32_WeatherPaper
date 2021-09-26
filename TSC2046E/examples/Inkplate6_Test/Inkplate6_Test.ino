#include <SPI.h>
#include "Inkplate.h"
#include "TSC2046E_Inkplate.h"

Inkplate display(INKPLATE_1BIT);
TSC2046E_Inkplate ts;
SPIClass *mySpi = NULL;
void setup() {
  Serial.begin(115200);
  display.begin();
  mySpi = display.getSPIptr();
  mySpi->begin(14, 12, 13, 15);
  ts.begin(mySpi, &display, 13, 14);
  ts.calibrate(800, 3420, 3553, 317, 0, 799, 0, 599);
  display.display();
}

void loop() {
  char temp[50];
  int x, y;
  if (ts.available(&x, &y))
  {
    sprintf(temp, "X:%4d Y:%4d", x, y);
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(4);
    display.setCursor(100, 100);
    display.println(temp);
    display.partialUpdate(false, true);
    delay(50);
  }
}