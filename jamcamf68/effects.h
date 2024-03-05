/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/

#pragma once


unsigned char newImage[jwidth][jheight][3];
//unsigned char oldImage[jwidth][jheight][3];
unsigned short avgImage[jwidth][jheight][3];
unsigned char difference[jwidth][jheight];
unsigned char distance[jwidth][jheight];
unsigned char flames[jwidth + 2][jheight + 1];
unsigned char flamesC[255][3];
//long long frames[jwidth][jheight];



void calibrateWall()
{
  calibration = 100;
}

void setEffect(int i)
{
  Serial.printf("Set Effect %d\n", i);
  effect = i;
}

void initEffects()
{

  //  for (int x = 0; x < jwidth; x++)
  //    for (int y = 0; y < jheight; y++)
  //      frames[x][y] = 0;

  for (int x = 0; x < jwidth + 2; x++)
    for (int y = 0; y < jheight + 1; y++)
      flames[x][y] = 0;

  for (int y = 0; y < disheight; y++) {
    for (int x = 0; x < diswidth; x++ ) {
      Out[x][y][2] = 0;
      Out[x][y][1] = 0;
      Out[x][y][0] = 0;
    }
  }

  for (int c = 0; c < 255; c++)
  {
    //flamesC[c][0] = c;                     //rgb
    //flamesC[c][1] = (c * c) / (255);
    //flamesC[c][2] = (c * c) / (255 * 8);

    flamesC[c][0] = (c * c) / (255);          //grb
    flamesC[c][1] = c;
    flamesC[c][2] = (c * c) / (255 * 8);
  }
}

void recalcAverage()
{
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      avgImage[x][y][0] = ((avgImage[x][y][0]) * 128 + newImage[x][y][0] * 128) >> 8;
      avgImage[x][y][1] = ((avgImage[x][y][1]) * 128 + newImage[x][y][1] * 128) >> 8;
      avgImage[x][y][2] = ((avgImage[x][y][2]) * 128 + newImage[x][y][2] * 128) >> 8;
    }
}

void calcDifference()
{
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      difference[x][y] = min(255,
                             abs((avgImage[x][y][0]) - newImage[x][y][0]) +
                             abs((avgImage[x][y][1]) - newImage[x][y][1]) +
                             abs((avgImage[x][y][2]) - newImage[x][y][2]));
    }
}


void downSampleBig(unsigned char *frame) {

  for (int y = 0; y < jheight ; y++) //jwidth
  {
    for (int x = 0; x < jwidth; x++) //jheight
    {

      int r = 0;
      int g = 0;
      int b = 0;

      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 2; i++)
        {       
          /*
          int px =  x * 2 + i;
          int py = y * 2 + j;
          
          unsigned char p0 = frame[(py * 80 + px) * 3 + 2];
          unsigned char p1 = frame[(py * 80 + px) * 3 + 1];
          unsigned char p2 = frame[(py * 80 + px) * 3 + 0];

          r += p0;
          g += p1;
          b += p2;
          */
          
          int px =  x * 2 + i;
          int py80 = 80 * (y * 2 + j);
          r += frame[( py80 + px) * 3 + 2];
          g += frame[( py80 + px) * 3 + 1];
          b += frame[( py80 + px) * 3 + 0];
          
        }
      }

      newImage[x][y][0] = b >> 2;
      newImage[x][y][1] = g >> 2;
      newImage[x][y][2] = r >> 2 ;

    }
  }
}


void downSample(unsigned char *frame)
{
  for (int y = 0; y < jwidth; y++)
  {
    for (int x = 0; x < jheight; x++)
    {
      int r = 0;
      int g = 0;
      int b = 0;
      for (int j = 1; j < 5; j++)        // move 6x6 block to 1x1 bit or 96x96 frame to 16x16 display (skip bits 0 and 6)
        for (int i = 1; i < 5; i++)
        {
          unsigned char p[3];
          getPixel(x * 6 + i, y * 6 + j, frame, p);
          r += p[0];
          g += p[1];
          b += p[2];
        }
      newImage[x][y][0] = r >> 4 ; //r >> 4; 6x6=36 / 2^4=16
      newImage[x][y][1] = g >> 4;
      newImage[x][y][2] = b >> 4;
      //Serial.printf("%02d / %02d : r %02d, g %02d, b %02d\n",y,x,newImage[x][y][0],newImage[x][y][1],newImage[x][y][2]);
    }
  }
  /*
    for(int y = 0; y < jwidth; y++)
    for(int x = 0; x < jheight; x++)
      getPixel(x * 4 + 2, y * 4 + 2, frame, newImage[x][y]);*/
}

void downSamplefast(unsigned char *frame)
{
  for (int y = 0; y < jwidth; y++)
  {
    for (int x = 0; x < jheight; x++)
    {
      getPixel(x * 6 + 3, y * 6 + 3, frame, newImage[x][y]);
      //Serial.printf("%02d / %02d : r %02d, g %02d, b %02d\n",y,x,newImage[x][y][0],newImage[x][y][1],newImage[x][y][2]);
    }
  }
}

void calcDistance()
{
  for (int y = 0; y < jwidth; y++)
    for (int x = 0; x < jheight; x++)
      if (difference[x][y] > 128)
        distance[x][y] = 0;
      else
        distance[x][y] = 255;

  for (int i = 0; i < jheight; i++)
  {
    for (int y = 1; y < jwidth - 1; y++)
      for (int x = 1; x < jheight - 1; x++)
        distance[x][y] = min(min(min(distance[y][x - 1], distance[y][x + 1]), min(distance[y - 1][x], distance[y + 1][x])) + 1, distance[x][y] + 0);
    for (int y = 1; y < jwidth - 1; y++)
    {
      distance[y][0] = min(min(distance[y][1], min(distance[y - 1][0], distance[y + 1][0])) + 1, distance[y][0] + 0);
      distance[y][jheight - 1] = min(min(distance[y][jheight - 2], min(distance[y - 1][jheight - 1], distance[y + 1][jheight - 1])) + 1, distance[y][jheight - 1] + 0);
    }
    for (int x = 1; x < jheight - 1; x++)
    {
      distance[0][x] = min(min(min(distance[0][x - 1], distance[0][x + 1]), distance[1][x]) + 1, distance[0][x] + 0);
      distance[jwidth - 1][x] = min(min(min(distance[jwidth - 1][x - 1], distance[jwidth - 1][x + 1]), distance[jwidth - 1][x]) + 1, distance[jwidth - 1][x] + 0);
    }
    distance[0][0] = min(min(distance[1][0], distance[0][1]) + 1, distance[0][0] + 0);
    distance[jheight - 1][0] = min(min(distance[jheight - 2][0], distance[jheight - 1][1]) + 1, distance[jheight - 1][0] + 0);
    distance[0][jwidth - 1] = min(min(distance[1][jwidth - 1], distance[0][jwidth - 2]) + 1, distance[0][jwidth - 1] + 0);
    distance[jheight - 1][jwidth - 1] = min(min(distance[jheight - 2][jwidth - 1], distance[jheight - 1][jwidth - 2]) + 1, distance[jheight - 1][jwidth - 1] + 0);
  }
}

void scroll (char* msg);
void scroll_and_wait (char* msg);

void next_effect() {

  char char_effect[8];
  sprintf( char_effect, "%d", effect);
  scroll_and_wait ( char_effect );
  /*
    for (int y = 0; y < disheight; y++) {
      for (int x = 0; x < diswidth; x++) {
        int sample = pixelMapdis(x, y);
        if ( y == effect) {
          if (sample != -1) gfxsetLED(sample, 50, 0, 0);
          delay(20);
        } else {
          if (sample != -1) gfxsetLED(sample, 0, 0, 0);
        }
      }
    }
    delay(1000);
    for (int y = 0; y < disheight; y++) {
      for (int x = 0; x < diswidth; x++) {
        int sample = pixelMapdis(x, y);
        gfxsetLED(sample, 0, 0, 0);
      }
    }
  */
}
extern int start_record;

void toggle_record() {
  for (int y = 0; y < disheight; y++) {
    for (int x = 0; x < diswidth; x++) {
      int sample = pixelMapdis(x, y);
      if ( x == y) {
        if (start_record == 1 ) {
          if (sample != -1) gfxsetLED(sample, 50, 0, 0);
        } else {
          if (sample != -1) gfxsetLED(sample, 0, 50, 0);
        }
        delay(20);
      } else {
        if (sample != -1) gfxsetLED(sample, 0, 0, 0);
      }
    }
  }
  FastLED.show();

  delay(100);
}

void colors()
{
  Serial.printf("Red (rgb) or Green (grb) - first\n");
  for (int y = 0; y < disheight; y++) {
    for (int x = 0; x < diswidth; x++) {
      int sample = pixelMapdis(x, y);
      //Serial.printf("x %d / y %d, %03d \n",x,y,sample);
      if (sample != -1) gfxsetLED(sample, 0, 0, 0);
    }
  }
  FastLED.show();

  for (int y = 0; y < disheight; y++) {
    for (int x = 0; x < diswidth; x++) {
      int sample = pixelMapdis(x, y);
      // if (sample != -1) gfxsetLED(sample, diswidth, 16 - disheight, (diswidth * 2) % 2);
      int a = 0;
      int b = 0;
      int c = 0;
      if (y >= 0 && y < 3) {
        a = 13; b = 0; c = 0;
      }
      if (y >= 3 && y < 6) {
        a = 0; b = 13; c = 0;
      }
      if (y >= 6 && y < 9) {
        a = 0; b = 0; c = 13;
      }
      if (y >= 9 && y < 11) {
        a = 13; b = 13; c = 0;
      }
      if (y >= 11 && y < 13) {
        a = 0; b = 13; c = 13;
      }
      if (y >= 13 && y < 15) {
        a = 13; b = 0; c = 13;
      }
      if (y >= 15 && y < 16) {
        a = 13; b = 13; c = 13;
      }


      if (sample != -1) gfxsetLED(sample, a, b, c);
      if ( y == 0 && x < 3) {
        delay(1);
      } else {
        delay(1);
      }
    }
    FastLED.show();

    delay(25);
  }
  //delay(100);
  /*
    for (int y = 0; y < disheight; y++) {
      for (int x = 0; x < diswidth; x++) {
        int sample = pixelMapdis(x, y);
        // if (sample != -1) gfxsetLED(sample, diswidth, 16 - disheight, (diswidth * 2) % 2);
        if (sample != -1) gfxsetLED(sample, 15, 0, 0);
        if ( y == 0 && x < 3) {
          delay(1);
        } else {
          delay(1);
        }
      }
    }
    delay(50);

    Serial.printf("Green or Red - second\n");
    for (int y = 0; y < disheight; y++) {
      for (int x = 0; x < diswidth; x++) {
        int sample = pixelMapdis(x, y);
        if (sample != -1) gfxsetLED(sample, 0, 15, 0);
        delay(1);
      }
    }
    delay(50);
    Serial.printf("Blue - third\n");
    for (int y = 0; y < disheight; y++) {
      for (int x = 0; x < diswidth; x++) {
        int sample = pixelMapdis(x, y);
        if (sample != -1) gfxsetLED(sample, 0, 0, 15);
        delay(1);
      }
  */
  delay(50);
  for (int y = 0; y < disheight; y++) {
    for (int x = 0; x < diswidth; x++) {
      int sample = pixelMapdis(x, y);
      if (sample != -1) gfxsetLED(sample, 0, 0, 0);
    }
  }
  FastLED.show();

  delay(200);

}

void calib()
{
  recalcAverage();
  //Serial.printf("Calibration %d", calibration);

  for (int y = 0; y < disheight; y++)
  {
    for (int x = 0; x < diswidth; x++)
    {
      int sample = pixelMapdis(x, y);
      //Serial.printf("x %d / y %d, %03d \n",x,y,sample);
      //if (sample != -1) gfxsetLED(sample, 128, 128, 128);
      if (sample != -1) gfxsetLED(sample, 64, 64, 64);
    }
  }
  FastLED.show();

  calibration--;
  //Serial.println();
}

void processDifference()
{
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      int sample = pixelMap(x, y);
      int d = min(128, difference[x][y] - 128) * 2 - 1;
      if (d > 0)
      {
        if (sample != -1) gfxsetLED(sample, d, d, d);
        Out[x][y][1] = d;
        Out[x][y][2] = d;
        Out[x][y][0] = d;
        //gfxsetLED(sample, newImage[x][y][0], newImage[x][y][1], newImage[x][y][2]);
      }
      else
      {
        if (sample != -1) gfxsetLED(sample, 0, 0, 0);
        Out[x][y][1] = 0;
        Out[x][y][2] = 0;
        Out[x][y][0] = 0;
      }
    }
      int yy = 15;
  for (int x = 0; x < disheight; x++) //jheight
  {
    int sample = pixelMapdis(x, yy);
    gfxsetLED(sample, 0, 0, 0);          //ver56 gfxsetLEDGamma(sample, 0, 0, 0);
  }
  FastLED.show();

}

void processDistance()
{
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      int sample = pixelMap(x, y);
      int r = min(distance[x][y] * 16, 255);
      if (sample != -1) gfxsetLED(sample, rainbow[r][0], rainbow[r][1], rainbow[r][2]);
      Out[x][y][1] = rainbow[r][0];
      Out[x][y][2] = rainbow[r][1];
      Out[x][y][0] = rainbow[r][2];
    }
      int yy = 15;
  for (int x = 0; x < disheight; x++) //jheight
  {
    int sample = pixelMapdis(x, yy);
    gfxsetLED(sample, 0, 0, 0);          //ver56 gfxsetLEDGamma(sample, 0, 0, 0);
  }
  FastLED.show();

}

void processFlames()
{
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
      if (difference[x][y] > 128)
        flames[x + 1][y] = (rand() & 127) + 128;
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      int f = flames[x + 1][y] = (flames[x][y + 1] + flames[x + 1][y + 1] + flames[x + 2][y + 1]) >> 2;
      int sample = pixelMap(x, y);
      if (sample != -1) gfxsetLED(sample, flamesC[f][0], flamesC[f][1], flamesC[f][2]); // rgb
      Out[x][y][1] = flamesC[f][0];
      Out[x][y][2] = flamesC[f][1];
      Out[x][y][0] = flamesC[f][2];
      //gfxsetLED(sample, flamesC[f][1], flamesC[f][0], flamesC[f][2]); // grb - colors corrected in init effects
    }
      int yy = 15;
  for (int x = 0; x < disheight; x++) //jheight
  {
    int sample = pixelMapdis(x, yy);
    gfxsetLED(sample, 0, 0, 0);          //ver56 gfxsetLEDGamma(sample, 0, 0, 0);
  }
  FastLED.show();

}

void processShape()
{
  static unsigned long lastBeat = 0;
  unsigned long t = millis();
  unsigned long d = t - lastBeat;
  if (d >= 1236)
  {
    lastBeat = t;
  }
  for (int y = 0; y < jheight; y++)
    for (int x = 0; x < jwidth; x++)
    {
      int sample = pixelMap(x, y);
      if (difference[x][y] > 128)
      {
        if (sample != -1) gfxsetLED(sample, 255, 255, 255);
        Out[x][y][1] = 255;
        Out[x][y][2] = 255;
        Out[x][y][0] = 255;
      }
      else
      {
        int r = d / 32 - distance[x][y];
        if (r < 0) r = -r;
        r = min(r * 64, 255);
        if (sample != -1) gfxsetLED(sample, 255 - r, 0, 0);
        Out[x][y][1] = 255 - r;
        Out[x][y][2] = 0;
        Out[x][y][0] = 0;
      }
    }
      int yy = 15;
  for (int x = 0; x < disheight; x++) //jheight
  {
    int sample = pixelMapdis(x, yy);
    gfxsetLED(sample, 0, 0, 0);          //ver56 gfxsetLEDGamma(sample, 0, 0, 0);
  }
  FastLED.show();
}

void showImage()
{

  int yy = 15;
  for (int x = 0; x < disheight; x++) //jheight
  {
    int sample = pixelMapdis(x, yy);
    gfxsetLED(sample, 0, 0, 0);          //ver56 gfxsetLEDGamma(sample, 0, 0, 0);
  }

  int how_many = 0;
  int skipped = 0;
  for (int y = 0; y < jheight; y++){
    for (int x = 0; x < jwidth; x++)
    {
      int sample = pixelMap(x, y);
      //Serial.printf("x %d, y %d,  map %d\n",x,y,sample);
      if (sample != -1) {
        //gfxsetLEDGamma(sample, newImage[x][y][1], newImage[x][y][0], newImage[x][y][2]); // grb
        //ver56 gfxsetLEDGamma(sample, newImage[x][y][1], newImage[x][y][2], newImage[x][y][0]);
        
        int a = newImage[x][y][1] ;
        int b = newImage[x][y][2] ;
        int c = newImage[x][y][0] ;
        int x = a + b + c;

 //         a = a / 3;
 //         b = b / 3;
 //         c = c / 3;
        
        if (x > 128) {

          how_many++;
        }
        gfxsetLED(sample, a, b, c);
      } else {
        skipped++;
      }
    }
  }
  //Serial.printf("How many overpower %d and skipped %d\n",how_many,skipped);
  //FastLED.show_at_max_brightness_for_power();
  FastLED.show();

}
/*
  void processFrames()
  {
  const int col[][3] = {{0, 0, 127}, {127, 0, 0}, {0, 127, 0}, {64, 0, 64}};
  for (int y = 0; y < jheight; y++) {
    for (int x = 0; x < jwidth; x++)
    {
      int sample = pixelMap(x, y);
      int bit = difference[x][y] > 128 ? 1 : 0;
      bit = 0;
      if (bit)
      {
        if (sample != -1) gfxsetLED(sample, col[255][0], col[255][1], col[255][2]);
        Out[x][y][1] = col[255][0];
        Out[x][y][2] = col[255][1];
        Out[x][y][0] = col[255][2];
        frames[x][y] = (frames[x][y] << 1) | bit;
      }
      else
      {
        int b = 0;
        for (; b < 4; b++)
          if ((frames[x][y] >> (b * 16 + 15)) & 1)
          {
            if (sample != -1) gfxsetLED(sample, col[b][0], col[b][1], col[b][2]);
            Out[x][y][1] = col[b][0];
            Out[x][y][2] = col[b][1];
            Out[x][y][0] = col[b][2];
            break;
          }
        if (b == 4) {
          if (sample != -1) gfxsetLED(sample, 0, 0, 0);
          Out[x][y][1] = 0;
          Out[x][y][2] = 0;
          Out[x][y][0] = 0;
        }
        frames[x][y] = frames[x][y] << 1;
      }
    }
  }
  }
*/
void life_start();

void processImage()
{
  if (calibration)
  {
    calib();
    return;
  }
  switch (effect)
  {
    case 0: //video
      showImage();
      break;
    case 1: //differnce
      calcDifference();
      processDifference();
      break;
    case 2: //distance
      calcDifference();
      calcDistance();
      processDistance();
      break;
    case 3: //pulsing
      calcDifference();
      calcDistance();
      processShape();
      break;
    case 4: //flames
      calcDifference();
      processFlames();
      break;
    case 5: //shadow
      //calcDifference();
      //processFrames();
      break;
  }
}
