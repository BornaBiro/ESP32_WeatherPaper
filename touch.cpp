#include "touch.h"

extern Adafruit_MCP23017 mcp;

void initTouch() {
  //mcp.begin(0);
  pinMode(AY, INPUT);
  pinMode(AX, INPUT);
}

bool touchAvailable() {
  mcp.pinMode(X0, OUTPUT);
  mcp.pullUp(Y1, HIGH);
  mcp.pinMode(X1, INPUT);
  mcp.pinMode(Y0, INPUT);
  mcp.digitalWrite(X0, LOW);
  delay(1);
  return !mcp.digitalRead(Y1);
}

void touchSleep() {
  mcp.pullUp(Y1, LOW);
  mcp.pinMode(X0, OUTPUT);
  mcp.digitalWrite(X0, LOW);
  mcp.pinMode(Y0, INPUT);
  mcp.pinMode(X1, INPUT);
  mcp.pinMode(Y1, INPUT);
}

bool readTouch(int *t) {
  int z1, i = 0;
  mcp.pinMode(Y0, OUTPUT);
  mcp.pinMode(Y1, OUTPUT);
  mcp.pinMode(X0, INPUT);
  mcp.pinMode(X1, INPUT);
  mcp.digitalWrite(Y0, LOW);
  mcp.digitalWrite(Y1, HIGH);
  delay(1);
  t[1] = analogRead(AX);
  mcp.pinMode(Y0, INPUT);
  mcp.pinMode(Y1, INPUT);
  mcp.pinMode(X0, OUTPUT);
  mcp.pinMode(X1, OUTPUT);
  mcp.digitalWrite(X0, HIGH);
  mcp.digitalWrite(X1, LOW);
  delay(1);
  t[0] = analogRead(AY);
  mcp.pinMode(X0, OUTPUT);
  mcp.pinMode(Y0, OUTPUT);
  mcp.pinMode(X1, INPUT);
  mcp.pinMode(Y1, INPUT);
  mcp.digitalWrite(X0, HIGH);
  mcp.digitalWrite(Y0, LOW);
  delay(1);
  z1 = analogRead(AY);
  if (z1 > 20) {
    return 1;
  }
  return 0;
}
