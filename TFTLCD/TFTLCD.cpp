#include "TFTLCD.h"

/*	Jeremi Roivas:

	Added this define, because Mike's addded write/read delays (5) were not
	sufficient enough. My rev3 MEGA board needed 6 microsecond delays to work.
	This define adds delay to all read and write operations for MEGA boards and 
	by default they are now 6, tested by me to work. 
	
	If you have white blacklight problem:
	
	PLEASE EDIT THIS HIGHER IF YOUR SCREEN IS BLACKLIGHT WHITE ONLY.
***************************************************************************************************************************/
#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) 
	#define SPEED_DELAY 6
#else
	#define SPEED_DELAY 0
#endif



#ifdef USE_ADAFRUIT_SHIELD_PINOUT
// special defines for the dataport
 #define DATAPORT1 PORTD
 #define DATAPIN1 PIND
 #define DATADDR1 DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2 PINB
 #define DATADDR2 DDRB

 #define DATA1_MASK 0xD0
 #define DATA2_MASK 0x2F

// for mega & shield usage, we just hardcoded it (its messy)

#else
 // for the breakout board tutorial, two ports are used :/
 #define DATAPORT1 PORTD
 #define DATAPIN1  PIND
 #define DATADDR1  DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2  PINB
 #define DATADDR2  DDRB

 #define DATA1_MASK 0xFC  // top 6 bits
 #define DATA2_MASK 0x03  // bottom 2 bits

 #define MEGA_DATAPORT PORTA
 #define MEGA_DATAPIN  PINA
 #define MEGA_DATADDR  DDRA
#endif


#include "glcdfont.c"
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include "wiring_private.h"

// Mike McCauley:
// Manage variations of GRAM arrangement.
// The inexpensive 2.4" touchscreens from Ebay made by http://www.mcufriend.com
// have unusual addressing so added this:
// Some instances of this device have reversed X or Y coordinates
// If your LCD display chip has inverted X addresses define this:
//#define INVERT_X
// If your LCD display chip has inverted Y addresses define this:
//#define INVERT_Y

#ifdef INVERT_X
#define X(x) (TFTWIDTH - x - 1)
#define I_X(x) (x)
#else
#define X(x) (x)
#define I_X(x) (TFTWIDTH - x - 1)
#endif
#ifdef INVERT_Y
#define Y(y) (TFTHEIGHT - y - 1)
#define I_Y(y) (y)
#else
#define Y(y) (y)
#define I_Y(y) (TFTHEIGHT - y - 1)
#endif


void TFTLCD::goHome(void) {
  goTo(0,0);
}

uint16_t TFTLCD::width(void) {
  return _width;
}
uint16_t TFTLCD::height(void) {
  return _height;
}



void TFTLCD::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void TFTLCD::setTextSize(uint8_t s) {
  textsize = s;
}

void TFTLCD::setTextColor(uint16_t c) {
  textcolor = c;
}

size_t TFTLCD::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textsize);
    cursor_x += textsize*6;
  }
  return 0;
}

void TFTLCD::drawString(uint16_t x, uint16_t y, char *c, 
			uint16_t color, uint8_t size) {
  while (c[0] != 0) {
    drawChar(x, y, c[0], color, size);
    x += size*6;
    c++;
  }
}
// draw a character
void TFTLCD::drawChar(uint16_t x, uint16_t y, char c, 
		      uint16_t color, uint8_t size) {
  for (uint8_t i =0; i<5; i++ ) {
    uint8_t line = pgm_read_byte(font+(c*5)+i);
    for (uint8_t j = 0; j<8; j++) {
      if (line & 0x1) {
	if (size == 1) // default size
	  drawPixel(x+i, y+j, color);
	else {  // big size
	  fillRect(x+i*size, y+j*size, size, size, color);
	} 
      }
      line >>= 1;
    }
  }
}

// draw a triangle!
void TFTLCD::drawTriangle(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint16_t x2, uint16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color); 
}

void TFTLCD::fillTriangle ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  int32_t dx1, dx2, dx3; // Interpolation deltas
  int32_t sx1, sx2, sy; // Scanline co-ordinates

  sx2=(int32_t)x0 * (int32_t)1000; // Use fixed point math for x axis values
  sx1 = sx2;
  sy=y0;

  // Calculate interpolation deltas
  if (y1-y0 > 0) dx1=((x1-x0)*1000)/(y1-y0);
    else dx1=0;
  if (y2-y0 > 0) dx2=((x2-x0)*1000)/(y2-y0);
    else dx2=0;
  if (y2-y1 > 0) dx3=((x2-x1)*1000)/(y2-y1);
    else dx3=0;

  // Render scanlines (horizontal lines are the fastest rendering method)
  if (dx1 > dx2)
  {
    for(; sy<=y1; sy++, sx1+=dx2, sx2+=dx1)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx2 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx2, sx2+=dx3)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
  else
  {
    for(; sy<=y1; sy++, sx1+=dx1, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx1 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx3, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
}

uint16_t TFTLCD::Color565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

// draw a rectangle
void TFTLCD::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t color) {
  // smarter version
  drawHorizontalLine(x, y, w, color);
  drawHorizontalLine(x, y+h-1, w, color);
  drawVerticalLine(x, y, h, color);
  drawVerticalLine(x+w-1, y, h, color);
}

// draw a rounded rectangle
void TFTLCD::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  drawHorizontalLine(x+r, y, w-2*r, color);
  drawHorizontalLine(x+r, y+h-1, w-2*r, color);
  drawVerticalLine(x, y+r, h-2*r, color);
  drawVerticalLine(x+w-1, y+r, h-2*r, color);
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}


// fill a rounded rectangle
void TFTLCD::fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

// fill a circle
void TFTLCD::fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
  drawVerticalLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}


// used to do circles and roundrects!
void TFTLCD::fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta,
			uint16_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    if (cornername & 0x1) {
      drawVerticalLine(x0+x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawVerticalLine(x0-x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}


// draw a circle outline

void TFTLCD::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, 
			uint16_t color) {
  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  drawCircleHelper(x0, y0, r, 0xF, color);
}

void TFTLCD::drawCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername,
			uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;


  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

// fill a rectangle
void TFTLCD::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t fillcolor) {
  // smarter version
  while (h--)
    drawHorizontalLine(x, y++, w, fillcolor);
}


void TFTLCD::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (x >= _width) return;

  drawFastLine(x,y,length,color,1);
}

void TFTLCD::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (y >= _height) return;
  drawFastLine(x,y,length,color,0);
}

void TFTLCD::drawFastLine(uint16_t x, uint16_t y, uint16_t length, 
			  uint16_t color, uint8_t rotflag)
{
  uint16_t newentrymod;
  uint16_t prevEntryMod = readRegister(TFTLCD_ENTRY_MOD); // JW ADD
  
  switch (rotation) {
  case 0:
      x = X(x);
      y = Y(y);
    if (rotflag)
#ifdef INVERT_Y
      newentrymod = 0x1008;   // we want a 'vertical line' decrementing
#else
      newentrymod = 0x1028;   // we want a 'vertical line' incrementing
#endif
    else 
#ifdef INVERT_X
      newentrymod = 0x1020;   // we want a 'horizontal line' decrementing
#else
      newentrymod = 0x1030;   // we want a 'horizontal line' incrementing
#endif
    break;
  case 1:
    swap(x, y);
    // first up fix the X
    x = I_X(x);
    y = Y(y);
    if (rotflag)
#ifdef INVERT_X
      newentrymod = 0x1010;   // we want a 'horizontal line' incrementing
#else
      newentrymod = 0x1000;   // we want a 'horizontal line' decrementing
#endif
    else 
#ifdef INVERT_Y
      newentrymod = 0x1008;   // we want a 'vertical line' decrementing
#else
      newentrymod = 0x1028;   // we want a 'vertical line' incrementing
#endif
    break;
  case 2:
      x = I_X(x);
      y = I_Y(y);
    if (rotflag)
#ifdef INVERT_Y
      newentrymod = 0x1028;   // we want a 'vertical line' incrementing
#else
      newentrymod = 0x1008;   // we want a 'vertical line' decrementing
#endif
    else 
#ifdef INVERT_X
      newentrymod = 0x1030;   // we want a 'horizontal line' incrementing
#else
      newentrymod = 0x1020;   // we want a 'horizontal line' decrementing
#endif
    break;
  case 3:
    swap(x,y);
    x = X(x);
    y = I_Y(y);
    if (rotflag)
#ifdef INVERT_X
      newentrymod = 0x1020;   // we want a 'horizontal line' decrementing
#else
      newentrymod = 0x1030;   // we want a 'horizontal line' incrementing
#endif
    else 
#ifdef INVERT_Y
      newentrymod = 0x1028;   // we want a 'vertical line' incrementing
#else
      newentrymod = 0x1008;   // we want a 'vertical line' decrementing
#endif
    break;
  }
  
  writeRegister(TFTLCD_ENTRY_MOD, newentrymod);

  writeRegister(TFTLCD_GRAM_HOR_AD, x); // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(TFTLCD_GRAM_VER_AD, y); // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(TFTLCD_RW_GRAM);  // Write Data to GRAM (R22h)

  setWriteDir();
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_wr, HIGH);

  while (length--) {
    writeData_unsafe(color); 
  }

  // set back to default
  digitalWrite(_cs, HIGH);
  
  writeRegister(TFTLCD_ENTRY_MOD, prevEntryMod); // JW ADD
}



// bresenham's algorithm - thx wikpedia
void TFTLCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
		      uint16_t color) {
  // if you're in rotation 1 or 3, we need to swap the X and Y's

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  //dy = abs(y1 - y0);
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void TFTLCD::fillScreen(uint16_t color) {
  goHome();
  uint32_t i;
  
  i = 320;
  i *= 240;
  
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_wr, HIGH);

  setWriteDir();
  while (i--) {
    writeData_unsafe(color); 
  }
  digitalWrite(_cs, HIGH);
}

void TFTLCD::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  // check rotation, move pixel around if necessary
    switch (rotation) {
	case 0:
	    x = X(x);
	    y = Y(y);
	    break;
	    
	case 1:
	    swap(x, y);
	    x = I_X(x);
	    y = Y(y);
	    break;
	case 2:
	    x = I_X(x);
	    y = I_Y(y);
	    break;
	case 3:
	    swap(x, y);
	    x = X(x);
	    y = I_Y(y);
	    break;
    }
    
  if ((x >= TFTWIDTH) || (y >= TFTHEIGHT)) return;
  writeRegister(TFTLCD_GRAM_HOR_AD, x); // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(TFTLCD_GRAM_VER_AD, y); // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(TFTLCD_RW_GRAM);  // Write Data to GRAM (R22h)
  writeData(color);
}

static const uint16_t _regValues[] PROGMEM = {
		0x00e5,0x8000,
		0x0000,0x0001,

		0x0001,0x0100,
		0x0002,0x0700,
		0x0003,0x1030,
		0x0004,0x0000,
		0x0008,0x0202,
		0x0009,0x0000,
		0x000a,0x0000,	
		0x000c,0x0000,
		0x000d,0x0000,
		0x000f,0x0000,
//*********************************************Power On
		0x0010,0x0000,
		0x0011,0x0000,
		0x0012,0x0000,
		0x0013,0x0000,

		0x0010,0x17b0,
		0x0011,0x0037,

		0x0012,0x0138,

		0x0013,0x1700,
		0x0029,0x000d,

		0x0020,0x0000,
		0x0021,0x0000,
//*********************************************Set gamma
		0x0030,0x0001,
		0x0031,0x0606,
		0x0032,0x0304,
		0x0033,0x0202,
		0x0034,0x0202,
		0x0035,0x0103,
		0x0036,0x011d,
		0x0037,0x0404,
		0x0038,0x0404,
		0x0039,0x0404,
		0x003c,0x0700,
		0x003d,0x0a1f,
		

//**********************************************Set Gram aera
		0x0050,0x0000,
		0x0051,0x00ef,
		0x0052,0x0000,
		0x0053,0x013f,
		0x0060,0x2700,
		0x0061,0x0001,
		0x006a,0x0000,
//*********************************************Paratial display
		0x0090,0x0010,
		0x0092,0x0000,
		0x0093,0x0003,
		0x0095,0x0101,
		0x0097,0x0000,
		0x0098,0x0000,
//******************************************** Plan Control
		0x0007,0x0021,

		0x0007,0x0031,

		0x0007,0x0173,

//		LLCD_WRITE_CMD(0x0022,

  // Display On

};

void TFTLCD::initDisplay(void) {
  uint16_t a, d;

  reset();
     
  for (uint8_t i = 0; i < sizeof(_regValues) / 4; i++) {
    a = pgm_read_word(_regValues + i*2);
    d = pgm_read_word(_regValues + i*2 + 1);
    if (a == 0xFF) {
      delay(d);
    } else {
      writeRegister(a, d);
      //Serial.print("addr: "); Serial.print(a); 
      //Serial.print(" data: "); Serial.println(d, HEX);
    }
  }
}

uint8_t TFTLCD::getRotation(void) {
  return rotation;
}



/********************************* low level pin initialization */

TFTLCD::TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset) {
  _cs = cs;
  _cd = cd;
  _wr = wr;
  _rd = rd;
  _reset = reset;
  
  rotation = 0;
  _width = TFTWIDTH;
  _height = TFTHEIGHT;

  // disable the LCD
  digitalWrite(_cs, HIGH);
  pinMode(_cs, OUTPUT);  
  
  digitalWrite(_cd, HIGH);
  pinMode(_cd, OUTPUT);  
  
  digitalWrite(_wr, HIGH);
  pinMode(_wr, OUTPUT);  
  
  digitalWrite(_rd, HIGH);
  pinMode(_rd, OUTPUT);  

  digitalWrite(_reset, HIGH); 
  pinMode(_reset, OUTPUT); 

  csport = digitalPinToPort(_cs);
  cdport = digitalPinToPort(_cd);
  wrport = digitalPinToPort(_wr);
  rdport = digitalPinToPort(_rd);

  cspin = digitalPinToBitMask(_cs);
  cdpin = digitalPinToBitMask(_cd);
  wrpin = digitalPinToBitMask(_wr);
  rdpin = digitalPinToBitMask(_rd);

  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = 0xFFFF;
}


/********************************** low level pin interface */

/* 
	Lett users define output port name for 8bit registers 
	Supported registers: A,B,C,E,F,H,J,K,L
	Not supported on UNO: D,E,F,H,J,K,LCD
	Not supported on Mega: D, G
	Not supported registers are those that may be found in other MCU's
	This said, you can add your register code in Low level pin setting down on the bottom
	*/
void TFTLCD::outputPort(char port) {
	outPort = port;		
}

void TFTLCD::reset(void) {
  if (_reset)
    digitalWrite(_reset, LOW);
  delay(2); 																					/////////////////Here is delay
  if (_reset)
    digitalWrite(_reset, HIGH);

  // resync
  writeData(0);
  writeData(0);
  writeData(0);  
  writeData(0);
}

inline void TFTLCD::setWriteDir(void) {
	switch (outPort) {
		case 'A':
			DDRA |= 0xFF;
			break;
		case 'B':
			DDRB |= 0xFF;
			break;
		case 'C':
			DDRC |= 0xFF;
			break;
		case 'E':
			DDRE |= 0xFF;
			break;
		case 'F':
			DDRF |= 0xFF;
			break;
		case 'H':
			DDRH |= 0xFF;
			break;
		case 'K':
			DDRK |= 0xFF;
			break;	
		case 'L' 
			DDRL |= 0xFF;
			break;
		case 'K':
			DDRK |= 0xFF;
			break;
		default:
			#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)
				DATADDR2 |= DATA2_MASK;
				DATADDR1 |= DATA1_MASK;
			#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) 

				#ifdef USE_ADAFRUIT_SHIELD_PINOUT
					DDRH |= 0x78;
					DDRB |= 0xB0;
					DDRG |= _BV(5);
				#else
					
					DDRH |= 0x78;
					DDRE |= 0x38;
					DDRG |= 0x20;
 
				#endif
			#else
				#error "No pins defined!"
			#endif		
	}

}

inline void TFTLCD::setReadDir(void) {
	switch (outPort) {
		// D and G ports are not suuported for limitles pin count for 8 bit
		// Operation
		case 'A':
			DDRA &= 0x00;
			break;
		case 'B':
			DDRB &= 0x00;
			break;
		case 'C':
			DDRC &= 0x00;
			break;
		case 'E':
			DDRE &= 0x00;
			break;
		case 'F':
			DDRF &= 0x00;
			break;
		case 'H':
			DDRH &= 0x00;
			break;
		case 'K':
			DDRK &= 0x00;
			break;	
		case 'L' 
			DDRL &= 0x00;
			break;
		case 'K':
			DDRK &= 0x00;
			break;
		default:
			#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)
				DATADDR2 &= ~DATA2_MASK;
				DATADDR1 &= ~DATA1_MASK;
			#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) 

				#ifdef USE_ADAFRUIT_SHIELD_PINOUT
					DDRH &= ~0x78;
					DDRB &= ~0xB0;
					DDRG &= ~_BV(5);
				#else
					DDRH &= ~0x78;
					DDRE &= ~0x38;
					DDRG &= ~(0x20);
				#endif
			#else
				#error "No pins defined!"
			#endif 
	}
}

inline void TFTLCD::write8(uint8_t d) {
	switch (outPort) {
		// D and G ports are not suuported for limitles pin count for 8 bit
		// Operation
		case 'A':
			PORTA = d;
			break;
		case 'B':
			PORTB = d;
			break;
		case 'C':
			PORTC = d;
			break;
		case 'E':
			PORTE = d;
			break;
		case 'F':
			PORTF = d;
			break;
		case 'H':
			PORTH = d;
			break;
		case 'K':
			PORTK = d;
			break;	
		case 'L' 
			PORTL = d;
			break;
		case 'K':
			PORTL = d;
			break;
		default:
			#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)

			DATAPORT2 = (DATAPORT2 & DATA1_MASK) | (d & DATA2_MASK);
			DATAPORT1 = (DATAPORT1 & DATA2_MASK) | (d & DATA1_MASK); // top 6 bits
  
			#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) 


				#ifdef USE_ADAFRUIT_SHIELD_PINOUT

					// bit 6/7 (PH3 & 4)
					// first two bits 0 & 1 (PH5 & 6)
					PORTH &= ~(0x78);
					PORTH |= ((d&0xC0) >> 3) | ((d&0x3) << 5);

					// bits 2 & 3 (PB4 & PB5)
					// bit 5 (PB7)
					PORTB &= ~(0xB0); 
					PORTB |= ((d & 0x2C) << 2);

					// bit 4  (PG5)
					if (d & _BV(4)) PORTG |= _BV(5);
					else PORTG &= ~_BV(5);

				#else
				
					// bit 6/7 (PH3 & 4)
					// first two bits 0 & 1 (PH5 & 6)
					PORTH &= ~(0x78);
					PORTH |= ((d&0xC0) >> 3) | ((d&0x3) << 5);
  	
					// bits 2 & 3 (PE4 & 5)
					// bit 5 (PE3)
					PORTE &= ~(0x38);
					PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
  	
					// bit 4
					PORTG &= ~(0x20);
					PORTG |= (d & 0x10) << 1;
			break;
	}
#endif

#else
  #error "No pins defined!"
#endif 
}

inline uint8_t TFTLCD::read8(void) {
	uint8_t d;
	switch (outPort) {
		// D and G ports are not suuported for limitles pin count for 8 bit
		// Operation
		case 'A':
			d = PINA;
			break;
		case 'B':
			d = PINB;
			break;
		case 'C':
			d = PINC;
			break;
		case 'E':
			d = PINE;
			break;
		case 'F':
			d = PINF;
			break;
		case 'H':
			d = PINH;
			break;
		case 'J':
			d = PINJ;
			break;	
		case 'L' 
			d = PINL;
			break;
		case 'K':
			d = PINK;
			break;
		default:
			#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)

				d = DATAPIN1 & DATA1_MASK; 
				d |= DATAPIN2 & DATA2_MASK; 

			#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__)  || defined(__AVR_ATmega1280__) 

				#ifdef USE_ADAFRUIT_SHIELD_PINOUT

					// bit 6/7 (PH3 & 4)
					// first two bits 0 & 1 (PH5 & 6)
					d = (PINH & 0x60) >> 5;
					d |= (PINH & 0x18) << 3;

					// bits 2 & 3 & 5 (PB4 & PB5, PB7)
					d |= (PINB & 0xB0) >> 2;

					// bit 4  (PG5)
					if (PING & _BV(5))
					d |= _BV(4);

				#else	
					d = (PINH & 0x60) >> 5;
					d |= (PINH & 0x18) << 3;
					d |= (PINE & 0x8) << 2;
					d |= (PINE & 0x30) >> 2;
					d |= (PING & 0x20) >> 1;	
				#endif
#			else
				#error "No pins defined!"

			#endif
			break;
	}
 return d;
}

/********************************** low level readwrite interface */

// the C/D pin is high during write
void TFTLCD::writeData(uint16_t data) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

	setWriteDir();
	
  	digitalWrite(_cs, LOW);
  	digitalWrite(_cd, HIGH);
  	digitalWrite(_rd, HIGH);
  
  	digitalWrite(_wr, HIGH);  
  	
  	write8(data >> 8);
  	digitalWrite(_wr, LOW);
  	delayMicroseconds(SPEED_DELAY);			//Original 5
  	digitalWrite(_wr, HIGH);

  	write8(data);
  	
  	digitalWrite(_wr, LOW);
  	delayMicroseconds(SPEED_DELAY);			//Original 5
  	digitalWrite(_wr, HIGH);

  	digitalWrite(_cs, HIGH);
}

// this is a 'sped up' version, with no direction setting, or pin initialization
// not for external usage, but it does speed up stuff like a screen fill
inline void TFTLCD::writeData_unsafe(uint16_t data) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

  write8(data >> 8);

  digitalWrite(_wr, LOW);
  delayMicroseconds(SPEED_DELAY);			//Original 5
  digitalWrite(_wr, HIGH);

  write8(data);

  digitalWrite(_wr, LOW);
  delayMicroseconds(SPEED_DELAY);			//Original 5
  digitalWrite(_wr, HIGH);
}

// the C/D pin is low during write
void TFTLCD::writeCommand(uint16_t cmd) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

   setWriteDir();
   
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, LOW);
  digitalWrite(_rd, HIGH);
  digitalWrite(_wr, HIGH);
  
  write8(cmd >> 8);

  digitalWrite(_wr, LOW); 
  delayMicroseconds(SPEED_DELAY);    	//original 10
  digitalWrite(_wr, HIGH);

  write8(cmd);

  digitalWrite(_wr, LOW); 
  delayMicroseconds(SPEED_DELAY);		//original 10
  digitalWrite(_wr, HIGH);
  digitalWrite(_cs, HIGH);
  
}

uint16_t TFTLCD::readData() {

  uint16_t d = 0;

  setReadDir();
   
  digitalWrite(_cs, LOW);
  digitalWrite(_cd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_wr, HIGH);
  
  digitalWrite(_rd, LOW);
  delayMicroseconds(10);			//original 10
  d = read8();
  digitalWrite(_rd, HIGH);
  
  d <<= 8;
  
  digitalWrite(_rd, LOW);
  delayMicroseconds(10);			//original 10
  d |= read8();
  digitalWrite(_rd, HIGH);
  
  digitalWrite(_cs, HIGH);

  return d;
}


/************************************* medium level data reading/writing */

uint16_t TFTLCD::readRegister(uint16_t addr) {
   writeCommand(addr);
   return readData();
}

void TFTLCD::writeRegister(uint16_t addr, uint16_t data) {
   writeCommand(addr);
   writeData(data);
}


void TFTLCD::goTo(uint16_t x, uint16_t y) {
  	calcGRAMPosition(&x, &y);
	writeRegister(0x0020, x);     // GRAM Address Set (Horizontal Address) (R20h)
  	writeRegister(0x0021, y);     // GRAM Address Set (Vertical Address) (R21h)	
  	writeCommand(0x0022);            // Write Data to GRAM (R22h)
}

void TFTLCD::setDefaultViewport()
{
	writeRegister(TFTLCD_HOR_START_AD, 0);    
	writeRegister(TFTLCD_HOR_END_AD, TFTWIDTH - 1);    
	writeRegister(TFTLCD_VER_START_AD, 0);     
	writeRegister(TFTLCD_VER_END_AD, TFTHEIGHT -1);     		
}

void TFTLCD::getViewport(uint16_t *bx, uint16_t *by, uint16_t *ex, uint16_t *ey)
{
	*bx = readRegister(TFTLCD_HOR_START_AD);    
	*ex = readRegister(TFTLCD_HOR_END_AD);    
	*by = readRegister(TFTLCD_VER_START_AD);     
	*ey = readRegister(TFTLCD_VER_END_AD);     
}

void TFTLCD::setViewport(uint16_t bx, uint16_t by, uint16_t ex, uint16_t ey)
{
	calcGRAMPosition(&bx, &by);
	calcGRAMPosition(&ex, &ey);
	
	// Fix coordinates to be in order
	if( ey < by )
		swap(ey, by);
	if( ex < bx )
		swap(ex, bx);
	
	writeRegister(TFTLCD_HOR_START_AD, bx);    
	writeRegister(TFTLCD_HOR_END_AD, ex);    
	writeRegister(TFTLCD_VER_START_AD, by);     
	writeRegister(TFTLCD_VER_END_AD, ey); 	
}

// Writes 16-bit data in bulk, using callback to get more
void TFTLCD::bulkWrite(uint16_t *data, uint16_t bufferSize, uint16_t (*getNextValues)(void *), void *userData)
{
	*portOutputRegister(csport) &= ~cspin;
	*portOutputRegister(cdport) |= cdpin;
	*portOutputRegister(rdport) |= rdpin;
	*portOutputRegister(wrport) |= wrpin;

	setWriteDir();
	while( bufferSize )
	{
		for(uint16_t i=0; i < bufferSize; i++)
		{
			writeData_unsafe(data[i]);
		}
		bufferSize = getNextValues(userData);
	}
	*portOutputRegister(csport) |= cspin;		
}

 
 
void TFTLCD::calcGRAMPosition(uint16_t *posx, uint16_t *posy)
{
  uint16_t x = *posx;
  uint16_t y = *posy;
  switch( rotation )
  {
      case 0:
	  x = X(x);
	  y = Y(y);
	  break;
      case 1:  // 90
	  swap(x, y);
	  x = I_X(x);
	  y = Y(y);
	  break;
      case 2:  // 180
	  x = I_X(x);
	  y = I_Y(y);
	  break;
      case 3: // 270
	  swap(x, y);
	  y = I_Y(y);
	  break;
  }
  *posx = x;
  *posy = y;
}



void TFTLCD::setRotation(uint8_t x) {

	x %= 4;  // cant be higher than 3
	rotation = x;
	switch (x) {
	case 0:
		writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
		_width = TFTWIDTH; 
		_height = TFTHEIGHT;
		break;
	case 1:
		writeRegister(TFTLCD_ENTRY_MOD, 0x1028);
		_width = TFTHEIGHT; 
		_height = TFTWIDTH;
		break;
	case 2:
		writeRegister(TFTLCD_ENTRY_MOD, 0x1000);
		_width = TFTWIDTH; 
		_height = TFTHEIGHT;
		break;
	case 3:
		 writeRegister(TFTLCD_ENTRY_MOD, 0x1018 );
		_width = TFTHEIGHT; 
		_height = TFTWIDTH;
		break;
	}
	setDefaultViewport();
}
