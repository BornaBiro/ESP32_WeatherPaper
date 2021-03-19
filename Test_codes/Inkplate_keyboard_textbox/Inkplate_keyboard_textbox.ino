#include "Inkplate.h"

Inkplate display(INKPLATE_1BIT);

#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;
#define X0 11
#define Y0 10
#define X1 9
#define Y1 8
#define AX 34
#define AY 35

static const char key[4][12] = {
  {'!', '"', '#', '$', '%', '&', '/', '(', ')', '=', '?', '*'},
  {'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Š', 'Ð'},
  {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'C', 'C', 'Ž'},
  {'>', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 8},
};

static const char key2[4][12] = {
  {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', char(39), '+'},
  {'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'š', 'd'},
  {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'c', 'c', 'ž'},
  {'<', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 8},
};

int t[2];
int ts[2];
int x = -1, y = -1;
int shiftKey = 0;

struct textBoxHandle
{
  static const uint8_t maxSize = 50;
  char text[maxSize + 1];
  uint8_t size = 20;
  uint8_t n = 0;
  uint8_t selected = 0;
  uint8_t fontScale = 1;
  int16_t x = 0;
  int16_t y = 0;
} text;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  display.begin();
  drawKeyboard();
  text.x = 10;
  text.y = 100;
  text.size = 15;
  text.fontScale = 5;
  textBox(&text, 0, -1, -1);
  display.display();
}
char str[250];
void loop() {
  // put your main code here, to run repeatedly:
  initTouch();
  touchSleep();
  if (touchAvailable()) {
    readTouch(ts);
    char tmp[30];
    char c = -1;
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    sprintf(tmp, "%4d, %4d", ts[0], ts[1]);
    display.println(tmp);
    ts[0] = map(ts[0], 2815, 594, 0, 799);
    ts[1] = map(ts[1], 2591, 372, 0, 599);
    if (ts[1] > 380 && ts[1] < 554) {
      x = (ts[0]) / 66.67;
      y = (ts[1] - 400) / 37.5;
      c = shiftKey ? key[y][x] : key2[y][x];
    }
    sprintf(tmp, "%3d, %3d\n%3d, %3d", ts[0], ts[1], x, y);
    display.print(tmp);
    if (ts[0] > 345 && ts[0] < 455 && ts[1] > 560 && ts[1] < 600) {
      c = ' ';
    }

    if (ts[0] > 540 && ts[0] < 650 && ts[1] > 560 && ts[1] < 600) {
      shiftKey = (shiftKey + 1) & 1;
      drawKeyboard();
    }
    //textBox(10, 100, 20, str, c, 3, ts[0], ts[1]);
    textBox(&text, c, ts[0], ts[1]);
    //unsigned long t1, t2;
    //t1 = millis();
    display.partialUpdate(true);
    //t2 = millis();
    //Serial.println(t2 - t1);
  }
}

void drawX(int x, int y) {
  display.drawFastHLine(x - 4, y, 9, BLACK);
  display.drawFastVLine(x, y - 4, 9, BLACK);
}

void drawKeyboard() {
  display.setTextSize(3);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 12; j++) {
      display.setCursor(33 + j * 66, 400 + 40 * i);
      display.print(shiftKey ? key[i][j] : key2[i][j]);
    }
  }
  display.setCursor(355, 560);
  display.print("SPACE");
  display.setCursor(550, 560);
  display.print("SHIFT");
}

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
  mcp.pullUp(Y1, HIGH);
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

void textBox(struct textBoxHandle *s, char _c, int _tsx, int _tsy)
{
  int k = 0;
  int _h = (s->fontScale * 8) + 4;
  int _w = (s->fontScale * 6 * s->size) + 4;
  int _startPos = 0;
  int i = 0;

  //Alter string with new char
  //Check if its new char
  if (_c != -1)
  {
    //If you need to add new char, check if is valid char, if there is room inside string for new char and if selected place is vaild (if its not out of string)
    //If it's everything ok, put new char in selected place, increment number of chars in string (variable n in struct) and selected place in string
    if (_c > 31 && _c < 127 && s->n < s->maxSize && s->selected <= s->n)
    {
      memmove(s->text + s->selected + 1, s->text + s->selected, s->n - s->selected);
      s->text[s->selected++] = _c;
      s->n++;
    }
    //If you need to remove char, on selected place move whole string from that place to end of string to one place to the left
    else if (_c == 8) //Clean string by moving rest of array
    {
      if (s->n > 0)
      {
        memmove(s->text + s->selected - 1, s->text + s->selected, s->n - s->selected);
        if (s->selected > 0) s->selected--;
        s->n--;
        s->text[s->n] = 0;
      }
    }
  }

  //Redraw textbox on screen. Only 5x7 fonts are allowed in textbox for now!
  display.setFont(NULL);
  display.setTextSize(s->fontScale);
  display.drawRect(s->x, s->y, _w, _h, BLACK);                    //Make a text box border
  display.fillRect(s->x + 1, s->y + 1, _w - 2, _h - 2, WHITE);    //Clear prevoius text
  display.setCursor(s->x + 3, s->y + 2);                          //Set cursor on vaild position (start of the textbox)

  //Calculate from what element string should be printed
  if (s->n > s->size)
  {
    _startPos = s->n - s->size;
  }

  //Start printing string
  for (i = 0; (s->text[i + _startPos] != 0) && (i < s->size); i++)
  {
    display.write(s->text[i + _startPos]);
  }

  //Check if there is touch on textbox. If it is, calculate new selected index of string where all new chars will be inserted.
  if (_tsx > s->x && _tsx < (s->x + _w) && _tsy > s->y && _tsy < (s->y + _h))
  {
    int j = (_tsx - s->x) / 6 / s->fontScale;
    s->selected = j + _startPos;
    if(s->selected > s->n) s->selected = s->n;
  }

  //Draw cursor on screen
  display.drawFastVLine(((s->selected - _startPos) * 6 * s->fontScale) + s->x + 1, s->y + 2, s->fontScale * 8, BLACK);
  //You can do this way, because, you will or alter the string by adding new letters or change cursor position, but not both in the same time!
}
