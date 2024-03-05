/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/

#define diswidth 16
#define disheight 16

class class_snake {
  public:
    int snake_ticks = 0;
    int snake_reset = 0;
    int snake_winner = 0;
    int snake_immortal = 0;
    int snake_tron = 0;

    int snakeA_random_red = 0;
    int snakeA_random_blue = 0;
    int snakeA_random_green = 0;
    int snakeA_changes = 0;
    int snakeA_len = 0;

    int snakeAXY[20][2];
    int snakeAXYdir[2];

    int snakeB_random_red = 0;
    int snakeB_random_blue = 0;
    int snakeB_random_green = 0;
    int snakeB_changes = 0;
    int snakeB_len = 0;

    int snakeBXY[20][2];
    int snakeBXYdir[2];

    int snake_upd_delay ;
    int snake_next_update;

    class_snake(int delay, int immortal, int tron) {
      snake_upd_delay = delay;
      snake_immortal = immortal;
      snake_tron = tron;
      snake_next_update = millis() + snake_upd_delay;
      snake_clear();
      snake_start();

    }
    ~class_snake() {
      snake_clear();
    }

    void snake_newrandom() {
      int x = random(0, 10);
      if (x > 5) {
        snakeA_random_red = random(80, 100);
        snakeA_random_blue = 100 - snakeA_random_red;
        snakeA_random_green = 0;
        snakeB_random_red = random(0, 20);
        snakeB_random_blue = 0;
        snakeB_random_green = 100 - snakeB_random_red;
      } else if (x > 2) {
        snakeA_random_red = 0;
        snakeA_random_blue = random(80, 100);
        snakeA_random_green = 100 - snakeA_random_blue;
        snakeB_random_red = random(0, 20);
        snakeB_random_blue = 0;
        snakeB_random_green = 100 - snakeB_random_red;
      } else {
        snakeA_random_red = random(80, 100);
        snakeA_random_blue = 100 - snakeA_random_red;
        snakeA_random_green = 0;
        snakeB_random_red = 0;
        snakeB_random_blue = random(80, 100);
        snakeB_random_green = 100 - snakeB_random_blue;
      }
      //Serial.printf("%d %d %d %d", snakeA_random_red, snakeA_random_blue, snakeA_random_green, x);
      /*
            int tot;

            tot = snakeA_random_red + snakeA_random_blue + snakeA_random_green;
            snakeA_random_red = 100.0 *  snakeA_random_red / tot ;
            snakeA_random_blue = 100.0 *  snakeA_random_blue / tot ;
            snakeA_random_green = 100.0 *  snakeA_random_green / tot ;

            tot = snakeB_random_red + snakeB_random_blue + snakeB_random_green;
            snakeB_random_red = 100.0 *  snakeB_random_red / tot ;
            snakeB_random_blue = 100.0 *  snakeB_random_blue / tot ;
            snakeB_random_green = 100.0 *  snakeB_random_green / tot ;

            //Serial.printf("--- %d %d %d %d\n", snakeA_random_red, snakeA_random_blue, snakeA_random_green, x);

      */
    }

    void snake_clear() {
      for (int i = 0; i < diswidth; i++) {
        for (int j = 0; j < disheight; j++) {
          //Out[i][j][2] = 255;
          //Out[i][j][1] = 255;
          //Out[i][j][0] = 255;
          
          //int sample = pixelMapdis(i, j);
          //gfxsetLED(sample, 0, 0, 0);
          both(i,j,0,0,0);
        }
      }
      //FastLED.show();

    }

    void snake_start() {
      snake_clear();
      snake_reset = -1;
      snake_winner = 0;
      snake_newrandom();

      if (random(0, 2) == 0) {
        if (random(0, 2) == 0) {
          snakeAXYdir[0] = 1;
          snakeAXYdir[1] = 0;
        } else {
          snakeAXYdir[0] = 0;
          snakeAXYdir[1] = 1;
        }
      } else {
        if (random(0, 2) == 0) {
          snakeAXYdir[0] = -1;
          snakeAXYdir[1] = 0;
        } else {
          snakeAXYdir[0] = 0;
          snakeAXYdir[1] = -1;
        }
      }

      if (random(0, 2) == 0) {
        if (random(0, 2) == 0) {
          snakeBXYdir[0] = 1;
          snakeBXYdir[1] = 0;
        } else {
          snakeBXYdir[0] = 0;
          snakeBXYdir[1] = 1;
        }
      } else {
        if (random(0, 2) == 0) {
          snakeBXYdir[0] = -1;
          snakeBXYdir[1] = 0;
        } else {
          snakeBXYdir[0] = 0;
          snakeBXYdir[1] = -1;
        }
      }

      snakeAXY[0][0] = random(3, 13);
      snakeAXY[0][1] = random(3, 13);
      snakeA_len = 3;
      for (int i = 0; i < snakeA_len ; i++) {
        snakeAXY[i][0] = snakeAXY[0][0] + i;
        snakeAXY[i][1] = snakeAXY[0][1] + 0;
      }
      snakeBXY[0][0] = random(3, 13);
      snakeBXY[0][1] = random(3, 13);
      snakeB_len = 3;
      for (int i = 0; i < snakeB_len ; i++) {
        snakeBXY[i][0] = snakeBXY[0][0] + i;
        snakeBXY[i][1] = snakeBXY[0][1] + 0;
      }
    }

    void snake_display() {

      if (!snake_tron) snake_clear();

      for (int k = 0; k < snakeA_len; k++) {
        float factor ;
        //snake[snakeAXY[k][0]][snakeAXY[k][1]] = 1;
        if (k == 0 ) {
          factor = 2.0;
        } else if (k == 1) {
          factor = 1.0;
        } else {
          factor = 0.5;
        }
        both(snakeAXY[k][0], snakeAXY[k][1],snakeA_random_green * factor, snakeA_random_red * factor, snakeA_random_blue * factor, snakeA_random_green * 2, snakeA_random_red * 2, snakeA_random_blue * 2);
        //Out[snakeAXY[k][0]][snakeAXY[k][1]][1] = snakeA_random_green * 2;
        //Out[snakeAXY[k][0]][snakeAXY[k][1]][2] = snakeA_random_red * 2;
        //Out[snakeAXY[k][0]][snakeAXY[k][1]][0] = snakeA_random_blue * 2;
        //int sample = pixelMapdis(snakeAXY[k][0], snakeAXY[k][1]);
        

       
        //gfxsetLED(sample, snakeA_random_green * factor, snakeA_random_red * factor, snakeA_random_blue * factor);
      }
      for (int k = 0; k < snakeB_len; k++) {
        float factor;
        //factor = -0.05 * k + 1;
        if (k == 0 ) {
          factor = 2.0;
        } else if (k == 1) {
          factor = 1.0;
        } else {
          factor = 0.5;
        }
        //    snake[snakeBXY[k][0]][snakeBXY[k][1]] = 2;
        both(snakeBXY[k][0], snakeBXY[k][1],snakeB_random_green * factor, snakeB_random_red * factor, snakeB_random_blue * factor, snakeB_random_green * 2, snakeB_random_red * 2, snakeB_random_blue * 2);
        //Out[snakeBXY[k][0]][snakeBXY[k][1]][1] = snakeB_random_green * 2;
        //Out[snakeBXY[k][0]][snakeBXY[k][1]][2] = snakeB_random_red * 2;
        //Out[snakeBXY[k][0]][snakeBXY[k][1]][0] = snakeB_random_blue * 2;

        //int sample = pixelMapdis(snakeBXY[k][0], snakeBXY[k][1]);
        //gfxsetLED(sample, snakeB_random_green * factor, snakeB_random_red * factor, snakeB_random_blue * factor);

      }
      FastLED.show();

      //Serial.printf("\n");
      delay(0);
    }

    void snake_update(int immortal, int tron) {
      snake_immortal = immortal;
      snake_tron = tron;
      //Serial.printf("Update dirX %d, dir %d, X %d Y %d\n", snakeAXYdir[0], snakeAXYdir[1], snakeAXY[0][0], snakeAXY[0][1]);
      if (millis() > snake_upd_delay) {
        snake_ticks++;
        if (snake_ticks == snake_reset) {
          snake_start();
        }


        if (snake_ticks % 25 == 0) {
          if (snakeA_len < 20) snakeA_len++;
          if (snakeB_len < 20) snakeB_len++;
        }

        if (snakeAXYdir[0] == 1 ||  snakeAXYdir[0] == -1) {
          if (snakeAXY[0][1] == 15) {
            if (random(0, 4) == 0) {
              snakeAXYdir[1] = -1;
              snakeAXYdir[0] = 0;
            }
          }
          if (snakeAXY[0][1] == 0) {
            if (random(0, 4) == 0) {
              snakeAXYdir[1] = 1;
              snakeAXYdir[0] = 0;
            }
          }
          if (random(0, 6) == 0) {
            if (random(0, 2) == 0) {
              if (snakeAXY[0][1] < 13) {
                snakeAXYdir[1] = 1;
                snakeAXYdir[0] = 0;
              } else {
                snakeAXYdir[1] = -1;
                snakeAXYdir[0] = 0;
              }
            } else {
              if (snakeAXY[0][1] > 3) {
                snakeAXYdir[1] = -1;
                snakeAXYdir[0] = 0;
              } else {
                snakeAXYdir[1] = 1;
                snakeAXYdir[0] = 0;
              }
            }
          }

        } else  if (snakeAXYdir[1] == 1 ||  snakeAXYdir[1] == -1) {
          if (snakeAXY[0][0] == 15) {
            if (random(0, 4) == 0) {
              snakeAXYdir[0] = -1;
              snakeAXYdir[1] = 0;
            }
          }
          if (snakeAXY[0][0] == 0) {
            if (random(0, 4) == 0) {
              snakeAXYdir[0] = 1;
              snakeAXYdir[1] = 0;
            }
          }
          if (random(0, 6) == 0) {
            if (random(0, 2) == 0) {
              if (snakeAXY[0][0] < 13) {
                snakeAXYdir[0] = 1;
                snakeAXYdir[1] = 0;
              } else {
                snakeAXYdir[0] = -1;
                snakeAXYdir[1] = 0;
              }
            } else {
              if (snakeAXY[0][0] > 3) {
                snakeAXYdir[0] = -1;
                snakeAXYdir[1] = 0;
              } else {
                snakeAXYdir[0] = 1;
                snakeAXYdir[1] = 0;
              }
            }
          }
        }

        if (snakeAXYdir[0] == 1) {
          if (snakeAXY[0][0] == 15) {
            snakeAXYdir[0] = 0;
            if (snakeAXY[0][1] == 0) {
              snakeAXYdir[1] = 1;
            } else if (snakeAXY[0][1] == 15) {
              snakeAXYdir[1] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeAXYdir[1] = 1;
              } else {
                snakeAXYdir[1] = -1;
              }
            }
          }
        } else if (snakeAXYdir[0] == -1) {
          if (snakeAXY[0][0] == 0) {
            snakeAXYdir[0] = 0;
            if (snakeAXY[0][1] == 0) {
              snakeAXYdir[1] = 1;
            } else if (snakeAXY[0][1] == 15) {
              snakeAXYdir[1] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeAXYdir[1] = 1;
              } else {
                snakeAXYdir[1] = -1;
              }
            }
          }
        } else if (snakeAXYdir[1] == 1) {
          if (snakeAXY[0][1] == 15) {
            snakeAXYdir[1] = 0;
            if (snakeAXY[0][0] == 0) {
              snakeAXYdir[0] = 1;
            } else if (snakeAXY[0][0] == 15) {
              snakeAXYdir[0] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeAXYdir[0] = 1;
              } else {
                snakeAXYdir[0] = -1;
              }
            }
          }
        } else if (snakeAXYdir[1] == -1) {
          if (snakeAXY[0][1] == 0) {
            snakeAXYdir[1] = 0;
            if (snakeAXY[0][0] == 0) {
              snakeAXYdir[0] = 1;
            } else if (snakeAXY[0][0] == 15) {
              snakeAXYdir[0] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeAXYdir[0] = 1;
              } else {
                snakeAXYdir[0] = -1;
              }
            }
          }
        }
        ////////////////////////////
        if (snakeBXYdir[0] == 1 ||  snakeBXYdir[0] == -1) {
          if (snakeBXY[0][1] == 15) {
            if (random(0, 4) == 0) {
              snakeBXYdir[1] = -1;
              snakeBXYdir[0] = 0;
            }
          }
          if (snakeBXY[0][1] == 0) {
            if (random(0, 4) == 0) {
              snakeBXYdir[1] = 1;
              snakeBXYdir[0] = 0;
            }
          }
          if (random(0, 5) == 0) {
            if (random(0, 2) == 0) {
              if (snakeBXY[0][1] < 13) {
                snakeBXYdir[1] = 1;
                snakeBXYdir[0] = 0;
              }
            } else {
              if (snakeBXY[0][1] > 3) {
                snakeBXYdir[1] = -1;
                snakeBXYdir[0] = 0;
              }
            }
          }
        } else  if (snakeBXYdir[1] == 1 ||  snakeBXYdir[1] == -1) {
          if (snakeBXY[0][0] == 15) {
            if (random(0, 4) == 0) {
              snakeBXYdir[0] = -1;
              snakeBXYdir[1] = 0;
            }
          }
          if (snakeBXY[0][0] == 0) {
            if (random(0, 4) == 0) {
              snakeBXYdir[0] = 1;
              snakeBXYdir[1] = 0;
            }
          }
          if (random(0, 5) == 0) {
            if (random(0, 2) == 0) {
              if (snakeBXY[0][0] < 13) {
                snakeBXYdir[0] = 1;
                snakeBXYdir[1] = 0;
              }
            } else {
              if (snakeBXY[0][0] > 2) {
                snakeBXYdir[0] = -1;
                snakeBXYdir[1] = 0;
              }
            }
          }
        }

        if (snakeBXYdir[0] == 1) {
          if (snakeBXY[0][0] == 15) {
            snakeBXYdir[0] = 0;
            if (snakeBXY[0][1] == 0) {
              snakeBXYdir[1] = 1;
            } else if (snakeBXY[0][1] == 15) {
              snakeBXYdir[1] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeBXYdir[1] = 1;
              } else {
                snakeBXYdir[1] = -1;
              }
            }
          }
        } else if (snakeBXYdir[0] == -1) {
          if (snakeBXY[0][0] == 0) {
            snakeBXYdir[0] = 0;
            if (snakeBXY[0][1] == 0) {
              snakeBXYdir[1] = 1;
            } else if (snakeBXY[0][1] == 15) {
              snakeBXYdir[1] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeBXYdir[1] = 1;
              } else {
                snakeBXYdir[1] = -1;
              }
            }
          }
        } else if (snakeBXYdir[1] == 1) {
          if (snakeBXY[0][1] == 15) {
            snakeBXYdir[1] = 0;
            if (snakeBXY[0][0] == 0) {
              snakeBXYdir[0] = 1;
            } else if (snakeBXY[0][0] == 15) {
              snakeBXYdir[0] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeBXYdir[0] = 1;
              } else {
                snakeBXYdir[0] = -1;
              }
            }
          }
        } else if (snakeBXYdir[1] == -1) {
          if (snakeBXY[0][1] == 0) {
            snakeBXYdir[1] = 0;
            if (snakeBXY[0][0] == 0) {
              snakeBXYdir[0] = 1;
            } else if (snakeBXY[0][0] == 15) {
              snakeBXYdir[0] = -1;
            } else {
              if (random(0, 2) == 0) {
                snakeBXYdir[0] = 1;
              } else {
                snakeBXYdir[0] = -1;
              }
            }
          }
        }

        for (int i = snakeA_len - 1; i > 0; i--) {    // advance snake
          snakeAXY[i][0] = snakeAXY[i - 1][0] ;
          snakeAXY[i][1] = snakeAXY[i - 1][1];
        }
        snakeAXY[0][0] = snakeAXY[0][0] + snakeAXYdir[0];
        snakeAXY[0][1] = snakeAXY[0][1] + snakeAXYdir[1];

        if (snake_ticks > 20) {                    // only kill snake >20
          for (int i = snakeA_len; i > 0; i--) {
            if ((snakeBXY[0][0] == snakeAXY[i][0])  && (snakeBXY[0][1] == snakeAXY[i][1])) {
              if (snake_immortal) {
                if (random(0, 2) == 0) {    // immortal snakes keep the tails half the time
                  snakeA_len = i;
                }
              } else {
                snakeA_len = i;
              }
              //Serial.printf("A at len %d at %d %d\n", i, snakeAXY[i][0], snakeAXY[i][1]);
            }
          }
          if (! snake_immortal) {
            bool headfirst = false;
            if (snakeAXYdir[1] != 0 && snakeBXYdir[1] != 0 ) headfirst = true;
            if (snakeAXYdir[0] != 0 && snakeBXYdir[0] != 0 ) headfirst = true;
            
            if ((snakeBXY[0][0] == snakeAXY[0][0])  && (snakeBXY[0][1] == snakeAXY[0][1]) && headfirst ) {
              Serial.printf("Crash\n");
              if (snakeA_len >= snakeB_len) {
                snake_winner = 1;
                snakeB_len = 0;
              } else {
                snake_winner = 2;
                snakeA_len = 0;
              }
            }
          }
        }
        ///

        for (int i = snakeB_len - 1; i > 0; i--) {    // advance
          snakeBXY[i][0] = snakeBXY[i - 1][0] ;
          snakeBXY[i][1] = snakeBXY[i - 1][1];
        }
        snakeBXY[0][0] = snakeBXY[0][0] + snakeBXYdir[0];
        snakeBXY[0][1] = snakeBXY[0][1] + snakeBXYdir[1];

        if (snake_ticks > 20) {
          for (int i = snakeB_len; i > 0; i--) {
            if ((snakeAXY[0][0] == snakeBXY[i][0])  && (snakeAXY[0][1] == snakeBXY[i][1])) {
              if (snake_immortal) {
                if (random(0, 2) == 0) {
                  snakeB_len = i;
                }
              } else {
                snakeB_len = i;
              }
              //Serial.printf("B at len %d at %d %d\n", i, snakeBXY[i][0], snakeBXY[i][1]);
            }
          }

          if (! snake_immortal) {
            bool headfirst = false;
            if (snakeAXYdir[1] != 0 && snakeBXYdir[1] != 0 ) headfirst = true;
            if (snakeAXYdir[0] != 0 && snakeBXYdir[0] != 0 ) headfirst = true;
            if ((snakeBXY[0][0] == snakeAXY[0][0])  && (snakeBXY[0][1] == snakeAXY[0][1]) && headfirst) {
              //Serial.printf("Crash\nCrash\nB loses\n\n");
              Serial.printf("Crash2\n");
              if (snakeA_len > snakeB_len) {
                snake_winner = 1;
                snakeB_len = 0;
              } else {
                snake_winner = 2;
                snakeA_len = 0;
              }
            }

            if (snakeA_len > snakeB_len + 10) {
              snake_winner = 1;
              snakeB_len = 0;
            } else  if (snakeB_len > snakeA_len + 10) {
              snake_winner = 2;
              snakeA_len = 0;
            }
          }
          if (snake_winner == 1) {
            delay(200);
            for (int i = 0 + 5; i < diswidth - 5; i++) {
              for (int j = 0 + 5; j < disheight - 5; j++) {
                int sample = pixelMapdis(i, j);
                float factor = 0.2;
                //gfxsetLED(sample, snakeA_random_green * factor, snakeA_random_red * factor, snakeA_random_blue * factor);
                both(i,j, snakeA_random_green * factor, snakeA_random_red * factor, snakeA_random_blue * factor, snakeA_random_green * 2, snakeA_random_red * 2, snakeA_random_blue * 2 );
              }
            }
            bmp_frame_avail = true;
            FastLED.show();

            delay(400);
            snake_start();
            return;
          } else if (snake_winner == 2) {
            delay(200);
            for (int i = 0 + 5; i < diswidth - 5; i++) {
              for (int j = 0 + 5; j < disheight - 5; j++) {
                //int sample = pixelMapdis(i, j);
                float factor = 0.2;
                both(i,j, snakeB_random_green * factor, snakeB_random_red * factor, snakeB_random_blue * factor, snakeB_random_green * 2, snakeB_random_red * 2, snakeB_random_blue * 2);
                //gfxsetLED(sample, snakeB_random_green * factor, snakeB_random_red * factor, snakeB_random_blue * factor);
              }
            }
            bmp_frame_avail = true;
            FastLED.show();

            delay(400);
            snake_start();
            return;
          }
        }

        if (snake_immortal && snake_ticks == 10000) {
          delay(400);
          snake_start();

        }
      } else {
        Serial.printf("s");
      }
    }

  private:
    int private_int ;
};
