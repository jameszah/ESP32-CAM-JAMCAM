/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024

   Original from here - heavily modified
   
   https://github.com/bitluni/ESP32Lib
   
*/

#pragma once
#include <math.h>
long sinTab[256];

void calculateSinTab()
{
  for (int i = 0; i < 256; i++)
  {
    sinTab[i] = int(sin(M_PI / 128.f * i) * 256);
  }
}

float smoothstep(float x)
{
  x = max(0.f, min(x, 1.f));
  return (3 * x * x) - (2 * x * x * x);
}

float rsqrt(float number)
{
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return y;
}

int rainbow[256][3];
void calcRainbow()
{
  const float cb[][3] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}};
  for (int i = 0; i < 256; i++)
  {
    //interpolate the colors from the cb array and calculate the R5G5B5 color
    float s = 6.f / 256 * i;
    int n = int(s);
    float f = s - n;
    float fi = (1 - f);
    const float *cf = cb[n];
    const float *cfn = cb[(n + 1) % 6];
    int r = int((fi * cf[0] * 255) + (f * cfn[0] * 255));
    int g = int((fi * cf[1] * 255) + (f * cfn[1] * 255));
    int b = int((fi * cf[2] * 255) + (f * cfn[2] * 255));

    rainbow[i][0] = r;   //rgb
    rainbow[i][1] = g;
    rainbow[i][2] = b;

    //rainbow[i][0] = g;   //grb
    //rainbow[i][1] = r;
    //rainbow[i][2] = b;
  }
}

float rdist(int x, int y, float x2, float y2)
{
  return rsqrt((x - x2) * (x - x2) + (y - y2) * (y - y2) + 1);
}

float dist(int x, int y, float x2, float y2)
{
  return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
}

void getPixel(int x, int y, unsigned char *image, unsigned char *pixel)
{
  //int p = image[(y * 160 + x) * 2 + 1] | (image[(y * 160 + x) * 2] << 8);
  int p = image[(y * 96 + x) * 2 + 1] | (image[(y * 96 + x) * 2] << 8);
  pixel[0] = (p >> 11) << 3;
  pixel[1] = ((p >> 5) & 0b111111) << 2;
  pixel[2] = (p & 0b11111) << 3;
}


void getPixel240240(int x, int y, unsigned char *image, unsigned char *pixel)
{
  //int p = image[(y * 160 + x) * 2 + 1] | (image[(y * 160 + x) * 2] << 8);
  //int p = image[(y * 96 + x) * 2 + 1] | (image[(y * 96 + x) * 2] << 8);
  int p = image[(y * 240 + x) * 2 + 1] | (image[(y * 240 + x) * 2] << 8);

  pixel[0] = (p >> 11) << 3;
  pixel[1] = ((p >> 5) & 0b111111) << 2;
  pixel[2] = (p & 0b11111) << 3;
}


void getPixel888240240(int x, int y, unsigned char *image, unsigned char *pixel)
{
  //int p = image[(y * 160 + x) * 2 + 1] | (image[(y * 160 + x) * 2] << 8);
  //int p = image[(y * 96 + x) * 2 + 1] | (image[(y * 96 + x) * 2] << 8);
  int p = image[(y * 240 + x) * 2 + 1] | (image[(y * 240 + x) * 2] << 8);

  pixel[0] = image[(y * 240 + x) * 3 + 2];
  pixel[1] = image[(y * 240 + x) * 3 + 1];
  pixel[2] = image[(y * 240 + x) * 3 + 0];
}

/*
  int pixelMap(int x, int y)         //original bitluni with panels and 40x30
  {
  if(x < 0 || x >= 40 || y < 0 || y >= 30)
    return -1;
  x = 39 - x;
  int panel = 1 - (x / 20) + (y / 15) * 2;
  int row = (y % 15);
  int col = ((row & 1) == 0) ? 19 - (x % 20) : x % 20;
  return panel * 300 + row * 20 + col;
  }
*/

int pixelMap(int xx, int yy)
{

  if (xx < 4 || xx >= jwidth  || yy < 0 || yy >= jheight)
    return -1;
  if (xx % 2 == 1 || yy % 2 == 1)
    return -1;
    
  int x = xx / 2;
  if (x < 2 || x > 18){
    return -1;
  } else {
    x = x - 3; //from 2
  }
 
  int y = yy /2; 
 
  //x = 15 - x;
   //y = 15 - y;
  
  int row = (y % disheight);
  int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
  return  row * diswidth + col;

//  int row = (y % 16);
//  int col = ((row & 1) == 0) ? 16 - 1 - (x % 16) : x % 16;
//  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
//  return  row * 16 + col;
}

int pixelMapdis(int x, int y)
{
  if (x < 0 || x >= diswidth  || y < 0 || y >= disheight)
    return -1;
  
  //x = 15 - x;
   //y = 15 - y;
  
  int row = (y % disheight);
  int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
  return  row * diswidth + col;

//  int row = (y % 16);
//  int col = ((row & 1) == 0) ? 16 - 1 - (x % 16) : x % 16;
//  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
//  return  row * 16 + col;
}

int pixelMapold(int y, int x)
{
  if (x < 0 || x >= jheight  || y < 0 || y >= jwidth)
    return -1;
  
  //x = 15 - x;
   //y = 15 - y;
  
  int row = (y % jwidth);
  int col = ((row & 1) == 0) ? jheight - 1 - (x % jheight) : x % jheight;
  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
  return  row * jheight + col;

//  int row = (y % 16);
//  int col = ((row & 1) == 0) ? 16 - 1 - (x % 16) : x % 16;
//  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
//  return  row * 16 + col;
}

int pixelMap240240(int x, int y)
{
  if (x < 0 || x >= 16 || y < 0 || y >= 16)
    return -1;
  x = 15 - x;

  int row = (y % 16);
  int col = ((row & 1) == 0) ? 16 - 1 - (x % 16) : x % 16;
  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
  return  row * 16 + col;
}
