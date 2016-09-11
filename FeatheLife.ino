// Adapted from:
// Code Example for jolliFactory's Bi-color 16X16 LED Matrix Conway's Game of Life example 1.0
// and Adafruit NeoPixel Example 'simple'

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define Width 8
#define Height 4

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      (Width * Height)

#define DELAY           250   // Delay between cycles (ms)
#define MAXGENERATIONS  100   // Maximum number of cycles allowed before restarting
#define MAXBLANK 10           // Maximum number of blank cycles before restarting
#define MAXSTATIC 10          // Maximum number of cycles that are exactly the same before restarting

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void hsv2rgb(unsigned int hue, unsigned int sat, unsigned int val, \
unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness );

int idx=0;
int noOfGeneration = 0;
int iCount = 0;
unsigned int hue;
int allOffCount = 0;  // tracking how many cycles all pixels are off
int sameCount = 0;   // tracking how many cycles all pixels the same as the prior cycle
byte sameBuffer[NUMPIXELS];  

byte t1[16][16];                      
byte t2[16][16];
byte last[16][16];



//**********************************************************************************************************************************************************
void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  
  randomize(t1); 
}



//**********************************************************************************************************************************************************
void loop()
{

  static bool onoff = false;
  
  if (idx++ > MAXGENERATIONS || allOffCount > MAXBLANK || sameCount > MAXSTATIC)   //limit no. of generations for display
  {
    allOffCount = 0;
    sameCount = 0;
    hue = random(360);
    randomize(t1);   
    noOfGeneration = 0;     
    idx=0;
  }

  onoff = !onoff;
  digitalWrite(13, onoff?HIGH:LOW);
  compute_previous_generation(t1,t2);
  compute_neighbouring_cells(t1,t2);
  compute_next_generation(t1,t2);
  display(t1);
  delay(DELAY);
}



//**********************************************************************************************************************************************************
void clear_window()
{
  pixels.clear();
  pixels.show();
}



//**********************************************************************************************************************************************************
void display(byte t1[16][16])
{
  byte i,j;

  int offCount = 0;
  byte newBuffer[NUMPIXELS];
  uint32_t c;  
  for(i=0; i<(Width); i++)
  {
    for(j=0; j<Height; j++)
    {  
      byte nCount = countNeighbors(t1,i,j);
      if (t1[i][j])
      {
        newBuffer[j*Width+i] = 1;
        byte r, g, b;
        unsigned int bright = 10 + 1 << nCount; // 2^nCount makes the scaling logrithmic, so the eye can see the differences
        if (bright > 255) bright = 255; 
        hsv2rgb(hue,255,255,&r,&g,&b,bright);
        c = pixels.Color(r,g,b);
      }
      else
      {
        c = pixels.Color(0,0,0);
        newBuffer[j*Width+i] = 0;
        offCount++;
      }
      pixels.setPixelColor(j*Width+i,c);
    }     
  }
  pixels.show();
  if (offCount == NUMPIXELS) allOffCount++;
  if (memcmp(newBuffer,sameBuffer,NUMPIXELS) == 0) sameCount++;
  memcpy(sameBuffer,newBuffer,NUMPIXELS);
  
}

//**********************************************************************************************************************************************************
void compute_previous_generation(byte t1[16][16],byte t2[16][16])
{
  byte i,j;

  for(i=0;i<Width;i++)
  {
    for(j=0;j<Height;j++)
    {
      t2[i][j]=t1[i][j];
      last[i][j]=t1[i][j];
    }
  }
}



//**********************************************************************************************************************************************************
void compute_next_generation(byte t1[16][16],byte t2[16][16])
{
  byte i,j;

  for(i=0;i<Width;i++)
  {
    for(j=0;j<Height;j++)
    {
      t1[i][j]=t2[i][j];
    }
  }
  
  noOfGeneration++;
  Serial.println(noOfGeneration);
}



//**********************************************************************************************************************************************************
void compute_neighbouring_cells(byte t1[16][16],byte t2[16][16])   //To Re-visit - does not seems correct
{
  byte i,j,a;

  for(i=0;i<Width;i++)
  {
    for(j=0;j<Height;j++)
    {
      a = countNeighbors(t1,i,j);
      
      if((t1[i][j]==0)&&(a==3)){t2[i][j]=1;}                   // populate if 3 neighours around it
      if((t1[i][j]==1)&&((a==2)||(a==3))){t2[i][j]=1;}         // stay alive if 2 or 3 neigbours around it
      if((t1[i][j]==1)&&((a==1)||(a==0)||(a>3))){t2[i][j]=0;}  // die if only one neighbour or over-crowding with 4 or more neighours
    }
  }
}


byte countNeighbors(byte t1[16][16],byte i, byte j)
{
  byte a;
  if((i==0)&&(j==0))
  {
    a=t1[i][j+1]+t1[i+1][j]+t1[i+1][j+1]+t1[i][Height-1]+t1[i+1][Height-1]+t1[Width-1][j]+t1[Width-1][j+1]+t1[Width-1][Height-1];
  }

  if((i!=0)&&(j!=0)&&(i!=(Width-1))&&(j!=(Height-1)))
  {
    a=t1[i-1][j-1]+t1[i-1][j]+t1[i-1][j+1]+t1[i][j+1]+t1[i+1][j+1]+t1[i+1][j]+t1[i+1][j-1]+t1[i][j-1];
  }
  
  if((i==0)&&(j!=0)&&(j!=(Height-1)))
  {
    a=t1[i][j-1]+t1[i+1][j-1]+t1[i+1][j]+t1[i+1][j+1]+t1[i][j+1]+t1[Width-1][j-1]+t1[Width-1][j]+t1[Width-1][j+1];
  }

  if((i==0)&&(j==(Height-1)))
  {
    a=t1[i][j-1]+t1[i+1][j-1]+t1[i+1][j]+t1[i][0]+t1[i+1][0]+t1[Width-1][0]+t1[Width-1][j]+t1[Width-1][j-1];
  }
  
  if((i==(Width-1))&&(j==0))
  {
    a=t1[i-1][j]+t1[i-1][j+1]+t1[i][j+1]+t1[i][Height-1]+t1[i-1][Height-1]+t1[0][j]+t1[0][j+1]+t1[0][Height-1];
  }
  
  if((i==(Width-1))&&(j!=0)&&(j!=(Height-1)))
  {
    a=t1[i][j-1]+t1[i][j+1]+t1[i-1][j-1]+t1[i-1][j]+t1[i-1][j+1]+t1[0][j]+t1[0][j-1]+t1[0][j+1];
  }
  
  if((i==(Width-1))&&(j==(Height-1)))
  {
    a=t1[i][j-1]+t1[i-1][j-1]+t1[i-1][j]+t1[0][j]+t1[0][j-1]+t1[i][0]+t1[i-1][0]+t1[0][0];
  }

  if((i!=0)&&(i!=(Width-1))&&(j==0))
  {
    a=t1[i-1][j]+t1[i-1][j+1]+t1[i][j+1]+t1[i+1][j+1]+t1[i+1][j]+t1[i][Height-1]+t1[i-1][Height-1]+t1[i+1][Height-1];
  }

  if((i!=0)&&(i!=(Width-1))&&(j==(Height-1)))
  {
    a=t1[i-1][j]+t1[i-1][j-1]+t1[i][j-1]+t1[i+1][j-1]+t1[i+1][j]+t1[i][0]+t1[i-1][0]+t1[i+1][0];
  }
  return a;
}


//**********************************************************************************************************************************************************
void randomize(byte t1[16][16])
{
  byte i,j;
  randomSeed(millis());
  for(i=0;i<Width;i++)
  {
    for(j=0;j<Height;j++)
    {
      t1[i][j]=random(2);
    }
  }
}

/******************************************************************************
 * Tis function converts HSV values to RGB values, scaled from 0 to maxBrightness
 * 
 * The ranges for the input variables are:
 * hue: 0-360
 * sat: 0-255
 * lig: 0-255
 * 
 * The ranges for the output variables are:
 * r: 0-maxBrightness
 * g: 0-maxBrightness
 * b: 0-maxBrightness
 * 
 * r,g, and b are passed as pointers, because a function cannot have 3 return variables
 * Use it like this:
 * int hue, sat, val; 
 * unsigned char red, green, blue;
 * // set hue, sat and val
 * hsv2rgb(hue, sat, val, &red, &green, &blue, maxBrightness); //pass r, g, and b as the location where the result should be stored
 * // use r, b and g.
 * 
 * (c) Elco Jacobs, E-atelier Industrial Design TU/e, July 2011.
 * 
 *****************************************************************************/


void hsv2rgb(unsigned int hue, unsigned int sat, unsigned int val, \
              unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness ) { 
  unsigned int H_accent = hue/60;
  unsigned int bottom = ((255 - sat) * val)>>8;
  unsigned int top = val;
  unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
  unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

  switch(H_accent) {
  case 0:
    *r = top;
    *g = rising;
    *b = bottom;
    break;

  case 1:
    *r = falling;
    *g = top;
    *b = bottom;
    break;

  case 2:
    *r = bottom;
    *g = top;
    *b = rising;
    break;

  case 3:
    *r = bottom;
    *g = falling;
    *b = top;
    break;

  case 4:
    *r = rising;
    *g = bottom;
    *b = top;
    break;

  case 5:
    *r = top;
    *g = bottom;
    *b = falling;
    break;
  }
  // Scale values to maxBrightness
  *r = *r * maxBrightness/255;
  *g = *g * maxBrightness/255;
  *b = *b * maxBrightness/255;
}



