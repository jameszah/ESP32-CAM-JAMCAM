/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/
    
#define diswidth 16
#define disheight 16

class class_Clive {
  public:
    int clive_active;

    unsigned char clive[diswidth][disheight];
    int clive_delay[diswidth][disheight];
    int clive_next_update[diswidth][disheight];

    int clive_random_red = 0;
    int clive_random_blue = 0;
    int clive_random_green = 0;
    int clive_changes = 0;

    int clive_upd_delay;
    int clive_upd_next;

    class_Clive (int delay)  { //(int diswidth, int disheight) {
      clive_upd_delay = delay;
      clive_upd_next = millis() + clive_upd_delay;
      Serial.printf("constructor\n");
      clive_clear();
      clive_start();
    }

    ~class_Clive() {
      clive_clear();
    }

    int pixelMapdis(int x, int y)
    {
      if (x < 0 || x >= diswidth  || y < 0 || y >= disheight)
        return -1;

      int row = (y % disheight);
      int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
      //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
      return  row * diswidth + col;
    }

    void clive_clear() {
      for (int i = 0; i < diswidth; i++) {
        for (int j = 0; j < disheight; j++) {
          //int sample = pixelMap(i, j);
          int sample = pixelMapdis(i, j);
          gfxsetLED(sample, 0, 0, 0);
        }
      }
        FastLED.show();

    }

    void clive_newrandom() {
      int x = random(0, 3);
      if (x == 0) {
        clive_random_red = random(0, 100);
      } else if (x == 1) {
        clive_random_blue = random(0, 100);
      } else {
        clive_random_green = random(0, 100);
      }
      //Serial.printf("%d %d %d %d", clive_random_red, clive_random_blue, clive_random_green, x);
      int tot = clive_random_red + clive_random_blue + clive_random_green;
      clive_random_red = 75.0 *  clive_random_red / tot ;
      clive_random_blue = 75.0 *  clive_random_blue / tot ;
      clive_random_green = 75.0 *  clive_random_green / tot ;
      //Serial.printf("--- %d %d %d %d\n", clive_random_red, clive_random_blue, clive_random_green, x);
    }

    void clive_start() {
      clive_newrandom();

      for (int i = 0; i < diswidth; i++) {
        for (int j = 0; j < disheight; j++) {
          clive_delay[i][j] = random(400, 600);
          clive_next_update[i][j] = millis() + clive_delay[i][j] - 250;
          if (random(0, 9) > 4) {
            both (i,j,255,255,255);
            ///int sample = pixelMapdis(i, j);
            ///gfxsetLED(sample, 0, 0, 0);
            clive[i][j] = 0;
            ///Out[i][j][0] = 255;
            ///Out[i][j][1] = 255;
            ///Out[i][j][2] = 255;
          } else {
            both (i,j,clive_random_green, clive_random_red, clive_random_blue);
            //int sample = pixelMapdis(i, j);
            //gfxsetLED(sample, clive_random_green, clive_random_red, clive_random_blue);
            clive[i][j] = 1;
            //Out[i][j][2] = clive_random_red;
            //Out[i][j][1] =  clive_random_green;
            //Out[i][j][0] =  clive_random_blue;
          }
        }
      }
    }

    void clive_update() {
      if (millis() > clive_upd_next) {
        clive_upd_next = millis() + clive_upd_delay;

        for (int i = 0; i < diswidth; i++) {
          for (int j = 0; j < disheight; j++) {

            if (clive_next_update[i][j] < millis()) {
              //clive_next_update[i][j] = clive_next_update[i][j] + clive_delay[i][j];
              clive_next_update[i][j] = millis() + clive_delay[i][j];
              clive_changes++;
              if (clive_changes % 1024 == 0) {
                clive_newrandom();
              }
              if (clive[i][j] == 1) {
                clive[i][j] = 0;
                both(i,j,0,0,0);
                //int sample = pixelMapdis(i, j); // int sample = pixelMap(i, j);
                //gfxsetLED(sample, 0, 0, 0);
                //Out[i][j][0] = 255;
                //Out[i][j][1] = 255;
                //Out[i][j][2] = 255;

              } else {
                clive[i][j] = 1;
                both(i,j,clive_random_green, clive_random_red, clive_random_blue);
                //int sample = pixelMapdis(i, j); //int sample = pixelMap(i, j);
                //gfxsetLED(sample, clive_random_green, clive_random_red, clive_random_blue);
                //Out[i][j][2] = clive_random_red;
                //Out[i][j][1] =  clive_random_green;
                //Out[i][j][0] =  clive_random_blue;
              }
            }
          }
        }
          FastLED.show();

      } else {
        Serial.print("x");
      }
    }

  private:
    int private_int;

};
