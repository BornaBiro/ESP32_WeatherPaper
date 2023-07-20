#include "Inkplate.h"

Inkplate display(INKPLATE_1BIT);
Inkplate *_ink = &display;

enum graphDataType
{
  DATATYPE_FLOAT,
  DATATYPE_DOUBLE,
  DATATYPE_UINT8_T,
  DATATYPE_UINT16_T,
  DATATYPE_INT16_T,
  DATATYPE_INT,
};

enum graphStyle
{
  GRAPHSTYLE_LINE,
  GRAPHSTYLE_DOT,
  GRAPHSTYLE_COLUMN
};

enum alignment
{
  ALIGMENT_LEFTBOT,
  ALIGMENT_CENTERBOT,
  ALIGMENT_RIGHTBOT,
  ALIGMENT_LEFT,
  ALIGMENT_CENTER,
  ALIGMENT_RIGHT,
  ALIGMENT_LEFTTOP,
  ALIGMENT_CENTERTOP,
  ALIGMENT_RIGHTTOP
};

struct graphTypedef
{
  int x;    // X position of the graph (0,0 is upper left corner)
  int y;    // Y posotion of the grapf (0,0 is upper left corner)
  int w;    // Width of the graph itself (not counting axis numners)
  int h;    // Heigth of the graph itself (not counting the axis numbers)
  uint32_t *xDataPtr;     // Pointer to the array of the x axis data (epoch data)
  void *yDataPtr;     // Pointer to the array of the y axis data (*xDataPtr and *yDataPtr form a (X,Y) data point.
  int xDataCount;     // How much there is xData in the array
  int yDataCount;     // How much there is yData in the array (x and y should have same amount of data in the array)
  int8_t xDiv;        // How many divisions on x axis
  int8_t yDiv;        // How many divisions on y axis
  int xDataOffset;    // How many bytes there are between two x data in the array
  int yDataOffset;    // How many bytes there are between two y data in the array
  uint32_t xStart;    // First data point of the x data (start time)
  uint32_t xStep;     // Step size of x data (epoch - seconds between y data)
  uint8_t yDataType;  // What kind of data is stored in the array (INT, DOUBLE, FLOAT, CHAR, ....)
  uint8_t style;      // Type of graph (Line, dots, columns).
  double max;
  double min;
};

//double dummyData[] = {23.0, 23.0, 22.9, 22.7, 22.4, 22.0, 21.2, 20.6, 19.4, 20.8, 25.8, 27.8, 30.1};
//uint32_t dummyDataTimes[] = {1689724800, 1689728400, 1689732000, 1689735600, 1689739200, 1689742800, 1689746400, 1689750000, 1689753600, 1689757200, 1689760800, 1689764400, 1689768000};
////uint32_t dummyDataTimes[] = {1689724800, 1689728400, 1689732000, 1689735600, 1689739200, 1689742800, 1689746400, 1689750000, 1689753600, 1689757200, 1689760800, 1689764400, 1689768000, 1689771600, 1689775200, 1689778800, 1689782400, 1689786000, 1689789600, 1689793200, 1689796800, 1689800400};
//struct graphTypedef graphTest
//{
//  .x = 50,
//  .y = 50,
//  .w = 600,
//  .h = 400,
//  .xDataPtr = dummyDataTimes,
//  .yDataPtr = dummyData,
//  .xDataCount = sizeof(dummyDataTimes) / sizeof(uint32_t),
//  .yDataCount = sizeof(dummyData) / sizeof(double),
//  .xDiv = 13,
//  .yDiv = 10,
//  .xDataOffset = sizeof(uint32_t),
//  .yDataOffset = sizeof(double),
//  .xStart = 1689811200,
//  .xStep = 7200,
//  .yDataType = DATATYPE_DOUBLE,
//  .style = GRAPHSTYLE_LINE,
//  .max = sqrt(-1),
//  .min = sqrt(-1),
//};

double temps[] = {20.171135, 20.18186, 20.085335, 20.063885, 19.999535, 20.09606, 19.892282, 19.65633, 19.65633, 19.645605, 19.59198, 19.474005, 19.441828, 19.441828, 19.516905, 19.474005, 19.559805, 19.559805, 19.52763, 19.48473, 19.431103, 19.366753, 19.398928, 19.323853, 19.216602, 19.077175, 18.9592, 18.862673, 18.884123, 18.948475, 19.012825, 19.06645, 19.045, 18.884123, 18.766148, 18.798323, 18.723248, 18.64817, 18.723248, 18.744698, 18.691072, 18.669622, 18.701797, 18.723248, 18.787598, 18.894848, 19.02355, 19.0879, 19.1737, 19.270227, 19.356028, 19.474005, 19.559805, 19.59198, 19.59198, 19.602705, 19.602705, 19.65633, 19.645605, 19.59198, 19.61343, 19.61343, 19.61343, 19.59198, 19.538355, 19.516905, 19.48473, 19.441828, 19.46328, 19.452553, 19.452553, 19.516905, 19.516905, 19.54908, 19.538355, 19.54908, 19.538355, 19.581255, 19.61343, 19.63488, 19.667055, 19.65633, 19.709955, 19.849382, 20.03171, 20.149685, 20.20331, 20.332012, 20.192585, 20.214035, 20.449987, 20.471437, 20.471437, 20.449987, 20.60014, 20.68594, 20.696665, 20.610865, 20.653765, 20.857542, 21.340172, 21.812075, 22.477032, 22.648632, 22.702259, 23.024012, 23.259964, 23.356489, 23.324314, 23.324314, 23.656792, 23.710417, 23.388664, 23.377939, 23.635342, 24.064346, 24.686401, 24.375374, 25.158306, 25.458609, 25.426434, 26.048489, 25.898338, 25.812536, 25.801811, 26.691994, 26.724171, 26.423866, 26.917221, 27.303326, 27.131723, 27.378401, 27.678703, 27.507103, 27.389126, 27.700153, 27.721603, 27.571453, 28.429461, 27.925381, 28.000456, 28.515261, 28.686863, 29.298193, 28.740488, 28.504536, 28.922815, 29.201668, 29.351818, 29.158768, 29.158768, 29.45907, 30.04895, 30.810432, 29.834448, 30.306353, 29.96315, 30.016775, 31.14291, 30.402878, 30.66028, 31.529015, 31.636265, 31.62554, 30.99276, 30.61738, 31.550465, 31.62554, 31.850767, 31.518288, 32.42992, 31.75424, 31.196535, 31.153635, 31.03566, 31.293062, 31.217985, 31.507563, 31.636265, 31.73279, 31.700615, 32.075993, 31.893667, 32.075993, 31.77569, 32.698048, 32.676598, 31.53974, 31.400312, 32.27977, 32.494274, 33.223579, 33.245029, 32.42992, 31.936567, 32.033092, 32.08672, 31.60409, 32.612247, 32.934002, 32.590797, 32.751675, 32.858925, 31.75424, 31.046385, 31.529015, 32.21542, 31.722065, 31.60409, 31.62554, 31.925842, 31.75424, 31.861492, 31.743515, 31.400312, 31.303787, 31.089285, 30.724632, 30.70318, 30.671005, 30.68173, 30.59593, 30.520855, 30.606655, 30.445778, 30.242002, 30.04895, 29.770098, 29.630671, 29.41617, 29.244568, 29.190943, 28.987165, 28.740488, 28.493811, 28.34366, 28.086258, 27.81813, 27.571453, 27.378401, 27.110273, 26.809971, 26.541843, 26.423866};
uint32_t tempEpoch[] = {1689811200, 1689811500, 1689811800, 1689812101, 1689812401, 1689812700, 1689813002, 1689813300, 1689813601, 1689813901, 1689814201, 1689814500, 1689814800, 1689815101, 1689815401, 1689815701, 1689816001, 1689816301, 1689816601, 1689816902, 1689817201, 1689817501, 1689817801, 1689818101, 1689818401, 1689818701, 1689819002, 1689819301, 1689819601, 1689819900, 1689820201, 1689820501, 1689820800, 1689821101, 1689821401, 1689821701, 1689822000, 1689822301, 1689822601, 1689822901, 1689823201, 1689823501, 1689823801, 1689824100, 1689824400, 1689824700, 1689825001, 1689825300, 1689825601, 1689825902, 1689826200, 1689826500, 1689826800, 1689827101, 1689827402, 1689827701, 1689828001, 1689828301, 1689828601, 1689828901, 1689829200, 1689829501, 1689829801, 1689830102, 1689830401, 1689830701, 1689831001, 1689831301, 1689831601, 1689831901, 1689832201, 1689832501, 1689832801, 1689833101, 1689833401, 1689833701, 1689834001, 1689834300, 1689834601, 1689834901, 1689835200, 1689835500, 1689835801, 1689836101, 1689836401, 1689836701, 1689837001, 1689837301, 1689837600, 1689837900, 1689838201, 1689838501, 1689838800, 1689839101, 1689839400, 1689839701, 1689840001, 1689840301, 1689840601, 1689840901, 1689841201, 1689841500, 1689841800, 1689842101, 1689842401, 1689842701, 1689843000, 1689843300, 1689843600, 1689843900, 1689844200, 1689844500, 1689844800, 1689845100, 1689845400, 1689845700, 1689846000, 1689846300, 1689846601, 1689846901, 1689847200, 1689847501, 1689847800, 1689848100, 1689848400, 1689848700, 1689849000, 1689849300, 1689849600, 1689849900, 1689850200, 1689850500, 1689850800, 1689851100, 1689851400, 1689851700, 1689852000, 1689852300, 1689852600, 1689852900, 1689853200, 1689853500, 1689853800, 1689854100, 1689854400, 1689854700, 1689855000, 1689855300, 1689855600, 1689855900, 1689856200, 1689856500, 1689856800, 1689857100, 1689857400, 1689857700, 1689858000, 1689858300, 1689858601, 1689858901, 1689859200, 1689859500, 1689859800, 1689860100, 1689860400, 1689860701, 1689861001, 1689861301, 1689861600, 1689861900, 1689862201, 1689862500, 1689862801, 1689863100, 1689863400, 1689863700, 1689864000, 1689864300, 1689864601, 1689864900, 1689865201, 1689865500, 1689865801, 1689866100, 1689866400, 1689866701, 1689867000, 1689867300, 1689867601, 1689867901, 1689868200, 1689868501, 1689868800, 1689869100, 1689869401, 1689869700, 1689870000, 1689870300, 1689870600, 1689870900, 1689871200, 1689871500, 1689871801, 1689872101, 1689872401, 1689872701, 1689873000, 1689873300, 1689873600, 1689873900, 1689874200, 1689874500, 1689874800, 1689875100, 1689875400, 1689875701, 1689876000, 1689876300, 1689876600, 1689876900, 1689877200, 1689877500, 1689877800, 1689878100, 1689878400, 1689878700, 1689879000, 1689879301, 1689879601, 1689879900, 1689880201, 1689880501, 1689880800, 1689881100, 1689881400, 1689881701, 1689882000, 1689882301, 1689882602, 1689882900, 1689883200, 1689883501, 1689883800, 1689884101, };

struct graphTypedef graph
{
  .x = 50,
  .y = 50,
  .w = 600,
  .h = 400,
  .xDataPtr = tempEpoch,
  .yDataPtr = temps,
  .xDataCount = sizeof(tempEpoch) / sizeof(uint32_t),
  .yDataCount = sizeof(temps) / sizeof(double),
  .xDiv = 13,
  .yDiv = 10,
  .xDataOffset = sizeof(uint32_t),
  .yDataOffset = sizeof(double),
  .xStart = tempEpoch[0],
  .xStep = 7200,
  .yDataType = DATATYPE_DOUBLE,
  .style = GRAPHSTYLE_LINE,
  .max = sqrt(-1),
  .min = sqrt(-1),
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  display.begin();
  display.display();

  drawGraphNew(graph);
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void drawGraph(struct graphTypedef _graphInfo)
{
  // First copy data into local array
  double *_intArray;
  _intArray = (double*)ps_malloc(_graphInfo.xDataCount * sizeof(double));
  if (_intArray == NULL) return;

  // Fill the array with the data
  for (int i = 0; i < _graphInfo.xDataCount; i++)
  {
    switch (_graphInfo.yDataType)
    {
      case DATATYPE_FLOAT:
        _intArray[i] = (*(float*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_DOUBLE:
        _intArray[i] = (*(double*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_UINT8_T:
        _intArray[i] = (*((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_UINT16_T:
        _intArray[i] = (*(uint16_t*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_INT16_T:
        _intArray[i] = (*(int16_t*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_INT:
        _intArray[i] = (*(int*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
    }
  }

  // Now draw the y and x axis
  _ink->fillRect(_graphInfo.x, _graphInfo.y, 2, _graphInfo.h, BLACK);
  _ink->fillRect(_graphInfo.x, _graphInfo.y + _graphInfo.h, _graphInfo.w, 2, BLACK);
  struct tm *_time;

  // Draw time data on x axis
  uint8_t _xSpacing = _graphInfo.w / _graphInfo.xDiv;
  uint8_t _ySpacing = _graphInfo.h / _graphInfo.yDiv;
  
  for (int i = 0; i < _graphInfo.xDataCount; i++)
  {
    char temp[10];
    time_t _epoch = (*(time_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * i)));
    _time = localtime((const time_t*)&_epoch);
    _ink->drawFastVLine(_graphInfo.x + (i * _xSpacing) + (_xSpacing / 2), _graphInfo.y, _graphInfo.h + 3, BLACK);
    sprintf(temp, "%d:%02d", _time->tm_hour, _time->tm_min);
    printAlignText(temp, _graphInfo.x + (i * _xSpacing) + (_xSpacing / 2), _graphInfo.y + _graphInfo.h + 10, ALIGMENT_CENTERTOP);
  }

  // Find max and min value
  float _dataMax = _intArray[0];
  float _dataMin = _dataMax;
  for (int i = 1; i < _graphInfo.xDataCount; i++)
  {
    if (isnan(_graphInfo.max)) if (_intArray[i] > _dataMax) _dataMax = _intArray[i];
    if (isnan(_graphInfo.min)) if (_intArray[i] < _dataMin) _dataMin = _intArray[i];
  }
  if (_dataMin == _dataMax) _dataMax = _dataMin + 1;
  if (!isnan(_graphInfo.max)) _dataMax = _graphInfo.max;
  if (!isnan(_graphInfo.min)) _dataMin = _graphInfo.min;

  // Now find data step
  float _yStep = (_dataMax - _dataMin) / _graphInfo.yDiv;

  // Draw data on y axis
  for (int i = 0; i < (_graphInfo.yDiv + 1); i++)
  {
    char temp[20];
    sprintf(temp, "%.1f", _dataMax - (i * _yStep));
    //_d->setCursor(_x - 30, _y + (_ySpacing * i) - 3);
    _ink->drawFastHLine(_graphInfo.x - 2, _graphInfo.y + (_ySpacing * i), _graphInfo.w, BLACK);
    //_d->print(temp);
    printAlignText(temp, _graphInfo.x - 5, _graphInfo.y + (_ySpacing * i), ALIGMENT_LEFT);

  }

  // Now draw all the data on graph
  switch (_graphInfo.style)
  {
    case GRAPHSTYLE_LINE:
      for (int i = 0; i < (_graphInfo.xDataCount - 1); i++)
      {
        _ink->drawThickLine(_graphInfo.x + (i * _xSpacing) + (_xSpacing / 2), (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y), _graphInfo.x + ((i + 1) * _xSpacing) + (_xSpacing / 2), (int16_t)map2(_intArray[i + 1], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y), BLACK, 2);
      }
      break;
    case GRAPHSTYLE_COLUMN:
      for (int i = 0; i < _graphInfo.xDataCount; i++)
      {
        int16_t _columnY = (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y);
        int16_t _columnH = _graphInfo.y + _graphInfo.h - _columnY;
        _ink->fillRect(_graphInfo.x + (i * _xSpacing) + 4, _columnY, _xSpacing - 8, _columnH, BLACK);
      }
      break;
    case GRAPHSTYLE_DOT:
      for (int i = 0; i < _graphInfo.xDataCount; i++)
      {
        _ink->fillCircle(_graphInfo.x + (i * _xSpacing) + (_xSpacing / 2) , (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y), 5, BLACK);
      }
      break;
  }
  free(_intArray);
}

void printAlignText(const char *text, int16_t x, int16_t y, enum alignment align)
{
  int16_t x1, y1;
  uint16_t w, h;
  _ink->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  switch (align)
  {
    case ALIGMENT_CENTERBOT:
    case ALIGMENT_CENTER:
    case ALIGMENT_CENTERTOP:
      x1 = x - w / 2;
      break;
    case ALIGMENT_RIGHTBOT:
    case ALIGMENT_RIGHT:
    case ALIGMENT_RIGHTTOP:
      x1 = x;
      break;
    case ALIGMENT_LEFTBOT:
    case ALIGMENT_LEFT:
    case ALIGMENT_LEFTTOP:
    default:
      x1 = x - w;
      break;
  }
  switch (align)
  {
    case ALIGMENT_CENTERBOT:
    case ALIGMENT_RIGHTBOT:
    case ALIGMENT_LEFTBOT:
      y1 = y - h;
      break;
    case ALIGMENT_CENTER:
    case ALIGMENT_RIGHT:
    case ALIGMENT_LEFT:
      y1 = y - (h / 2);
      break;
    case ALIGMENT_CENTERTOP:
    case ALIGMENT_LEFTTOP:
    case ALIGMENT_RIGHTTOP:
    default:
      y1 = y;
      break;
  }
  _ink->setCursor(x1, y1);
  _ink->print(text);
}

double map2(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void drawGraphNew(struct graphTypedef _graphInfo)
{ 
  // First copy data into local array
  double *_intArray;
  _intArray = (double*)ps_malloc(_graphInfo.xDataCount * sizeof(double));
  if (_intArray == NULL) return;

  // Fill the array with the data
  for (int i = 0; i < _graphInfo.xDataCount; i++)
  {
    switch (_graphInfo.yDataType)
    {
      case DATATYPE_FLOAT:
        _intArray[i] = (*(float*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_DOUBLE:
        _intArray[i] = (*(double*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_UINT8_T:
        _intArray[i] = (*((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_UINT16_T:
        _intArray[i] = (*(uint16_t*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_INT16_T:
        _intArray[i] = (*(int16_t*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
      case DATATYPE_INT:
        _intArray[i] = (*(int*)((uint8_t*)(_graphInfo.yDataPtr) + (_graphInfo.yDataOffset * i)));
        break;
    }
  }

  // Now draw the y and x axis
  _ink->fillRect(_graphInfo.x, _graphInfo.y, 2, _graphInfo.h, BLACK);
  _ink->fillRect(_graphInfo.x, _graphInfo.y + _graphInfo.h, _graphInfo.w, 2, BLACK);
  struct tm *_time;

  // Draw time data on x axis
  uint8_t _xSpacing = _graphInfo.w / _graphInfo.xDiv;
  uint8_t _ySpacing = _graphInfo.h / _graphInfo.yDiv;

  // Calculate the first and last timestamp allowed to be printed. Add 1 minute for the last and first entry due RTC inaccuracy.
  // This is needed to know what data will be printed on the graph.
  uint32_t _firstEpoch = _graphInfo.xStart - 59;
  uint32_t _lastEpoch = (_graphInfo.xStart) + (_graphInfo.xStep * (_graphInfo.xDiv - 1)) + 59;

  // Print out the times on x axis. Time are calculated from start time, step time and number of divisions on x axis.
  for (int i = 0; i < _graphInfo.xDiv; i++)
  {
    char temp[10];
    time_t _epoch = (_graphInfo.xStart) + (_graphInfo.xStep * i);
    _time = localtime((const time_t*)&_epoch);
    _ink->drawFastVLine(_graphInfo.x + (i * _xSpacing) + (_xSpacing / 2), _graphInfo.y, _graphInfo.h + 3, BLACK);
    sprintf(temp, "%d:%02d", _time->tm_hour, _time->tm_min);
    printAlignText(temp, _graphInfo.x + (i * _xSpacing) + (_xSpacing / 2), _graphInfo.y + _graphInfo.h + 10, ALIGMENT_CENTERTOP);
  }

  // Find the first and last index of the data array (hopefully they are sorted from older to the newest).
  // There is a posibility that data will be missing, so use default values; for bottom use zero, for top use number of elements in X array minus 1.
  int _firstIndex = 0;
  int _lastIndex = _graphInfo.xDataCount - 1;

  // Finding first and last index is a littlebit tricky. Search from bottom to top to find first index and at the same time search from the top to bottom to find the last index.
  // BUT there is a one more gotcha! You need to "flip" the index number for the top side (or for the last index) and also substract by one (since we are talking about the indexes not numbers).
  for (int i = 0; i < _graphInfo.xDataCount; i++)
  {
    uint32_t _currentEpochFromBottom = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * i)));
    uint32_t _currentEpochFromTop = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * (_graphInfo.xDataCount - i - 1))));
    
    if (_currentEpochFromBottom <= _firstEpoch) _firstIndex = i;
    if (_currentEpochFromTop >= _lastEpoch) _lastIndex = _graphInfo.xDataCount - i - 1;
  }

  // Find max and min value but only of the data pairs that are in the time and date interval.
  // Hopefully x and y data arrays are paired...  
  float _dataMax = _intArray[_firstIndex];
  float _dataMin = _dataMax;
  for (int i = _firstIndex + 1; i < _lastIndex; i++)
  {
    if (isnan(_graphInfo.max)) if (_intArray[i] > _dataMax) _dataMax = _intArray[i];
    if (isnan(_graphInfo.min)) if (_intArray[i] < _dataMin) _dataMin = _intArray[i];
  }
  if (_dataMin == _dataMax) _dataMax = _dataMin + 1;
  if (!isnan(_graphInfo.max)) _dataMax = _graphInfo.max;
  if (!isnan(_graphInfo.min)) _dataMin = _graphInfo.min;

  // Now find data step
  float _yStep = (_dataMax - _dataMin) / _graphInfo.yDiv;

  // Draw data on y axis
  for (int i = 0; i < (_graphInfo.yDiv + 1); i++)
  {
    char temp[20];
    sprintf(temp, "%.1f", _dataMax - (i * _yStep));
    _ink->drawFastHLine(_graphInfo.x - 2, _graphInfo.y + (_ySpacing * i), _graphInfo.w, BLACK);
    printAlignText(temp, _graphInfo.x - 5, _graphInfo.y + (_ySpacing * i), ALIGMENT_LEFT);

  }

  // Now draw all the data on graph
  switch (_graphInfo.style)
  {
    case GRAPHSTYLE_LINE:
      for (int i = _firstIndex; i < (_lastIndex - 1); i++)
      {
        uint32_t _epoch1 = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * i)));
        uint32_t _epoch2 = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * (i + 1))));
        int _x1 = (int16_t)map2(_epoch1, _firstEpoch + 59, _lastEpoch - 59, _graphInfo.x + (_xSpacing / 2), _graphInfo.x + _graphInfo.w - (_xSpacing / 2));
        int _x2 = (int16_t)map2(_epoch2, _firstEpoch + 59, _lastEpoch - 59, _graphInfo.x + (_xSpacing / 2), _graphInfo.x + _graphInfo.w - (_xSpacing / 2));
        int _y1 = (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y);
        int _y2 = (int16_t)map2(_intArray[i + 1], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y);
        _ink->drawThickLine(_x1, _y1, _x2, _y2, BLACK, 2);
      }
      break;
    case GRAPHSTYLE_COLUMN:
    {
      // Calculate the step width for each colum based on data amout and graph width. Not perfect solution, but it's usable.
      int _columnW = _graphInfo.w / double(_lastIndex - _firstIndex) - 6;

      // Check if the width in not negative or zero. Set to one in that case.
      if (_columnW <= 0) _columnW = 1;
      
      Serial.printf("lastIndex: %d, firstIndex: %d, cWidth: %d\r\n", _lastIndex, _firstIndex, _columnW);
      for (int i = _firstIndex; i < _lastIndex; i++)
      {
        uint32_t _epoch = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * i)));
        int16_t _xPos = (int16_t)map2(_epoch, _firstEpoch + 59, _lastEpoch - 59, _graphInfo.x + (_xSpacing / 2), _graphInfo.x + _graphInfo.w - (_xSpacing / 2));
        int16_t _yPos = (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y);
        int16_t _columnH = _graphInfo.y + _graphInfo.h - _yPos;
        _ink->fillRect(_xPos, _yPos, _columnW, _columnH, BLACK);
      }
    }
      break;
    case GRAPHSTYLE_DOT:
      for (int i = _firstIndex; i < _lastIndex; i++)
      {
        uint32_t _epoch = (*(uint32_t*)((uint8_t*)(_graphInfo.xDataPtr) + (_graphInfo.xDataOffset * i)));
        int _xPoint = (int16_t)map2(_epoch, _firstEpoch + 59, _lastEpoch - 59, _graphInfo.x + (_xSpacing / 2), _graphInfo.x + _graphInfo.w - (_xSpacing / 2));
        int _yPoint = (int16_t)map2(_intArray[i], _dataMin, _dataMax, _graphInfo.y + _graphInfo.h, _graphInfo.y);
        _ink->fillCircle(_xPoint, _yPoint, 5, BLACK);
      }
      break;
  }
  free(_intArray);
}
