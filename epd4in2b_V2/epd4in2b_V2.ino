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

#include <Scheduler.h>
// Include pico/multicore.h to run core1 function
// NB: no delay(), neither Serial.print etc.... can be called on core1. Only sleep_ms or pico sleep_... funcs

#include "pico/multicore.h"
extern "C" {
  #include <hardware/sync.h>
  #include <hardware/flash.h>
};


#define FLASH_TARGET_OFFSET ( PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE )

#define COLORED             ( 0 )
#define UNCOLORED           ( 1 )

#define DIR_FWD             ( 0 )
#define DIR_BWD             ( 1 )


// PINS
#define PUL                 ( 1 )
#define DIR1                ( 0 )
#define BTN_1               ( 9 )
#define BTN_2               ( 10 )
#define BTN_3               ( 11 )
#define BTN_4               ( 12 )
#define BTN_5               ( 13 )
#define BTN_6               ( 14 )
#define BTN_7               ( 15 )

#define DEFAULT_SPEED       ( 1 )
#define DEFAULT_TURNS       ( 56 )

#define MAGICK              ( 0xDEEDEE ) 

#define ONE_RPM_STEPS       ( 1600 )

struct data
{
  unsigned int      turns_count:10;
  unsigned int      speed:2;
  unsigned int      direction:1;
  unsigned int      magick:4;
  unsigned int      horisontal_turns:7;
  unsigned int      vertical_turns:8;
} __attribute__((packed));

struct data     __data;



int             buf[FLASH_PAGE_SIZE/sizeof(int)];  // One page buffer of ints
int             *p;
int             addr;
unsigned int    page; // prevent comparison of unsigned and signed int
int             first_empty_page = -1;
unsigned int    position = 0;
unsigned int    menu_entered = 0;

unsigned char   image[1500];
Paint           paint(image, 400, 28);    //width should be the multiple of 8 
Epd             epd;

int buttonState = 0;

int loop_motor_enable = 0;
int __stop = 0;

void loop_motor() 
{
  while ( 1 )
  {
    
    digitalWrite( DIR1, 1 );// __data.direction );
    if ( loop_motor_enable )
    {
      digitalWrite(LED_BUILTIN, HIGH);
      for ( int i = 0; i < ( ONE_RPM_STEPS * __data.turns_count ); i ++ )
      {
        digitalWrite( PUL, HIGH );
        delayMicroseconds( 100 );
        digitalWrite( PUL, LOW );
        delayMicroseconds( 100 );
        if ( __stop )
          break;
      }
      loop_motor_enable = 0;      
      digitalWrite(LED_BUILTIN, LOW);

    }
    delayMicroseconds(10000);
    
  } 

}



void load_defaults()
{
  __data.turns_count = 56;
  __data.speed = 2;
  __data.direction = DIR_FWD;
  __data.magick = MAGICK;
  __data.horisontal_turns = 0;
  __data.vertical_turns = 0;
}


void start_stop_screen(char *text)
{
  char    line_string[350];

  epd.ClearFrame();
  paint.Clear(UNCOLORED);
  memset( line_string, 0, sizeof( line_string ) );
  sprintf( line_string, "         %s", text );
  paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 0, 122, paint.GetWidth(), paint.GetHeight());

  epd.DisplayFrame();

}

void menu_screen( int number )
{
  char    line_string[350];

  epd.ClearFrame();

  paint.Clear( UNCOLORED );
  memset( line_string, 0, sizeof( line_string ) );
  sprintf( line_string, "Coil Turns   %d", __data.turns_count );
  paint.DrawStringAt( 2, 2, line_string, &Font24, COLORED );
  if ( position != 1 )
  {
    epd.SetPartialWindowBlack( paint.GetImage(), 0, 2,  paint.GetWidth(), paint.GetHeight() );
  }
  else
  {
    epd.SetPartialWindowRed( paint.GetImage(), 0, 2,  paint.GetWidth(), paint.GetHeight() );
  }

  paint.Clear(UNCOLORED);
  memset( line_string, 0, sizeof( line_string ) );
  sprintf( line_string, "Speed       %d", __data.speed );
  paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  if ( position != 2 )
  {
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 32, paint.GetWidth(), paint.GetHeight());
  }
  else
  {
    epd.SetPartialWindowRed(paint.GetImage(), 0, 32, paint.GetWidth(), paint.GetHeight());
  }

  paint.Clear(UNCOLORED);
  memset( line_string, 0, sizeof( line_string ) );
  sprintf( line_string, "Direction   %d", __data.direction );
  paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  if ( position != 3 )
  {
   epd.SetPartialWindowBlack(paint.GetImage(), 0, 62, paint.GetWidth(), paint.GetHeight());
  }
  else
  {
   epd.SetPartialWindowRed(paint.GetImage(), 0, 62, paint.GetWidth(), paint.GetHeight());
  }

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 92, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 122, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 152, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 182, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 212, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 242, paint.GetWidth(), paint.GetHeight());

  // paint.Clear(UNCOLORED);
  // memset( line_string, 0, sizeof( line_string ) );
  // sprintf( line_string, "Speed       %d", __data.speed );
  // paint.DrawStringAt(2, 2, line_string, &Font24, COLORED);
  // epd.SetPartialWindowBlack(paint.GetImage(), 0, 272, paint.GetWidth(), paint.GetHeight());

  epd.DisplayFrame();

  /* This displays an image */
  // epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);
  // epd.Sleep();


}



void setup() 
{

  pinMode(BTN_1, INPUT_PULLUP);   // START
  pinMode(BTN_2, INPUT_PULLUP);   // DOWN
  pinMode(BTN_3, INPUT_PULLUP);   // LEFT
  pinMode(BTN_4, INPUT_PULLUP);   // MIDDLE
  pinMode(BTN_5, INPUT_PULLUP);   // RIGHT
  pinMode(BTN_6, INPUT_PULLUP);   // UP
  pinMode(BTN_7, INPUT_PULLUP);   // STOP



  pinMode ( PUL, OUTPUT );
  pinMode ( DIR1, OUTPUT );

  // put your setup code here, to run once:
  Serial.begin(115200);

  addr = XIP_BASE + FLASH_TARGET_OFFSET;
  memcpy( (void * )&__data, ( void * )addr, sizeof( __data ) );
  if( 0xDEEDEE != __data.magick )
    load_defaults();

  if (epd.Init() != 0) 
  {
    Serial.print("e-Paper init failed");
    return;
  }

  menu_screen( sizeof( 0xDEEDEE ) );
  multicore_launch_core1( loop_motor );

}
static int count = 0;
void loop() 
{
  
  if ( count == 0 )
  {
     digitalWrite(LED_BUILTIN, HIGH);
  //   delayMicroseconds(50000);
  //   digitalWrite(LED_BUILTIN, LOW);
  }

  count ++;
  if ( LOW == digitalRead( BTN_1 ) )
  {
    //digitalWrite(LED_BUILTIN, HIGH);
    //start_stop_screen( "START" );
    loop_motor_enable = 1;
    __stop = 0;
  }
 
  if ( LOW == digitalRead( BTN_2 ) )    //down
  {
    digitalWrite(LED_BUILTIN, HIGH);
    position --;
    if ( position <= 1 )
      position = 3;

      menu_screen(0);
  }

  if ( LOW == digitalRead( BTN_3 ) )
  {
    digitalWrite(LED_BUILTIN, HIGH);
      //menu_screen(2);
  }

  if ( LOW == digitalRead( BTN_4 ) )
  {
    digitalWrite(LED_BUILTIN, HIGH);
      //menu_screen(2);
  }


  if ( LOW == digitalRead( BTN_5 ) )
  {
    digitalWrite(LED_BUILTIN, HIGH);
      //menu_screen(2);
  }

  if ( LOW == digitalRead( BTN_6 ) )
  {
    position ++;
    if ( position >= 3 )
      position = 1;

 
    digitalWrite(LED_BUILTIN, HIGH);
//    menu_screen(0);
  }


  if ( LOW == digitalRead( BTN_7 ) )
  {
    //digitalWrite(LED_BUILTIN, LOW);
    //start_stop_screen( "STOP" );
    __stop = 1;
    loop_motor_enable = 0;
  }
  //delayMicroseconds(1000);

//   position = 2;
//   menu_screen(0);
// delay(10);

//   position = 3;
//   menu_screen(0);
// delay(10);


}
