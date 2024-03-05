/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/

#define diswidth 16
#define disheight 16

class class_life {
  public:

    unsigned char life[diswidth ][disheight ];
    unsigned char life_next[diswidth ][disheight ];
    unsigned char life_prev[diswidth ][disheight ];

    int random_red = 0;
    int random_blue = 0;
    int random_green = 0;

    int life_upd_delay;
    int life_upd_next;

    class_life(int delay) {
      life_upd_delay = delay;
      life_upd_next = millis() + life_upd_delay;
      life_clear();
      life_start();

    }

    ~class_life() {
      life_clear();
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

    void life_newrandom() {
      if (random(0,4) > 1) {
        random_red = random(0, 30);
      } else random_red = 0;
      if (random(0,4) > 1) {
        random_blue = random(0, 30);
      } else random_blue = 0;
      if (random(0,4) > 1) {
        random_green = random(0, 30);
      } else random_green = 0;
      if (random_red + random_blue + random_green == 0){
        random_red = random(0, 30);
      }
    }

    void life_clear() {
      for (int i = 0; i < disheight; i++) {
        for (int j = 0; j < diswidth; j++) {
          life[j][i] = 0;
          int sample = pixelMapdis(j, i);
          gfxsetLED(sample, 0, 0, 0);
        }
      }
        FastLED.show();

    }

    void life_seed() {

      for (int i = 0; i < disheight ; i++) {
        for (int j = 0; j < diswidth; j++) {
          if (random(0, 9) > 3) {
            life[j][i] = 0;
          } else {
            life[j][i] = 1;
          }
        }
      }
    }

    void life_glider() {

      life[1][2] = 1;
      life[2][3] = 1;
      life[3][1] = 1;
      life[3][2] = 1;
      life[3][3] = 1;

      int x = random(3, 6);

      life[1][2 + x] = 1;
      life[2][3 + x] = 1;
      life[3][1 + x] = 1;
      life[3][2 + x] = 1;
      life[3][3 + x] = 1;

    }

    void life_inter() {

      int x = random(0, 7);
      int y = random(0, 7);
      life[5 + x][4 + y] = 1;
      life[6 + x][4 + y] = 1;
      life[4 + x][5 + y] = 1;
      life[5 + x][5 + y] = 1;
      life[5 + x][6 + y] = 1;
    }

    void life_update() {
      if (millis() > life_upd_next) {
        for (int i = 0; i < disheight ; i++) {
          for (int j = 0; j < diswidth ; j++) {
            int alive = 0;
            
            // arduino module returns negative numbers !!!
            alive += life[(j+diswidth - 1) % diswidth][(i+disheight - 1) % disheight];
            alive += life[(j+diswidth - 1) % diswidth][i + 0];
            alive += life[(j+diswidth - 1) % diswidth][(i + 1) % disheight];
            alive += life[j + 0][(i+disheight - 1) % disheight];
            //alive += life[i+0][i+0]; this is me
            alive += life[j + 0][(i + 1) % disheight];
            alive += life[(j + 1) % diswidth][(i+disheight - 1) % disheight];
            alive += life[(j + 1) % diswidth][i + 0];
            alive += life[(j + 1) % diswidth][(i + 1) % disheight];

            //if (j == 0 && i == 6){
            //  Serial.printf("alive %d, j-1diswidth %d lifej-1diswidth %d\n",alive,(j-1)%diswidth,life[(j - 1) % diswidth][(i - 1) % disheight]);
            //}

            life_next[j][i] = 0;
            if (alive < 2) life_next[j][i] = 0; // 0 or 1 is dead
            if (alive == 2) {
              if (life[j][i] > 0) {
                life_next[j][i] = 1;  // 2 neighors survive
              } else {
                life_next[j][i] = 0;
              }
            }
            if (alive == 3) life_next[j][i] = 1;  // 3 neighbors born or survive
            if (alive > 3) life_next[j][i] = 0;   // 3+ is dead
          }
        }

        int diff = 0;
        int ddiff = 0;

        for (int i = 0; i < disheight ; i++) {
          for (int j = 0; j < diswidth ; j++) {
            if (life_prev[j][i] != life_next[j][i]) ddiff = 1;
          }
        }

        for (int i = 0; i < disheight ; i++) {
          for (int j = 0; j < diswidth ; j++) {
            if (life[j][i] != life_next[j][i]) diff = 1;
            life_prev[j][i] = life[j][i];
            life[j][i] = life_next[j][i];
          }
        }

        if (diff == 1 && ddiff == 0) diff = 0;

        if (!diff) {
          life_clear();
          life_newrandom();
          if (random(0, 9) > 7) {
            life_glider();
            life_inter();
          } else if (random(0, 9) > 6) {
            life_inter();
          } else {
            life_seed();
          }
        }
      } else {
        Serial.printf("l");
      }
    }

    void life_display() {
      for (int i = 0; i < disheight ; i++) {
        for (int j = 0; j < diswidth ; j++) {
          //int sample = pixelMap(row , col );
          int sample = pixelMapdis(j, i);


          if (life[j][i] > 0) {
            //gfxsetLEDGamma(sample, newImage[y][x][1], newImage[y][x][0], newImage[y][x][2]); // grb
            /*
            if (j == 0) {
              if (i == 6 || i == 10){
              gfxsetLED(sample, 0, 22, 0);
              } else {
                gfxsetLED(sample, 22, 0, 0);
              }
            } else {
              gfxsetLED(sample, random_green, random_red, random_blue);
            }
            */
            both(j,i,random_green, random_red, random_blue, 4);
            //gfxsetLED(sample, random_green, random_red, random_blue);
            //Out[j][i][2] =  random_red * 4;
            //Out[j][i][1] =  random_green * 4;
            //Out[j][i][0] =  random_blue * 4;
          }
          else {
            both(j,i,0,0,0);
            //gfxsetLED(sample, 0, 0, 0);
            //Out[j][i][0] =  255;
            //Out[j][i][1] =  255;
            //Out[j][i][2] =  255;
          }
        }
      }
        FastLED.show();

    }



    void life_start() {
      Serial.printf("Starting life ...\n");
      life_seed();
      life_glider();
      life_update();
      life_newrandom();
      int xx = 0;


      life_display();
      life_update();

    }
  private:
    int private_int;

};
