/********************************************************************************************************************************
Tron Legacy - Sam Flynn's Identity Disk
Rik van der Mark - 2017. Code from Adafruit and Erv Plecter was used to get it all working.

- The related project page on my website is: https://www.ricor.net/tron-legacy-sam-flynn-identity-disc/
- My GitHub page: https://github.com/RiCor-net
- YouTube: RiCor Net

This sketch was written to build upon the sketches written before where I tested button control, NeoPixel and Audio.
All files are on the mentioned GitHub page and videos will be made on my channel to explain what I did.

In this iteration of the sketch I have added the Tron Disc animation code from the sketch written bij Erv Plecter.
A big thank you to Erv for providing his sketch as it is a very big factor that makes this disc work.
Erv's website: http://www.plecterlabs.com/shop/article_info.php?articles_id=46
YouTube: Erv Plecter

I kindly ask that people credit Adafruit, Erv Plecter and me if they re-use this code.
Thank you and enjoy!

< End of Line >
********************************************************************************************************************************/

// This sketch requires the folowing libraries:
/* include SPI, MP3 and SD libraries for the Music Maker FeatherWing; */

/* NeoPixel related libraries; */
#include <Adafruit_NeoPixel.h>

/* Timing libraries */ 
#include <elapsedMillis.h>


/****************************************************/
// Definitions

/*Buttos, switches and sensors */
#define BUTTON1_PIN A2
#define BUTTON2_PIN A3
#define BUTTON3_PIN A4

/**************************/
/* Pins used on the Feather M0 or 32u4 */


/**************************/
/* NeoPixel strip definitions for the outer ring */
#define PIXEL_PIN 11    /* Digital IO pin connected to the NeoPixels strip. */
#define PIXEL_COUNT 88  /* Number of NeoPixels on the strip */

/* NeoPixel strip definitions for the inner c-ring 
   Actually consists of 118 LEDs but you only drive 59 "unique" LEDs */ 
#define PIXEL_PIN2 12    /* Digital IO pin connected to the NeoPixels strip. */ 
#define PIXEL_COUNT2 59  /* Number of NeoPixels on the strip */ 

/* This section delares the NeoPixel objects. Used later to control the strips. (Refer to Adafruit NeoPixel überguide for more information)*/
// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick

/*   First line declares the NeoPixel strip used for the outer ring, */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

/* Second line declares the NeoPixel strip used for the inner c-ring. */
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(PIXEL_COUNT2, PIXEL_PIN2, NEO_GRBW + NEO_KHZ800);

/**************************/
// ERV DEFINITIONS

#define MAX_SERIAL   80
#define MAX_STRING_LEN  80

#define MIN_RANDOM_PERIOD    1
#define MAX_RANDOM_PERIOD    2


#define SEQUENCE_OFF         0
#define SEQUENCE_POWERON     1
#define SEQUENCE_BLADE       2
#define SEQUENCE_POWEROFF    3
#define SEQUENCE_LOCKUP      4
#define SEQUENCE_CLASH       5
#define SEQUENCE_PULSE       6
#define SEQUENCE_BOOT        7

#define MAX_SEQUENCE        8

#define BLADE_SIZE          5
#define BLADE_SEGMENTS      4
#define BLADE_OFFSET        23

#define INNER_RING_SEGMENTS  16
#define INNER_RING_SEGMENT_SIZE  7


#define SAM_DISC            0
#define RINZLER_DISC        1

#define PULSE_DURATION      450
#define PULSE_DOWN          0
#define PULSE_UP            1
#define NO_BRIGHTNESS       255
#define MAX_BRIGHTNESS      255

#define BOOT_DURATION       16


/****************************************************/
// Global variables

/*Buttos, switches and sensors */
bool oldState1 = HIGH;
bool oldState2 = HIGH;
bool oldState3 = HIGH;
int showType1 = 0;
int showType2 = 2;
int showType3 = 4;

/**************************/
// ERV GLOBAL VARIABLES
const byte BootPattern[7] = {1,1,1,0,1,1,0};
const byte BootPattern2[8] = {1,1,1,0,1,1,1,0};

// Global Vars
int inByte = 0;         // incoming serial byte
char SerialString[MAX_SERIAL];
char StringBuffer[MAX_STRING_LEN];
unsigned char SerialIndex;
unsigned char DebugMode = false;
unsigned char SelectedSequence = SEQUENCE_OFF;
elapsedMillis timeElapsed;
unsigned int CurrentEventPeriod = 1;
unsigned int CurrentSequenceStep = 0;
uint32_t CurrentColor;
uint32_t CurrentColorFlash;
uint32_t CurrentColorInner;
byte IdleFlag = false;
byte LoopFlag = false;
byte SameOrRinzler = SAM_DISC;
unsigned int Brightness = MAX_BRIGHTNESS;
unsigned long PulseBrightness;
byte PulseDirection = PULSE_DOWN;
byte CurrentR, CurrentG, CurrentB;

unsigned char InnerRingPixels = 0;


/**************************/
/* Colors */ 

// Adafruit NeoPixel Überguide;
/*The next three arguments are the pixel color, expressed as red, green and blue brightness levels, 
  where 0 is dimmest (off) and 255 is maximum brightness. The last optional argument is for white, 
  which will only be used if the strip was defined during creation as an RGBW type and the strip actually is RGBW type. */

//Set 4th digit to 0 for now

uint32_t Black = strip.Color(0,0,0,0);
uint32_t White = strip.Color(60,127,200,0);
//uint32_t White = strip.Color(0,0,0,255);
//uint32_t BlueishWhite = strip.Color(60,127,200,0);    // Blueish white
uint32_t WhiteHalf = strip.Color(15,30,40,0);
uint32_t Red = strip.Color(255,60,0,0);
uint32_t Redhalf = strip.Color(60,30,30,0);
uint32_t Green = strip.Color(0,255,0,0);
uint32_t Blue = strip.Color(0,0,255,0);

uint32_t BlueFlash = strip.Color(100,200,255,0);
uint32_t RedFlash = strip.Color(255,200,0,0);

uint32_t WhiteInnerRing = strip.Color(30,64,100,0);
uint32_t RedInnerRing = strip.Color(150,30,0,0);

uint32_t BlueBlade = strip.Color(0,45,255,0);
uint32_t RedBlade = strip.Color(255,90,0,0);

/**************************/

unsigned int TimingArray[MAX_SEQUENCE] = {1, 5, 2, 5, 0, 2, 2, 250};
uint32_t *ColorArray[MAX_SEQUENCE] = {&Black, &CurrentColor, &CurrentColor, &Black, &CurrentColor, &CurrentColorFlash, &CurrentColor,&CurrentColorInner};

unsigned int IdleArray[MAX_SEQUENCE] = {true, false, false, false,false,false,false,true};
unsigned int LengthArray[MAX_SEQUENCE] = {PIXEL_COUNT, PIXEL_COUNT, PIXEL_COUNT, PIXEL_COUNT, PIXEL_COUNT, 70, PULSE_DURATION, BOOT_DURATION};
unsigned int LoopArray[MAX_SEQUENCE] = {false, false, true, false, true, false, true, false};
unsigned int BladeBrightness[2*BLADE_SIZE] = {100,90,60,40,30,20,10,5,0,0}; // % of brightness related to the main color of the disc
uint32_t TheBlade[2*BLADE_SIZE];


/****************************************************/
// Functions

//ERV FUNCTIONS
void ParseSyntax(void);
unsigned char SkipToValue(void);
unsigned char SkipToNextValue(unsigned char CurrentIndex);
void EmptySerial(void);
void EmptyString(void);
void Animate(void);
void InitBlade(byte WhichDisc);
void InitSequence(void);

////////////////////////////////////////////////////////////////////////////
/* Setup code, runs once */
void setup() 
{
  Serial.begin(9600); /* set data rate in bits per second (baud) for serial data communication  */
  delay(1000); /* 1 second delay to make sure all the serial output is displayed during setup. */
               /* Not active when the disc is used stand-alone */

  /* Serial messaging to let the disk talk to the world :) 
     It will also give a status for all the different hardware elements during initialisation lateron */
  Serial.println("Hello World!"); //REMOVE LATER
  Serial.println("================================================");
  Serial.println("RiCor.net");
  Serial.println("Tron Legacy Identity Disc v0.1");
  Serial.println("System Initializing");
  Serial.println("");
  Serial.println("------------------------------------------------");


/*********************************/

  /* Setup the switch pins as digital inputs */
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  Serial.println("> Button Pins setup OK");  


/*********************************/
              
//Music Maker setup code section


/*********************************/
              
  /* Initializing the two NeoPixel strips */
  Serial.println("> Initialising the NeoPixel strips");
  
  strip.begin(); /* Prepare data pin for NeoPixel output */
  strip.show();  /* Initialize all pixels to 'off' */
  
  strip2.begin(); /* Prepare data pin for NeoPixel output */
  strip2.show();  /* Initialize all pixels to 'off' */

  CurrentColor = White;
  CurrentColorInner = WhiteInnerRing;
  
  // Init Blade effects
  InitBlade(SAM_DISC);


/*********************************/

  Serial.println("------------------------------------------------");
  Serial.println("");
  Serial.println("Disc Activated and Synchronized");
  Serial.println("Grid is Live!");
  Serial.println("");
  Serial.println("================================================");
  Serial.println("");
}

////////////////////////////////////////////////////////////////////////////
/* Main code, runs repeatedly */
void loop() 
{ 

  // Time Management. Works like an int but in main loop
  // Not as accurate as real a real ISR but we'll see if the visual result is ok
  if((timeElapsed > CurrentEventPeriod) && !IdleFlag )
  {
      timeElapsed = 0;
      if(!TimingArray[SelectedSequence])
        CurrentEventPeriod = random(MIN_RANDOM_PERIOD, MAX_RANDOM_PERIOD);
        
      Animate();    
      CurrentSequenceStep++;
      if(CurrentSequenceStep >= LengthArray[SelectedSequence])
      {
         CurrentSequenceStep = 0;
         switch(SelectedSequence)
         {
            case SEQUENCE_POWERON:
            case SEQUENCE_CLASH:
             strip.setBrightness(MAX_BRIGHTNESS);  
             SelectedSequence = SEQUENCE_BLADE;
             InitSequence();
             break;
             
            case SEQUENCE_PULSE:
              if(PulseDirection == PULSE_UP)
                PulseDirection = PULSE_DOWN;
              else
                PulseDirection = PULSE_UP;  
              break;
             
            default:
              if(!LoopFlag)
               IdleFlag = true;
         }
      }
  } 
  
/****************************************************/
  /* Button used for controlling the blade */
  bool newState1 = digitalRead(BUTTON1_PIN); /* Get current button state (HIGH or LOW) */

  if (newState1 == LOW && oldState1 == HIGH) /* Check if state changed from high to low (button press). */
  {
    delay(20); /* Short delay to debounce button. */
    newState1 = digitalRead(BUTTON1_PIN); /* Check if button is still low after debounce. */
    if (newState1 == LOW) 
    {
      showType1++;       /* increase "showType" by 1 */
      if (showType1 > 1) /* If "showType1" becomes higher then "1" it triggers the next line which resets it to "0"
                            effectively toggling it between 0 and 1. */
        showType1=0;
      ParseSyntax(showType1); /* Parses the value of "showType*" to the ButtonControl function. */
    }
  }
  oldState1 = newState1;  /* Set the last button state to the old state. */
    
/*********************************/
  /* Button used for turning the music on/off */
  bool newState2 = digitalRead(BUTTON2_PIN);

  if (newState2 == LOW && oldState2 == HIGH)
  {
    delay(20);
    newState2 = digitalRead(BUTTON2_PIN);
    if (newState2 == LOW) 
    {
      showType2++;
      if (showType2 > 3) /* for this button "ShowType2" toggles between 3 and 4 */
        showType2=2;
      ParseSyntax(showType2);
    }
  }
  oldState2 = newState2;
    
/*********************************/
  /* Button used for going to the next song */
  bool newState3 = digitalRead(BUTTON3_PIN);

  if (newState3 == LOW && oldState3 == HIGH)
  {
    delay(20);
    newState3 = digitalRead(BUTTON3_PIN);
    if (newState3 == LOW) 
    {
      ParseSyntax(showType3); /* For this button only one value is required so it always parses
                                   the value given to "showType3" variable to the "ButtonControl" function. */
    }
  }
  oldState3 = newState3;
    
/****************************************************/

} 

////////////////////////////////////////////////////////////////////////////
// Functions

/*********************************/
void InitSequence(void)
{
  uint16_t i;
  
  CurrentEventPeriod = TimingArray[SelectedSequence];
  if(!CurrentEventPeriod)
    CurrentEventPeriod = random(MIN_RANDOM_PERIOD, MAX_RANDOM_PERIOD);
  IdleFlag = false;
  LoopFlag = LoopArray[SelectedSequence];
  CurrentSequenceStep = 0;
  
  // Specific things to do on some sequence init
  switch(SelectedSequence)
  {
    case SEQUENCE_POWEROFF:
    case SEQUENCE_POWERON:
    case SEQUENCE_LOCKUP:
      break;
    
    case SEQUENCE_BLADE:
      InitBlade(SameOrRinzler);
      break;
      
    case SEQUENCE_CLASH:
      for(i=0; i<strip.numPixels(); i++)
        strip.setPixelColor(i, *ColorArray[SEQUENCE_CLASH]);
      strip.show();
      break;
      
    case SEQUENCE_PULSE:
      for(i=0; i<strip.numPixels(); i++) 
        strip.setPixelColor(i, Black);    // Turn off the strip with black
      strip.show();
      
      strip.setBrightness(NO_BRIGHTNESS);
      //CurrentColor = White;  
      for(i=0; i<strip.numPixels(); i++)
        strip.setPixelColor(i, *ColorArray[SEQUENCE_PULSE]);
      PulseDirection = PULSE_UP;
      CurrentR = (CurrentColor >> 16) & 0xFF;
      CurrentG  = (CurrentColor >> 8) & 0xFF;
      CurrentB  = CurrentColor & 0xFF; 
      break;  
    
    case SEQUENCE_BOOT:
      InnerRingPixels = 0;
      break;
    
    default:
      break;
  }
}

/*********************************/
void InitBlade(byte WhichDisc)
{
  byte r, g, b;
  uint16_t i;
    
  switch(WhichDisc)
  {
     case SAM_DISC:
       r = (BlueBlade >> 16) & 0xFF;
       g  = (BlueBlade >> 8) & 0xFF;
       b  = BlueBlade & 0xFF;
       CurrentColor = White;
       CurrentColorFlash = BlueFlash;
       CurrentColorInner = WhiteInnerRing;
       break;
       
     case RINZLER_DISC:
       r = (RedBlade >> 16) & 0xFF;
       g  = (RedBlade >> 8) & 0xFF;
       b  = RedBlade & 0xFF; 
       CurrentColor = Red;
       CurrentColorFlash = RedFlash;
       CurrentColorInner = RedInnerRing;
       break;
       
     default:
       break;
  }
  for(i=0; i<(2*BLADE_SIZE); i++)
    TheBlade[i] = strip.Color(BladeBrightness[i] * r / 100, BladeBrightness[i] * g / 100, BladeBrightness[i] * b / 100);   
}  

/*********************************/
void Animate(void)
{
    byte r, g, b;
    uint32_t DimmedColor;
    
    uint16_t BladePosition;
    uint16_t i, j;
    
    switch(SelectedSequence)
      {        
        case SEQUENCE_POWERON: /* RiCor: Blade animation */
          strip.setPixelColor(CurrentSequenceStep, *ColorArray[SEQUENCE_POWERON]);
          strip.show();
          break;
        
        case SEQUENCE_POWEROFF: /* RiCor: Stop all animation */
          strip.setPixelColor((PIXEL_COUNT-1) - CurrentSequenceStep, *ColorArray[SEQUENCE_POWEROFF]);
          strip.show();
          break;
          
         case SEQUENCE_BLADE:
           for(i=0; i<PIXEL_COUNT; i++)
            strip.setPixelColor(i, *ColorArray[SEQUENCE_BLADE]);
            
           for(j=0 ; j < BLADE_SEGMENTS ; j++)
           {
             BladePosition = CurrentSequenceStep + (BLADE_OFFSET*j);
             for(i=0; i<BLADE_SIZE; i++)
             {
                BladePosition = BladePosition % PIXEL_COUNT;
                if(BladePosition >= i)
                  strip.setPixelColor(BladePosition-i, TheBlade[i]);
                else
                  strip.setPixelColor((PIXEL_COUNT-1)+BladePosition-i, TheBlade[i]);  
             }
           }
           strip.show();
           break; 
          
          case SEQUENCE_LOCKUP:
           for(i=0; i<PIXEL_COUNT; i++)
            strip.setPixelColor(i, *ColorArray[SEQUENCE_BLADE]);
            
           for(j=0 ; j < BLADE_SEGMENTS ; j++)
           {
             BladePosition = CurrentSequenceStep + (BLADE_OFFSET*j);
             for(i=0; i<(2*BLADE_SIZE); i++)
             {
                BladePosition = BladePosition % PIXEL_COUNT;
                if(BladePosition >= i)
                  strip.setPixelColor(BladePosition-i, TheBlade[i]);
                else
                  strip.setPixelColor((PIXEL_COUNT-1)+BladePosition-i, TheBlade[i]);  
             }
           }
           strip.show();
           break; 
         
         case SEQUENCE_CLASH:
           strip.setBrightness(MAX_BRIGHTNESS-(CurrentSequenceStep*2));
           strip.show();           
           break;
         
         case SEQUENCE_PULSE:
           if(PulseDirection == PULSE_UP)
           {
             PulseBrightness = ((100 * CurrentSequenceStep) / PULSE_DURATION) + 1 ; 
           }
           else
           {
             PulseBrightness = 100 - ((100 * CurrentSequenceStep) / PULSE_DURATION);
             //p("bright down=%ld\n",PulseBrightness);
             if(PulseBrightness < 1)
               PulseBrightness = 1;
             
           }
           r = CurrentR * PulseBrightness / 100;
           g = CurrentG * PulseBrightness / 100;
           b = CurrentB * PulseBrightness / 100;
           
           DimmedColor = strip.Color(r,g,b);
           for(i=0; i<PIXEL_COUNT; i++)
             strip.setPixelColor(i, DimmedColor);
           strip.show();
           break;
         
         case SEQUENCE_BOOT: /* RiCor: C-ring boot animation */
           //p("InnerRingPixels=%d\n",InnerRingPixels);
           switch(InnerRingPixels)
           {
             case 7:
             case 22:
             case 37:
             case 52:
               //p("Long\n");
               for(i=0; i<(INNER_RING_SEGMENT_SIZE+1); i++)
               {
                 if(BootPattern2[i])
                   strip2.setPixelColor(InnerRingPixels,WhiteInnerRing);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;
             
             default:
               //p("Short\n");
               for(i=0; i<INNER_RING_SEGMENT_SIZE; i++)
               {
                 if(BootPattern[i])
                   strip2.setPixelColor(InnerRingPixels,WhiteInnerRing);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;             
           
           }

           strip2.show();
           break;
         
          
         default : 
           break; 
      }
}

/*********************************/

void ParseSyntax(int i)
{
  unsigned char Index;
  unsigned char TempSequence;
  
  
  // Sequence Update
  // SEQUENCE_OFF         0
  // SEQUENCE_POWERON     1
  // SEQUENCE_BLADE       2
  // SEQUENCE_POWEROFF    3
  // SEQUENCE_LOCKUP      4
  // SEQUENCE_CLASH       5
  // SEQUENCE_PULSE       6
  // SEQUENCE_BOOT        7
  
    switch(i)
    {
      case 1:
        Serial.println("C-Ring activated"); /* RiCor: Serial message for testing purposes */
        SelectedSequence = SEQUENCE_BOOT; /* RiCor: Triggers C-RING boot animation in the "Animate" function */
        InitSequence();
        break;
      
      case 3:
        Serial.println("Blade activated"); /* RiCor: Serial message for testing purposes */
        SelectedSequence = SEQUENCE_POWERON; /* RiCor: Triggers blade animation in the "Animate" function */
        InitSequence();
        break;
      
      case 2:
        Serial.println("Blade deactivated"); /* RiCor: Serial message for testing purposes */
        SelectedSequence = SEQUENCE_POWEROFF; /* RiCor: Stops the blade animation in the "Animate" function */
        InitSequence();
        break;
      
      case 'b':
        SelectedSequence = SEQUENCE_BLADE;
        InitSequence();
        break;
  
      case 'l': //RiCor: is the letter "L", not the no. "1"
        SelectedSequence = SEQUENCE_LOCKUP;
        InitSequence();
        break;
    
      case 'c':
        SelectedSequence = SEQUENCE_CLASH;
        InitSequence();
        break;
        
      case 'p':
        SelectedSequence = SEQUENCE_PULSE;
        InitSequence();
        break;  
  
      case 's':
          SameOrRinzler = SAM_DISC;
//        if(DebugMode)
//        p("Disc updated to %d\n",SameOrRinzler);
        InitBlade(SameOrRinzler);
        InitSequence();
        
        InnerRingPixels = 0;
        while(InnerRingPixels<PIXEL_COUNT2)
        {
          switch(InnerRingPixels)
          {
             case 7:
             case 22:
             case 37:
             case 52:
               //p("Long\n");
               for(uint16_t i=0; i<(INNER_RING_SEGMENT_SIZE+1); i++)
               {
                 if(BootPattern2[i])
                   strip2.setPixelColor(InnerRingPixels,CurrentColorInner);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;
             
             default:
               //p("Short\n");
               for(uint16_t i=0; i<INNER_RING_SEGMENT_SIZE; i++)
               {
                 if(BootPattern[i])
                   strip2.setPixelColor(InnerRingPixels,CurrentColorInner);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;             
           }
        }
        strip2.show();  
        break;
        
      case 'r':
        SameOrRinzler = RINZLER_DISC;
        InitBlade(SameOrRinzler);
        InitSequence();
        InnerRingPixels = 0;
        while(InnerRingPixels<PIXEL_COUNT2)
        {
          switch(InnerRingPixels)
          {
             case 7:
             case 22:
             case 37:
             case 52:
               //p("Long\n");
               for(uint16_t i=0; i<(INNER_RING_SEGMENT_SIZE+1); i++)
               {
                 if(BootPattern2[i])
                   strip2.setPixelColor(InnerRingPixels,CurrentColorInner);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;
             
             default:
               //p("Short\n");
               for(uint16_t i=0; i<INNER_RING_SEGMENT_SIZE; i++)
               {
                 if(BootPattern[i])
                   strip2.setPixelColor(InnerRingPixels,CurrentColorInner);
                 else
                   strip2.setPixelColor(InnerRingPixels,Black);
                 InnerRingPixels ++;
               }
               break;             
           }
        }
        strip2.show();  
        break;
        
     
     case 'o':
       for(uint16_t i=0; i<strip.numPixels(); i++)
         strip.setPixelColor(i, *ColorArray[SEQUENCE_OFF]);
       strip.show();
       CurrentSequenceStep = 0;
       IdleFlag = true;
       break;
     
     // ALL off
     case 4:
       Serial.println("ALL OFF");
       for(uint16_t i=0; i<strip.numPixels(); i++)
       {
         strip.setPixelColor(i, Black);
         strip2.setPixelColor(i, Black);
       }
       strip.show();
       strip2.show();
       CurrentSequenceStep = 0;
       IdleFlag = true;
       break;  
       
       default:
       break;
    }
}


/*********************************/
/* Music Maker functions */



////////////////////////////////////////////////////////////////////////////

// END OF LINE //







