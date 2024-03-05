/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024

   Original from here - heavily modified
   
   https://github.com/bitluni/ESP32Lib

*/

//unsigned char timeImage[jwidth][jheight];

void display_let_col_pos(int letter, int col, int position) {
  //int onerow = font[letter][row];
  for (int row = 0; row < 7; row++) {
    int sample = pixelMapdis(position, row );
    if (sample != -1) {
      int onerow = font[letter][row];
      if (bitRead(onerow, 5 - col ) > 0) {
        //disp[col ][row ] = 1;
        gfxsetLED(sample, 33, 33, 33);
      } else {
        gfxsetLED(sample, 0, 0, 0);
      }
      delay(10);
    }
  }
}

void testtext() {
  for (int pos = 0 ; pos < 16; pos++) {
    //int pos = 5;
    for (int i = 0 ; i < 6; i++) {
      display_let_col_pos(66, i, pos + i);

    }
    delay(0);
  }
  Serial.printf("done\n");
}

void disp (int letter, int i, int j) {
  for (int row = 0; row < 7; row++) { //display at 0,0 with left blank space
    int onerow = font[letter][row];
    for (int col = 0; col < 6; col++) {
      int sample = pixelMap(col + 1, row + j);
      if (sample != -1) {
        if (bitRead(onerow, 5 - col ) > 0) {
          //disp[col + i][row + j] = 1;
          gfxsetLED(sample, 33, 33, 33);
        } else {
          gfxsetLED(sample, 0, 0, 0);
        }
      }
    }
  }
}

void do_text() {

  scroll(localip);
  return;
  testtext();
  delay(1000);

  int disp[jwidth][jheight];
  int ascii ;

  ascii = 66;
  for (int x = 66; x < 68; x++) { //48 .. 122

    for (int row = 0; row < 7; row++) { //display at 0,0 with left blank space
      int onerow = font[x][row];
      for (int col = 0; col < 6; col++) {
        int sample = pixelMapdis(col, row);
        if (sample != -1) {
          if (bitRead(onerow, 5 - col ) > 0) {
            disp[col][row] = 1;
            gfxsetLED(sample, 33, 33, 33);
          } else {
            gfxsetLED(sample, 0, 0, 0);
          }
        }
      }
    }
    delay(700);
  }
}

void disptime(int secbars) {
  if (!scroll_active) {
    time(&now);
    String x = ctime(&now);
    //   Mon Jul 16 02:03:55 1987\n\0
    // x.substring(11, 17).toCharArray(nowtime, 6); // x.length() + 1);

    tm *timeinfo = localtime(&now);
    int secs = timeinfo->tm_sec;
    int mins = timeinfo->tm_min;
    int hrs = timeinfo->tm_hour;

    for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 16; j++) {
        //int sample = pixelMapdis(j, i);
        //Out[j][i][1] = 0;
        //Out[j][i][2] = 0;
        //Out[j][i][0] = 0;
        //gfxsetLED(sample, 0, 0, 0);
        both(i,j,0,0,0);
      }
    }

    if (secbars == 1) {
      for (int dig = 0; dig < 2; dig++) {
        //int letter = (int)x[11 + dig] - 48;
        int letter = (int)x[11 + dig] ;
        for (int row = 0; row < 6; row++) {
          //int onerow = font4x6[letter * 6 + row];
          int onerow = full4x6[letter * 6 + row];
          for (int col = 0; col < 4; col++) {
            if (bitRead(onerow, 7 - col ) > 0) {
              //timeImage[dig * 4 + col][row] = 1;
              Out[dig * 4 + col - 1][row][1] = 0;
              Out[dig * 4 + col - 1][row][2] = 133;
              Out[dig * 4 + col - 1][row][0] = 0;

              int sample = pixelMapdis(dig * 4 + col - 1, row);
              gfxsetLED(sample, 0, 13, 0);
            } else {
              //timeImage[dig * 4 + col][row] = 0;
              //int sample = pixelMapdis(dig * 4 + col, row);
              //     gfxsetLED(sample, 0, 0, 0);
            }
          }
        }
      }

      // int sample = pixelMapdis(7, 1);
      // gfxsetLED(sample, 33, 33, 33);
      // sample = pixelMapdis(7, 2);
      // gfxsetLED(sample, 33, 33, 33);

      //  for (int i = 0; i < 7; i++){
      //    int sample = pixelMapdis(8, i);
      //    gfxsetLED(sample, 0, 0, 0);
      //  }
    }


    int sample;
    if (secbars == 1) {
      if (secs % 2 == 1) {
        sample = pixelMapdis(8, 3);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(7, 3);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(8, 2);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(7, 2);
        gfxsetLED(sample, 0, 0, 3);
      }

      for (int dig = 2; dig < 4; dig++) {
        int letter = (int)x[11 + dig + 1] ;
        for (int row = 0; row < 6; row++) {
          //int onerow = font4x6[letter * 6 + row];
          int onerow = full4x6[letter * 6 + row];
          for (int col = 0; col < 4; col++) {
            if (bitRead(onerow, 7 - col ) > 0) {
              //timeImage[dig * 4 + col][row] = 1;
              int sample = pixelMapdis(dig * 4 + col + 1 - 1, row);
              if (sample != -1) {
                gfxsetLED(sample, 0, 13, 0);
                Out[dig * 4 + col + 1 - 1][row][1] = 0; //rainbow[r][0];
                Out[dig * 4 + col + 1 - 1][row][2] = 133; //rainbow[r][1];
                Out[dig * 4 + col + 1 - 1][row][0] = 0; //rainbow[r][2];

              }
            } else {
              //timeImage[dig * 4 + col][row] = 0;
              int sample = pixelMapdis(dig * 4 + col + 1, row);
              //   if (sample != -1) gfxsetLED(sample, 0, 0, 0);
            }
          }
        }
      }

      if (  !scroll_active ) {

        sample = pixelMapdis(8, 9);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(7, 9);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(8, 8);
        gfxsetLED(sample, 0, 0, 3);
        sample = pixelMapdis(7, 8);
        gfxsetLED(sample, 0, 0, 3);

        for (int dig = 4; dig < 6; dig++) {
          int letter = (int)x[11 + dig + 2] ;
          for (int row = 0; row < 5; row++) {              // drop bottom pixel
            //int onerow = font4x6[letter * 6 + row];
            int onerow = full4x6[letter * 6 + row];
            for (int col = 0; col < 4; col++) {
              if (bitRead(onerow, 7 - col ) > 0) {
                //timeImage[dig * 4 + col][row] = 1;
                int sample = pixelMapdis((dig - 2) * 4 + col + 1 - 1, row + 6);
                if (sample != -1) {
                  gfxsetLED(sample, 0, 13, 0);
                  Out[(dig - 2) * 4 + col + 1 - 1][row + 6][1] = 0;
                  Out[(dig - 2) * 4 + col + 1 - 1][row + 6][2] = 133;
                  Out[(dig - 2) * 4 + col + 1 - 1][row + 6][0] = 0;

                }
              } else {
                //timeImage[dig * 4 + col][row] = 0;
                int sample = pixelMapdis(dig * 4 + col + 1, row);
                //   if (sample != -1) gfxsetLED(sample, 0, 0, 0);
              }
            }
          }
        }
      }
    }

    if (secbars == 2) {
      hrs = hrs % 12;
      for (int i = 0; i < 12; i++) {

        if (i < hrs) {
          for (int x = 0; x < 7; x++) {
            Out[i + 2][x][1] = 0;
            Out[i + 2][x][2] = 0;
            Out[i + 2][x][0] = 133;
            int sample = pixelMapdis(i + 2, x );
            if ((i + 1) % 4 == 0) {
              gfxsetLED(sample, 0, 0, 11);
            } else {
              gfxsetLED(sample, 2, 2, 2);
            }
          }
        }
      }

      for (int i = 0; i < 60; i++) {
        int x = i % 8;
        int numrows = (int) i / 8;
        //int sample = pixelMapdis(x+3, numrows+6);
        int sample = pixelMapdis(x , numrows + 8);

        if (i < mins) {
          if ( (i+1)%4 == 0 || (numrows+1)%4 == 0){
            gfxsetLED(sample, 1, 3, 0);
          } else {
            gfxsetLED(sample, 0, 3, 0);
          }
          Out[x ][numrows + 8][1] = 0;
          Out[x ][numrows + 8][2] = 133;
          Out[x ][numrows + 8][0] = 0;

        } else {
          //    gfxsetLED(sample, 0, 0, 0);
        }
      }

      for (int i = 0; i < 60; i++) {
        int x = i % 8;
        int numrows = (int) i / 8;
        //int sample = pixelMapdis(x+3, numrows+6);
        int sample = pixelMapdis(x + 8, numrows + 8);

        if (i < secs) {
          Out[x + 8][numrows + 8][1] = 133;
          Out[x + 8][numrows + 8][2] = 0;
          Out[x + 8][numrows + 8][0] = 0;
          if ( (i+1)%4 == 0 || (numrows+1)%4 == 0 ){
            gfxsetLED(sample, 3, 0, 1);
          } else {
            gfxsetLED(sample, 3, 0, 0);
          }

//          gfxsetLED(sample, 3, 0, 0);
        } else {
          //      gfxsetLED(sample, 0, 0, 0);
        }
      }
    }

    if (secbars == 44) {
      for (int i = 0; i < 24; i++) {
        int x = i % 10;
        int numrows = (int) i / 10;
        //int sample = pixelMapdis(x+3, numrows+6);
        int sample = pixelMapdis(numrows, x + 6);

        if (i < hrs) {
          Out[numrows][x + 6][1] = 0;
          Out[numrows][x + 6][2] = 0;
          Out[numrows][x + 6][0] = 133;

          gfxsetLED(sample, 0, 0, 3);
        } else {
          //   gfxsetLED(sample, 0, 0, 0);
        }
      }

      for (int i = 0; i < 60; i++) {
        int x = i % 10;
        int numrows = (int) i / 10;
        //int sample = pixelMapdis(x+3, numrows+6);
        int sample = pixelMapdis(numrows + 4, x + 6);

        if (i < mins) {
          gfxsetLED(sample, 0, 3, 0);
          Out[numrows + 4][x + 6][1] = 0;
          Out[numrows + 4][x + 6][2] = 133;
          Out[numrows + 4][x + 6][0] = 0;

        } else {
          //    gfxsetLED(sample, 0, 0, 0);
        }
      }

      for (int i = 0; i < 60; i++) {
        int x = i % 10;
        int numrows = (int) i / 10;
        //int sample = pixelMapdis(x+3, numrows+6);
        int sample = pixelMapdis(numrows + 10, x + 6);

        if (i < secs) {
          Out[numrows + 10][x + 6][1] = 133;
          Out[numrows + 10][x + 6][2] = 0;
          Out[numrows + 10][x + 6][0] = 0;

          gfxsetLED(sample, 3, 0, 0);
        } else {
          //      gfxsetLED(sample, 0, 0, 0);
        }
      }
    }

    if (secbars == 5) {

      if (secs % 2 == 1) {
        both (8, 0, 19, 19, 0);
        both (7, 1, 19, 19, 0);
        both (8, 1, 19, 19, 0);
        both (9, 1, 19, 19, 0);
        both (8, 2, 19, 19, 0);

        //sample = pixelMapdis(8, 0);  gfxsetLED(sample, 19, 19, 0);
        //sample = pixelMapdis(7, 1);  gfxsetLED(sample, 19, 19, 0);
        //sample = pixelMapdis(8, 1);  gfxsetLED(sample, 19, 19, 0);
        //sample = pixelMapdis(9, 1);  gfxsetLED(sample, 19, 19, 0);
        //sample = pixelMapdis(8, 2);  gfxsetLED(sample, 19, 19, 0);
      }
      if (hrs >= 5) {
        //both (0,4,0,19,0); //sample = pixelMapdis(0, 4);  gfxsetLED(sample, 0, 19, 0);
        both (1,4,0,19,0); //sample = pixelMapdis(1, 4);  gfxsetLED(sample, 0, 19, 0); 
        both (2,4,0,19,0); //sample = pixelMapdis(2, 4);  gfxsetLED(sample, 0, 19, 0);
        //both (0,5,0,19,0); //sample = pixelMapdis(0, 5);  gfxsetLED(sample, 0, 19, 0);
        both (1,5,0,19,0); //sample = pixelMapdis(1, 5);  gfxsetLED(sample, 0, 19, 0);
        both (2,5,0,19,0); //sample = pixelMapdis(2, 5);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs >= 10) {
        //both (4,4,0,19,0); //sample = pixelMapdis(4, 4);  gfxsetLED(sample, 0, 19, 0);
        both (5,4,0,19,0); //sample = pixelMapdis(5, 4);  gfxsetLED(sample, 0, 19, 0);
        both (6,4,0,19,0); //sample = pixelMapdis(6, 4);  gfxsetLED(sample, 0, 19, 0);
        //both (4,5,0,19,0); //sample = pixelMapdis(4, 5);  gfxsetLED(sample, 0, 19, 0);
        both (5,5,0,19,0); //sample = pixelMapdis(5, 5);  gfxsetLED(sample, 0, 19, 0);
        both (6,5,0,19,0); //sample = pixelMapdis(6, 5);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs >= 15) {
        //both (8,4,0,19,0); //sample = pixelMapdis(8, 4);  gfxsetLED(sample, 0, 19, 0);
        both (9,4,0,19,0); //sample = pixelMapdis(9, 4);  gfxsetLED(sample, 0, 19, 0);
        both (10,4,0,19,0); //sample = pixelMapdis(10, 4);  gfxsetLED(sample, 0, 19, 0);
        //both (8,5,0,19,0); //sample = pixelMapdis(8, 5);  gfxsetLED(sample, 0, 19, 0);
        both (9,5,0,19,0); //sample = pixelMapdis(9, 5);  gfxsetLED(sample, 0, 19, 0);
        both (10,5,0,19,0); //sample = pixelMapdis(10, 5);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs >= 20) {
        //both (12,4,0,19,0); //sample = pixelMapdis(12, 4);  gfxsetLED(sample, 0, 19, 0);
        both (13,4,0,19,0); //sample = pixelMapdis(13, 4);  gfxsetLED(sample, 0, 19, 0);
        both (14,4,0,19,0); //sample = pixelMapdis(14, 4);  gfxsetLED(sample, 0, 19, 0);
        //both (12,5,0,19,0); //sample = pixelMapdis(12, 5);  gfxsetLED(sample, 0, 19, 0);
        both (13,5,0,19,0); //sample = pixelMapdis(13, 5);  gfxsetLED(sample, 0, 19, 0);
        both (14,5,0,19,0); //sample = pixelMapdis(14, 5);  gfxsetLED(sample, 0, 19, 0);
      }

      if (hrs % 5 >= 1) {
        //both (0,7,0,19,0); //sample = pixelMapdis(0, 7);  gfxsetLED(sample, 0, 19, 0);
        both (1,7,0,19,0); //sample = pixelMapdis(1, 7);  gfxsetLED(sample, 0, 19, 0);
        both (2,7,0,19,0); //sample = pixelMapdis(2, 7);  gfxsetLED(sample, 0, 19, 0);
        //both (0,8,0,19,0); //sample = pixelMapdis(0, 8);  gfxsetLED(sample, 0, 19, 0);
        both (1,8,0,19,0); //sample = pixelMapdis(1, 8);  gfxsetLED(sample, 0, 19, 0);
        both (2,8,0,19,0); //sample = pixelMapdis(2, 8);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs % 5 >= 2) {
        //both (4,7,0,19,0); //sample = pixelMapdis(4, 7);  gfxsetLED(sample, 0, 19, 0);
        both (5,7,0,19,0); //sample = pixelMapdis(5, 7);  gfxsetLED(sample, 0, 19, 0);
        both (6,7,0,19,0); //sample = pixelMapdis(6, 7);  gfxsetLED(sample, 0, 19, 0);
        //both (4,8,0,19,0); //sample = pixelMapdis(4, 8);  gfxsetLED(sample, 0, 19, 0);
        both (5,8,0,19,0); //sample = pixelMapdis(5, 8);  gfxsetLED(sample, 0, 19, 0);
        both (6,8,0,19,0); //sample = pixelMapdis(6, 8);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs % 5 >= 3) {
        //both (8,7,0,19,0); //sample = pixelMapdis(8, 7);  gfxsetLED(sample, 0, 19, 0);
        both (9,7,0,19,0); //sample = pixelMapdis(9, 7);  gfxsetLED(sample, 0, 19, 0);
        both (10,7,0,19,0); //sample = pixelMapdis(10, 7);  gfxsetLED(sample, 0, 19, 0);
        //both (8,8,0,19,0); //sample = pixelMapdis(8, 8);  gfxsetLED(sample, 0, 19, 0);
        both (9,8,0,19,0); //sample = pixelMapdis(9, 8);  gfxsetLED(sample, 0, 19, 0);
        both (10,8,0,19,0); //sample = pixelMapdis(10, 8);  gfxsetLED(sample, 0, 19, 0);
      }
      if (hrs % 5 >= 4) {
        //both (12,7,0,19,0); //sample = pixelMapdis(12, 7);  gfxsetLED(sample, 0, 19, 0);
        both (13,7,0,19,0); //sample = pixelMapdis(13, 7);  gfxsetLED(sample, 0, 19, 0);
        both (14,7,0,19,0); //sample = pixelMapdis(14, 7);  gfxsetLED(sample, 0, 19, 0);
        //both (12,8,0,19,0); //sample = pixelMapdis(12, 8);  gfxsetLED(sample, 0, 19, 0);
        both (13,8,0,19,0); //sample = pixelMapdis(13, 8);  gfxsetLED(sample, 0, 19, 0);
        both (14,8,0,19,0); //sample = pixelMapdis(14, 8);  gfxsetLED(sample, 0, 19, 0);
      }

      if (mins >= 5) {
        both (3,10,19,19,0); //sample = pixelMapdis(3, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 10) {
        both (4,10,19,19,0); //sample = pixelMapdis(4, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 15) {
        both (5,10,0,19,0); //sample = pixelMapdis(5, 10);  gfxsetLED(sample, 0, 19, 0);
      }
      if (mins >= 20) {
        both (6,10,19,19,0); //sample = pixelMapdis(6, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 25) {
        both (7,10,19,19,0); //sample = pixelMapdis(7, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 30) {
        both (8,10,0,19,0); //sample = pixelMapdis(8, 10);  gfxsetLED(sample, 0, 19, 0);
      }
      if (mins >= 35) {
        both (9,10,19,19,0); //sample = pixelMapdis(9, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 40) {
        both (10,10,19,19,0); //sample = pixelMapdis(10, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 45) {
        both (11,10,0,19,0); //sample = pixelMapdis(11, 10);  gfxsetLED(sample, 0, 19, 0);
      }
      if (mins >= 50) {
        both (12,10,19,19,0); //sample = pixelMapdis(12, 10);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins >= 55) {
        both (13,10,19,19,0); //sample = pixelMapdis(13, 10);  gfxsetLED(sample, 19, 19, 0);
      }

      if (mins % 5 >= 1) {
        both (6,11,19,19,0); //sample = pixelMapdis(6, 11);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins % 5 >= 2) {
        both (7,11,19,19,0); //sample = pixelMapdis(7, 11);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins % 5 >= 3) {
        both (8,11,19,19,0); //sample = pixelMapdis(8, 11);  gfxsetLED(sample, 19, 19, 0);
      }
      if (mins % 5 >= 4) {
        both (9,11,19,19,0); //sample = pixelMapdis(9, 11);  gfxsetLED(sample, 19, 19, 0);
      }

/*
      if (secs >= 5) {
        both (3,13,0,0,19); //sample = pixelMapdis(3, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 10) {
        both (4,13,0,0,19); //sample = pixelMapdis(4, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 15) {
        both (5,13,19,0,0); //sample = pixelMapdis(5, 13);  gfxsetLED(sample, 19, 0, 19);
      }
      if (secs >= 20) {
        both (6,13,0,0,19); //sample = pixelMapdis(6, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 25) {
        both (7,13,0,0,19); //sample = pixelMapdis(7, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 30) {
        both (8,13,19,0,0); //sample = pixelMapdis(8, 13);  gfxsetLED(sample, 19, 0, 19);
      }
      if (secs >= 35) {
        both (9,13,0,0,19); //sample = pixelMapdis(9, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 40) {
        both (10,13,0,0,19); //sample = pixelMapdis(10, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 45) {
        both (11,13,19,0,0); //sample = pixelMapdis(11, 13);  gfxsetLED(sample, 19, 0, 19);
      }
      if (secs >= 50) {
        both (12,13,0,0,19); //sample = pixelMapdis(12, 13);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs >= 55) {
        both (13,13,0,0,19); //sample = pixelMapdis(13, 13);  gfxsetLED(sample, 0, 0, 19);
      }

      if (secs % 5 >= 1) {
        both (6,14,0,0,19); //sample = pixelMapdis(6, 14);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs % 5 >= 2) {
        both (7,14,0,0,19); //sample = pixelMapdis(7, 14);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs % 5 >= 3) {
        both (8,14,0,0,19); //sample = pixelMapdis(8, 14);  gfxsetLED(sample, 0, 0, 19);
      }
      if (secs % 5 >= 4) {
        both (9,14,0,0,19); //sample = pixelMapdis(9, 14);  gfxsetLED(sample, 0, 0, 19);
      }

*/
    }


    if (secbars == 3  ) {


      for (int i = 0; i < 60; i++) {
        int x = i % 15;
        int numrows = (int) i / 15;
        //int sample = pixelMapdis(x+3, numrows+6);
        //int sample = pixelMapdis(numrows + 10, x + 6);
        int sample = pixelMapdis(x + 0, numrows + 12);

        if (i < secs) {
          Out[x + 0][numrows + 12][1] = 133; //rainbow[r][0];
          Out[x + 0][numrows + 12][2] = 0; //rainbow[r][1];
          Out[x + 0][numrows + 12][0] = 0; //rainbow[r][2];

          gfxsetLED(sample, 3, 0, 0);
        } else {
          //      gfxsetLED(sample, 0, 0, 0);
        }
      }
    }

    FastLED.show();
    bmp_frame_avail = true;
  }
}

bool buff(int i, int j) {
  int ind = i * 6 + j;  //from 7
  return big[ind];
}
void buffstore(bool x, int i, int j) {
  int ind = i * 6 + j;  // from 7
  big[ind] = x;
}
void buffclear() {
  for (int i = 0; i < 600 * 6; i++) { // from 7
    big[i] = 0;
  }
}

void dispclear() {
  for (int i = 0; i < diswidth; i++) {
    for (int j = 0; j < disheight; j++) {
      int sample = pixelMapdis(i, j);
      if (sample != -1) gfxsetLED(sample, 0, 0, 0);
    }
  }
  FastLED.show();

}
/*
  void the_old_scroll_loop( void * parameter )
  {
  uint32_t ulNotifiedValue;
  Serial.print("scroll, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  new_scroll = false;

  dispclear();

  //Serial.printf("strelen %d\n", strlen(scroll_msg));
  for (int leng = 0; leng < strlen(scroll_msg) + 5 ; leng++) {
    int letter;
    if (leng < 2 || leng > (strlen(scroll_msg) + 1) ) {
      letter = 32;
    } else {
      letter = (int)scroll_msg[leng - 2];
    }

    //Serial.printf("%d >%c<\n", letter, scroll_msg[leng-2]);

    for (int row = 0; row < 7; row++) {
      int onerow = font[letter][row];
      for (int col = 0; col < 6; col++) {
        if (bitRead(onerow, 5 - col ) > 0) {

          buffstore(true, leng * 6 + col, row);
        } else {
          //bb buff[leng * 6 + col][row] = false;
          buffstore(false, leng * 6 + col, row);
        }
      }
    }
  }
  //Serial.printf("the_scroll_loop high water %d\n", uxTaskGetStackHighWaterMark(the_scroll_loop_task));
  for (int i = 0; i < (strlen(scroll_msg) + 2) * 6 + 1; i++) {
    for (int j = 0 ; j < 16; j++) {
      for (int row = 0; row < 7; row++) {
        int sample = pixelMapdis(j, row + 5);
        //Serial.printf("i %3d j %3d row %3d sample %3d data %3d\n",i,j,row,sample,(int)buff[i][row]);
        if (sample != -1) {
          if ( buff( i + j, row ) ) {   //bb if (buff[i + j][row]) {
            if ( !buff ( i + j - 1, row ) ) {  //bb if (!buff[i + j - 1][row]) {
              gfxsetLED(sample, 13, 13, 13);
            }
          } else {
            if ( buff ( i + j - 1, row ) ) {  //bb if (buff[i + j - 1][row]) {
              gfxsetLED(sample, 0, 0, 0);
            }
          }
        }
      }

      FastLED.show();
      //delay(0); // from 5
    }

  }
  //Serial.printf("the_scroll_loop high water %d\n", uxTaskGetStackHighWaterMark(the_scroll_loop_task));
  delay(100);
  scroll_active = false;
  vTaskDelete(NULL);
  }
*/

void the_scroll_loop( void * parameter )
{
  uint32_t ulNotifiedValue;
  Serial.print("scroll, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  new_scroll = false;

  dispclear();

  //Serial.printf("strelen %d\n", strlen(scroll_msg));
  for (int leng = 0; leng < strlen(scroll_msg) + 8 ; leng++) { //5to4 wrong
    int letter;
    if (leng < 2 || leng > (strlen(scroll_msg) + 1) ) {
      letter = 32;
    } else {
      letter = (int)scroll_msg[leng - 2];
    }

    //Serial.printf("%d >%c<\n", letter, scroll_msg[leng-2]);

    for (int row = 0; row < 6; row++) { //7to6
      int onerow = full4x6[letter * 6 + row];
      for (int col = 0; col < 4; col++) {
        if (bitRead(onerow, 7 - col ) > 0) {
          buffstore(true, leng * 4 + col, row);
        } else {
          buffstore(false, leng * 4 + col, row);
        }
      }
    }
  }

  for (int i = 0; i < (strlen(scroll_msg) + 2) * 4 + 1; i++) {
    for (int j = 0 ; j < 16; j++) {
      for (int row = 0; row < 6; row++) {
        int sample = pixelMapdis(j, row + 11);

        if (sample != -1) {
          if ( buff( i + j, row ) ) {   //bb if (buff[i + j][row]) {
            if ( !buff ( i + j - 1, row ) ) {  //bb if (!buff[i + j - 1][row]) {
              gfxsetLED(sample, 13, 13, 13);
            }
          }  else {
            if ( buff ( i + j - 1, row ) ) {  //bb if (buff[i + j - 1][row]) {
              gfxsetLED(sample, 0, 0, 0);
            }
          }
        }
      }

      FastLED.show();
      //delay(0); // from 5
    }
  }

  delay(100);
  scroll_active = false;
  vTaskDelete(NULL);
}



void scroll_and_wait(char* msg) {
  scroll(msg);
  while (scroll_active) {
    delay(50);
  }
}

void scroll (char* msg) {
  if (scroll_active) {
    Serial.printf("Scroll was active\n");
  } else {
    if (strlen(msg) > 99) {
      Serial.printf("Scroll too long\n");
    } else {
      strcpy(scroll_msg, msg);
      //Serial.printf("%d>%s<\n", strlen(scroll_msg), scroll_msg);
      scroll_active = true;
      new_scroll = true;
      xTaskCreatePinnedToCore(
        the_scroll_loop,
        "the_scroll_loop",
        1000, // 4096,       // heap
        NULL,
        2,          // prio higher than 1
        &the_scroll_loop_task,
        1);         // on cpu 0 // switch to 1 so both cores hashare the http traffic // a1

      delay(20);

      if ( the_scroll_loop_task == NULL ) {
        Serial.printf("the_scroll_loop_task failed to start! %d\n", the_scroll_loop_task);
      } else {
        Serial.println("the_scroll_loop_task started");
      }
    }
  }
}
