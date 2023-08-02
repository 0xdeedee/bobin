/**
 *  @filename   :   epd4in2b_V2.ino
 *  @brief      :   4.2inch e-paper display (B_V2) demo 
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     Nov 25 2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include "epd4in2b_V2.h"
#include "imagedata.h"
#include "epdpaint.h"


extern "C" {
  #include <hardware/sync.h>
  #include <hardware/flash.h>
};


#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

#define COLORED     0
#define UNCOLORED   1


#define BTN_1     (9)
#define BTN_2     (10)
#define BTN_3     (11)
#define BTN_4     (12)
#define BTN_5     (13)
#define BTN_6     (14)
#define BTN_7     (15)

#define DEFAULT_SPEED       ( 1 )
#define DEFAULT_TURNS       ( 56 )

#define MAGICK      ( 0xDEEDEE ) 


struct data
{
  unsigned int      turns_count:10;
  unsigned int      speed:2;
  unsigned int      magick:4;
  unsigned int      horisontal_turns:8;
  unsigned int      vertical_turns:8;
} __attribute__((packed));
struct data     __data;




int             buf[FLASH_PAGE_SIZE/sizeof(int)];  // One page buffer of ints
int             *p;
int             addr;
unsigned int    page; // prevent comparison of unsigned and signed int
int             first_empty_page = -1;
bool            load_defaults = 0;

unsigned char   image[1500];
Paint           paint(image, 400, 28);    //width should be the multiple of 8 
Epd             epd;

int buttonState = 0;



void menu_screen(int number)
{
  epd.ClearFrame();

  paint.Clear(UNCOLORED);
  char    line_one_string[350];

  memset(line_one_string, 0, sizeof(line_one_string));
  sprintf(line_one_string, "Coil Turns      %d", number);
  paint.DrawStringAt(2, 2, line_one_string, &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 2,  paint.GetWidth(), paint.GetHeight());


  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 32, paint.GetWidth(), paint.GetHeight());
  
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "1 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 62, paint.GetWidth(), paint.GetHeight());


  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "2 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 92, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "3 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 122, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "4 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 152, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "5 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 182, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "6 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 212, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "7 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 242, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(2, 2, "8 Hello world", &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 272, paint.GetWidth(), paint.GetHeight());

  epd.DisplayFrame();

  /* This displays an image */
  // epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);
  // epd.Sleep();


}



void setup() 
{

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BTN_5, INPUT_PULLUP);
  pinMode(BTN_6, INPUT_PULLUP);
  pinMode(BTN_7, INPUT_PULLUP);

  // put your setup code here, to run once:
  Serial.begin(115200);

  addr = XIP_BASE + FLASH_TARGET_OFFSET;
  memcpy( (void * )&__data, ( void * )addr, sizeof( __data ) );
  if( 0xDEEDEE != __data.magick )
    load_defaults = 1;

  if (epd.Init() != 0) 
  {
    Serial.print("e-Paper init failed");
    return;
  }

  menu_screen( sizeof( 0xDEEDEE ) );

}

void loop() 
{

  if ( LOW == digitalRead( BTN_1 ) )
  {
    digitalWrite(LED_BUILTIN, HIGH);
    menu_screen(1);
  }
 
  if ( LOW == digitalRead( BTN_2 ) )
  {
    digitalWrite(LED_BUILTIN, HIGH);
    menu_screen(2);
  }

  delayMicroseconds(1000);

  // delayMicroseconds(500000);
  // digitalWrite(LED_BUILTIN, HIGH);
    
  // delayMicroseconds(500000);
  // digitalWrite(LED_BUILTIN, LOW);

}
