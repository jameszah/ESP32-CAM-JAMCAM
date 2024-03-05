
/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/

#define vers 68

/*

  https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0

  The is Arduino code, with standard setup for ESP32-CAM
    - Board ESP32 Wrover Module
    - Partition Scheme Huge APP (3MB No OTA)
    - or with AI Thinker ESP32-CAM

   Board AI Thinker ESP32-CAM
   240 MHz
   80 MHz
   QIO
   Minimal SPIFFS(1.9MB APP with OTA/190KB SPIFFS)   <- Important

   Important Libraries

   #include <JPEGDEC.h>
   https://github.com/bitbank2/JPEGDEC
   Using library JPEGDEC at version 1.4.2 in folder: C:\ArduinoPortable\sketch\libraries\JPEGDEC

   #include <FastLED.h>
   https://github.com/FastLED/FastLED
   Using library FastLED at version 3.6.0 in folder: C:\ArduinoPortable\sketch\libraries\FastLED 

   //ver56 #include <ESP32Lib.h>
   https://github.com/bitluni/ESP32Lib
   Using library bitluni_ESP32Lib at version 0.3.4 in folder: C:\ArduinoPortable\sketch\libraries\bitluni_ESP32Lib
   -- switched for fast led - this seems quicker for small number of leds, but not for entire display ????
   -- plus all the vga stuff
   -- kept the various led wall stuff
   
  Hardware
  Basic ESP32-CAM - 4BM psram, but 2MB will probably do
  GPIO 12 - capacitive touch wire or sensor 
  GPIO 13 - data wire for the led matrix
  GPIO 4 - Blinding Disk Active Light - might blink if bad sd card
  GPIO 33 - red led on back not used
  GPIO 1/2 - serial tx/rx will contain debug info messages
  GPIO 0 - hold to GND and power on to program

  Must have working 2640 camera installed
  Normally used with SD card to store movies
  Normally will have WiFi router to access
  Always has a soft AP to access camera without router
  Will read config.txt from SD for timezone, wifi router name/pas,
    and soft wifi name/pass, and defualt touch sequence
  SD must be fat32, and will write a config.txt if doesnot exist,
    which can be edited by going to soft AP name:JAMCAM pass:12344321
  Will run with SD card and without a wifi router

  ver 34 from 33.4
  ver 35 with snakes
  ver 36 Jan 8, 2024 JamesZahary
  ver 37
  ver 38
  ver 39 Jan 11, 2024
   -config editor, ap and sta same time
  ver 40 Jan 12, 2024
   - reduce memory, simple url
  ver 49 Jan 22, 2024
  ver 51 Jan 25, 2024 new clock - maybe its done
  ver 52 Jan 27, 2024 sync in no internet mode, no sd, no wifi
  ver 53 Jan 28 - change touch sensor, turn off lights during boot
  ver 54
  ver 55 - fix modulo bug, change effect busines
  ver 56f - SWITXH TO FAST LED!
  ver 58f - power control for fastled
  ver 61
  ver 62 - 2 ssid + 1 spft + weather-api
  ver 63 - video editing and re-indexing ....
  ver 65 - done - binaries stored
  ver 66 - increase tx power
  ver 67
  ver 68 - status when editing indexing

  Mar 2, 2024 - time to give up
  
*/

#define jwidth 40
#define jheight 30
#define diswidth 16
#define disheight 16

//ver56 #include <ESP32Lib.h>
#include <FastLED.h>
#define NUM_LEDS 256
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

#include "font.h"

#include "ESPxWebFlMgr.h"
const word filemanagerport = 8080;
ESPxWebFlMgr filemgr(filemanagerport);

//uint32_t ARRAY_LENGTH ;
void *ptrVal;
uint8_t *rgb ;
uint8_t *rgbout ;
bool  *big ;
//27
#include <JPEGDEC.h>
JPEGDEC jpz;

void open_avi();
int read_quartet( File fd);

//ver56 SerialLED gfx;
SemaphoreHandle_t baton;
bool ota_active = false;
bool ota_start = false;

uint8_t* framebuffer;
uint8_t* framebuffer3;
bool framebuffer_good = false;
bool framebuffer3_good = false;

int framebuffer_len;
int framebuffer3_len;
long framebuffer_time = 0;
long framebuffer3_time = 0;

uint8_t* framebuffer2;
int framebuffer2_len = 0;
long framebuffer2_time = 0;

//int imageRes[] = {jwidth, jheight};
int currentImage = 1;
int calibration = 100;
volatile bool frameAvailable = false;
int effect = 0;
int prev_effect = 0;

int power = 0;
bool change_power = false;

bool net_frame_avail = false;
bool bmp_frame_avail = false;

extern TaskHandle_t the_camera_loop_task;
TaskHandle_t  the_file_loop_task, the_scroll_loop_task;

extern void the_camera_loop (void* pvParameter);
extern int start_record;
bool new_scroll = false;
bool scroll_active = false;
esp_err_t init_sdcard();  // from avi.cpp
bool no_sd_card = true;
bool no_wifi = true;

char scroll_msg[110];

bool firstloop = true;
int last_frame = 0;
int framenumber = 0;
int total_delay = 0;
int total_gap = 0;
int total_fmt = 0;
bool life_active = false;
bool clive_active = false;
bool snake_active = false;

int next_life = 0;
int next_display = 0;
int next_frame = 0;
int next_frame_prev = 0;
int next_snake = 0;
int next_stats = 0;

int stats_rec = 0;
int stats_disp = 0;
int stats_stream = 0;
int stats_stream_bmp = 0;
int stats_life = 0;
int stats_clive = 0;
int recent_frame_write = 0;

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>

WiFiMulti jMulti;

//#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"
#include <ArduinoOTA.h>


char ssid[30];
char ssid2[30];
char password[30];
char password2[30];
char softssid[30];
char softpassword[30];
char weather_report[60];
bool new_weather = false;

time_t now;
struct tm timeinfo;
char localip[40];

int pixelMap(int x, int y);
int delete_old_stuff_flag = 0;
int delete_all_files = 0;
char file_to_edit[50] = "/JamCam0481.0007.avi";
int touch_effect[4];




unsigned char Out[jwidth][jheight][3];

void gfxsetLED(int sample, int g, int r, int b) {
  leds[sample] = CRGB (r, g, b);
}

void both ( int x, int y, unsigned char g, unsigned char r, unsigned char b ) {
  both ( x, y, g, r,  b , 10) ;
}

void both ( int x, int y, unsigned char g, unsigned char r, unsigned char b, unsigned char g2, unsigned char r2, unsigned char b2 ) {
  int row = (y % disheight);
  int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  int sample = row * diswidth + col;

  leds[sample] = CRGB (r, g, b);

  if ( b == 0 && g == 0 && r == 0 ) {
    Out[x][y][0] = 255;
    Out[x][y][1] = 255;
    Out[x][y][2] = 255;
  } else {
    Out[x][y][0] = b2;
    Out[x][y][1] = g2;
    Out[x][y][2] = r2;
  }
}
void both ( int x, int y, unsigned char g, unsigned char r, unsigned char b , int factor) {

  int row = (y % disheight);
  int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  int sample = row * diswidth + col;

  leds[sample] = CRGB (r, g, b);

  if ( b == 0 && g == 0 && r == 0 ) {
    Out[x][y][0] = 255;
    Out[x][y][1] = 255;
    Out[x][y][2] = 255;
  } else {
    Out[x][y][0] = b * factor;
    Out[x][y][1] = g * factor;
    Out[x][y][2] = r * factor;
  }
}


/*
  int row = (y % disheight);
  int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  int idx = row * diswidth + col;

  //int pixelMapdis(int x, int y)
  //{
  //  if (x < 0 || x >= diswidth  || y < 0 || y >= disheight)
  //    return -1;
  //int row = (y % disheight);
  //int col = ((row & 1) == 0) ? diswidth - 1 - (x % diswidth) : x % diswidth;
  //Serial.printf("x %d, y %d,  row %d, col %d, map %d\n",x,y,row,col,row*16+col);
  //return  row * diswidth + col;
  /
*/

#include "tools.h"
#include "effects.h"
#include "life.h"
#include "clive.h"
#include "snake.h"
#include "misc.h"

#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
static const char* _STREAM_PART_BMP = "Content-Type: image/x-windows-bmp\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t stream_httpd_bmp = NULL;
httpd_handle_t index_httpd = NULL;

#include <EEPROM.h>

int MagicNumber = 32;                // change this number to reset the eprom in your esp32 for file numbers

int file_number = 0;
int file_group = 0;

long boot_time = 0;

struct eprom_data {
  int eprom_good;
  int file_group;
};

void do_eprom_read() {

  eprom_data ed;

  EEPROM.begin(200);
  EEPROM.get(0, ed);

  if (ed.eprom_good == MagicNumber) {
    Serial.println("Good settings in the EPROM ");
    file_group = ed.file_group;
    file_group++;
    Serial.print("New File Group "); Serial.println(file_group );
  } else {
    Serial.println("No settings in EPROM - Starting with File Group 1 ");
    file_group = 1;
  }
  do_eprom_write();
  file_number = 1;
}

void do_eprom_write() {

  eprom_data ed;
  ed.eprom_good = MagicNumber;
  ed.file_group  = file_group;

  Serial.println("Writing to EPROM ...");

  EEPROM.begin(200);
  EEPROM.put(0, ed);
  EEPROM.commit();
  EEPROM.end();
}

#include "web.h" // contains the web page for index

static esp_err_t index_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("index_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  Serial.printf("Page len %d\n", strlen(page_html));
  httpd_resp_send(req, page_html, strlen(page_html));
  return res;;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  delete_old_stuff() - delete oldest files to free diskspace
//
#include <SD_MMC.h>
void listDir( const char * dirname, uint8_t levels) {

  Serial.printf("Listing directory: %s\n", "/");

  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File filex = root.openNextFile();
  while (filex) {
    if (filex.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(filex.name());
      if (levels) {
        listDir( filex.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(filex.name());
      Serial.print("  SIZE: ");
      Serial.println(filex.size());
    }
    filex = root.openNextFile();
  }
}

void delete_old_stuff() {
  delete_old_stuff_target ( 0.8 );
}

void delete_old_stuff_target(float target) {

  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  //listDir( "/", 0);

  float full = 1.0 * SD_MMC.usedBytes() / SD_MMC.totalBytes();

  Serial.printf("Full is %4.2f\n", full);

  if (full  <  0.9) {
    Serial.printf("Nothing deleted, %.1f%% disk full\n", 100.0 * full);
  } else {
    Serial.printf("Disk is %.1f%% full ... deleting oldest file\n", 100.0 * full);
    while (full > target) {

      double del_number = 999999999;
      char del_numbername[50];

      File f = SD_MMC.open("/");

      File file = f.openNextFile();

      while (file) {
        //Serial.println(file.name());
        if (!file.isDirectory()) {

          char foldname[50];
          strcpy(foldname, file.name());
          for ( int x = 0; x < 50; x++) {
            if ( (foldname[x] >= 0x30 && foldname[x] <= 0x39) || foldname[x] == 0x2E) {
            } else {
              if (foldname[x] != 0) foldname[x] = 0x20;
            }
          }

          double i = atof(foldname);
          if ( i > 0 && i < del_number) {
            strcpy (del_numbername, file.name());
            del_number = i;
          }
          //Serial.printf("Name is %s, number is %f\n", foldname, i);
        }
        file = f.openNextFile();

      }
      Serial.printf("lowest is Name is %s, number is %f\n", del_numbername, del_number);
      if (del_number < 999999999) {
        deleteFolderOrFile(del_numbername);
      }
      full = 1.0 * SD_MMC.usedBytes() / SD_MMC.totalBytes();
      Serial.printf("Disk is %.1f%% full ... \n", 100.0 * full);
      f.close();
    }
  }
}

void delete_all () {
  Serial.printf("Deleting All avi\n");
  File f = SD_MMC.open("/" );
  if (!f) {
    Serial.printf("Failed to open folder\n" );
    return;
  }
  File file = f.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      //Serial.print("  DIR : ");
      //Serial.println(file.name());
    } else {
      //Serial.print("  FILE: ");
      //Serial.print(file.name());
      //Serial.print("  SIZE: ");
      //Serial.print(file.size());
      char fname[50];
      strcpy(fname, file.name());

      //int len = strlen(fname);
      char* ret = strrchr(fname, '.');
      //char* ret2 = strchr(&ret[1],'.');
      int is_avi = strcmp (ret, ".avi");
      //Serial.printf("fname >%s< ret >%s<  is_avi %d\n",fname,ret,is_avi);
      if (is_avi == 0) {
        Serial.printf("Delete %s - ", fname);
        if (SD_MMC.remove("/" + String(fname))) {
          Serial.println(" deleted.");
        }   else {
          Serial.println(" FAILED.");
        }

      } else {
        Serial.printf("Skip %s\n", fname);
      }
    }
    file = f.openNextFile();
  }
  f.close();

}

void deleteFolderOrFile(const char * val) {
  // Function provided by user @gemi254
  Serial.printf("Deleting : %s\n", val);
  File f = SD_MMC.open("/" + String(val));
  if (!f) {
    Serial.printf("Failed to open %s\n", val);
    return;
  }

  if (f.isDirectory()) {
    File file = f.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.print("  DIR : ");
        Serial.println(file.name());
      } else {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.print(file.size());
        if (SD_MMC.remove(file.name())) {
          Serial.println(" deleted.");
        } else {
          Serial.println(" FAILED.");
        }
      }
      file = f.openNextFile();
    }
    f.close();
    //Remove the dir
    if (SD_MMC.rmdir("/" + String(val))) {
      Serial.printf("Dir %s removed\n", val);
    } else {
      Serial.println("Remove dir failed");
    }

  } else {
    //Remove the file
    if (SD_MMC.remove("/" + String(val))) {
      Serial.printf("File %s deleted\n", val);
    } else {
      Serial.println("Delete failed");
    }
  }
}
////

String TIMEZONE = "GMT0BST,M3.5.0/01,M10.5.0/02";
bool configfile = false;
String weather_api = "";
String postal = "";

void read_config_file() {

  // put a file "config.txt" onto SD card, to set parameters different from your hardcoded parameters
  // it should look like this - one paramter per line, in the correct order, followed by 2 spaces, and any comments you choose

  String junk;

  String czone = "GMT";
  String cssid = "";
  String cpass = "";
  String cssid2 = "";
  String cpass2 = "";

  String cap_ssid = "JAMCAM";
  String cap_pass = "12344321";
  String eff1, eff2, eff3, eff4;

  File config_file = SD_MMC.open("/config.txt", "r");
  if (config_file) {

    Serial.println("Reading config.txt");
    czone = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    cap_ssid = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    cap_pass = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    cssid = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    cpass = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    cssid2 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    cpass2 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    eff1 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    eff2 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    eff3 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    eff4 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    weather_api = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    postal = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');

    config_file.close();


    Serial.printf("=========   Data from config.txt and defaults  =========\n");
    Serial.printf("Zone len %d, %s\n", czone.length(), czone.c_str());
    Serial.printf("cap_ssid %s\n", cap_ssid);
    Serial.printf("cap_pass %s\n", cap_pass);

    Serial.printf("ssid %s\n", cssid);
    Serial.printf("pass %s\n", cpass);
    Serial.printf("ssid2 %s\n", cssid2);
    Serial.printf("pass2 %s\n", cpass2);

    Serial.printf("eff1 %s\n", eff1);
    Serial.printf("eff2 %s\n", eff2);
    Serial.printf("eff3 %s\n", eff3);
    Serial.printf("eff4 %s\n", eff3);

    Serial.println(weather_api);
    Serial.printf("postal %s\n", postal);


    if (cssid.length() > 1) cssid.toCharArray(ssid, cssid.length() + 1);
    if (cpass.length() > 1  ) cpass.toCharArray(password, cpass.length() + 1);
    if (cssid2.length() > 1) cssid2.toCharArray(ssid2, cssid2.length() + 1);
    if (cpass2.length() > 1  ) cpass2.toCharArray(password2, cpass2.length() + 1);

    if (cap_ssid.length() > 1 && cap_ssid != "none" ) {
      cap_ssid.toCharArray(softssid, cap_ssid.length() + 1);
    } else {
      String x = "JAMCAM";
      x.toCharArray(softssid, x.length() + 1);
    }
    if (cap_pass.length() > 1  ) {
      cap_pass.toCharArray(softpassword, cap_pass.length() + 1);
    } else {
      String x = "12344321";
      x.toCharArray(softpassword, x.length() + 1);
    }

    int neff1 = atoi(eff1.c_str());
    int neff2 = atoi(eff2.c_str());
    int neff3 = atoi(eff3.c_str());
    int neff4 = atoi(eff4.c_str());
    if (neff1 < 0 || neff1 > 14) neff1 = 0;
    if (neff2 < 0 || neff2 > 14) neff1 = 9;
    if (neff3 < 0 || neff3 > 14) neff1 = 4;
    if (neff4 < 0 || neff4 > 14) neff1 = 5;

    Serial.printf("ssid >%s<, pass >%s<, cap_ssid >%s<, cap_pass >%s< eff1 %d eff2 %d eff3 %d eff4 %d\n", ssid, password, softssid, softpassword, neff1, neff2, neff3, neff4);

    touch_effect[0] = neff1;
    touch_effect[1] = neff2;
    touch_effect[2] = neff3;
    touch_effect[3] = neff4;

    configfile = true;
    TIMEZONE = czone;
  } else {

    Serial.printf("No config file\n");

    if (no_sd_card) {
    } else {
      //scroll ("No Config file -- creating one -- JAMCAM/12344321");
      File new_config = SD_MMC.open("/config.txt", "w");
      new_config.print(config_txt);
      new_config.close();

      delay(100);
      ESP.restart();
    }
  }
}

struct oneframe {
  uint8_t* the_frame;
  int the_frame_length;
  int the_frame_number;
  int the_frame_total;
};

//////////////////////////////
oneframe find_a_frame (char * avi_file_name, int frame_pct) ; // from avi.cpp file



static esp_err_t find_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char  buf[120];
  size_t buf_len;
  char  new_res[20];

  oneframe x;

  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  int frame_pct;
  char filename[50];

  Serial.print("find_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    //Serial.printf("Query => %s\n", buf);
    char param[32];
    if (httpd_query_key_value(buf, "f", filename, sizeof(filename)) == ESP_OK) {
      //Serial.printf( "Found URL query parameter => f=>%s<\n", filename);

    }
    if (httpd_query_key_value(buf, "n", param, sizeof(param)) == ESP_OK) {
      int nn = atoi(param);
      if (nn >= 0 && nn <= 30000 ) {
        frame_pct = nn;
        //Serial.printf( "Found URL query parameter => n=%d\n", frame_pct);
      }
    }
  }

  //uint8_t* the_frame = find_a_frame ( "/JamCam0090.0001.avi", 12);

  x = find_a_frame ( filename, frame_pct);
  //the_frame = x.the_frame;

  _jpg_buf_len = x.the_frame_length;
  _jpg_buf = x.the_frame;


  if (x.the_frame == NULL) {
    Serial.printf("no frame\n");

    httpd_resp_send(req, page_html, strlen(page_html));
  } else {

    res = httpd_resp_set_type(req, "image/jpeg");
    if (res != ESP_OK) {
      return res;
    }

    if (res == ESP_OK) {
      char fname[50];
      char frame_num_char[8];
      char frame_pct_char[8];
      char frame_total_char[8];

      sprintf(fname, "inline; filename=frame_%d.jpg", frame_pct);
      sprintf(frame_num_char, "%d", x.the_frame_number);
      sprintf(frame_total_char, "%d", x.the_frame_total);

      sprintf(frame_pct_char, "%d", frame_pct);

      httpd_resp_set_hdr(req, "Content-Disposition", fname);
      httpd_resp_set_hdr(req, "FrameNum", frame_num_char);
      httpd_resp_set_hdr(req, "Total", frame_total_char);
      httpd_resp_set_hdr(req, "FramePct", frame_pct_char);
      httpd_resp_set_hdr(req, "File", filename);


    }
    if (res == ESP_OK) {
      res = httpd_resp_send(req, (const char *)_jpg_buf, _jpg_buf_len);
    }

    free (x.the_frame);
  }
  return res;;
}


//////////////////////////////
static esp_err_t effect_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char  buf[120];
  size_t buf_len;
  char  new_res[20];

  Serial.print("effect_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  effect = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    Serial.printf("Query => %s\n", buf);
    char param[32];
    if (httpd_query_key_value(buf, "e", param, sizeof(param)) == ESP_OK) {

      int x = atoi(param);
      if (x >= 0 && x <= 20 ) {
        effect = x;
      }
      //Serial.printf( "Found URL query parameter => e=%d", effect);
    }
  }

  httpd_resp_send(req, page_html, strlen(page_html));
  return res;;
}

static esp_err_t power_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char  buf[120];
  size_t buf_len;
  char  new_res[20];

  Serial.print("power_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  power  = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    Serial.printf("Query => %s\n", buf);
    char param[32];
    if (httpd_query_key_value(buf, "p", param, sizeof(param)) == ESP_OK) {

      int x = atoi(param);
      if (x >= 100 && x <= 2000 ) {
        power = x;
        change_power = true;
      }
      //Serial.printf( "Found URL query parameter => e=%d", effect);
    }
  }

  httpd_resp_send(req, page_html, strlen(page_html));
  return res;;
}

static esp_err_t time_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char  buf[120];
  size_t buf_len;
  char  new_res[20];

  Serial.print("time_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));


  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    Serial.printf("Time => %s\n", buf);
    char param[32];
    if (httpd_query_key_value(buf, "t", param, sizeof(param)) == ESP_OK) {

      int x = atoi(param);
      //if (x >= 0 && x <= 20 ) {
      //  effect = x;
      //}
      Serial.printf( "Found URL query parameter => t=%d\n", x);

      char tzchar[60];
      TIMEZONE.toCharArray(tzchar, TIMEZONE.length() + 1);        // name of your camera for mDNS, Router, and filenames
      //Serial.println(TIMEZONE);
      Serial.printf("Char >%s<\n", tzchar);
      setenv("TZ", tzchar, 1);  // mountain time zone from #define at top
      tzset();
      struct timeval tv;
      tv.tv_sec = x;
      tv.tv_usec = 0;
      settimeofday(&tv, NULL);
    }
  }

  httpd_resp_send(req, page_html, strlen(page_html));
  return res;;
}

static esp_err_t reboot_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  start_record = 0;

  Serial.print("reboot_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));


  httpd_resp_send(req, page_html, strlen(page_html));
  delay(100);
  ESP.restart();
  return res;;
}



static esp_err_t edit_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char  buf[120];
  size_t buf_len;
  char  new_res[20];

  Serial.print("edit_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));


  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    Serial.printf("Query => %s\n", buf);
    char param[32];

    if (httpd_query_key_value(buf, "f", file_to_edit, sizeof(file_to_edit)) == ESP_OK) {
      Serial.printf( "Found URL query parameter => f=>%s<\n", file_to_edit);

    }

  }


  httpd_resp_send(req, edit_html, strlen(edit_html));

  return res;;
}


static esp_err_t delete_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;

  Serial.print("delete_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));


  httpd_resp_send(req, page_html, strlen(page_html));
  delay(100);
  delete_all_files = 1;
  return res;;
}
static esp_err_t ota_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  start_record = 0;

  Serial.print("ota_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));


  httpd_resp_send(req, page_html, strlen(page_html));
  delay(100);
  ota_start = true;
  return res;;
}


//extern int avi_length ;
extern char avi_file_name[100];
extern long avi_start_time;
extern int avi_length;
extern int frame_interval ;
extern int speed_up_factor ;


static esp_err_t record_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("record_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  speed_up_factor = 1;
  avi_length = 600;
  frame_interval = 35;

  if (!no_sd_card) {
    start_record = (start_record + 1) % 2 ;
  } else {
    Serial.printf("Cannot record without SD Card!\n");
  }

  httpd_resp_send(req, page_html, strlen(page_html));
  return res;
}

static esp_err_t rec_tl_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("rec_tl_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  speed_up_factor = 30;
  avi_length = 1800;
  frame_interval = 1000;
  if (!no_sd_card) {
    start_record = (start_record + 1) % 2 ;
  } else {
    Serial.printf("Cannot record without SD Card!\n");
  }

  httpd_resp_send(req, page_html, strlen(page_html));
  return res;
}

static esp_err_t status_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("status_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  delay(101);

  int remain = (-millis() + (avi_start_time + avi_length * 1000) ) / 1000;
  //Serial.printf("remain %d\n", remain);

  String x = " {\"OnOff\":\"";

  if (start_record == 1) {
    if (frame_interval == 1000) {
      x = x + "TL";
    } else {
      x = x + "On";
    }
  } else {
    x = x + "Off";
  }
  x = x + "\",\"File\":\"";

  int fnl = strlen(avi_file_name);
  //Serial.printf("fnl %d \n", fnl);

  String fn(avi_file_name);
  //Serial.println(fn);

  x = x + fn + "\", \"Remain\": ";
  x = x + String(remain) ;

  int total =  SD_MMC.totalBytes() / (1024 * 1024);
  int used =  SD_MMC.usedBytes() / (1024 * 1024) ;
  int freesp = total - used;

  x = x + ",\"Size\":" + String(total);
  x = x + ",\"Free\":" + String(freesp);
  if (no_wifi) {
    x = x + ",\"rssi\":" + String(0);
  } else {
    x = x + ",\"rssi\":" + String(WiFi.RSSI());
  }

  x = x + ",\"IP\":" + "\"" + String(localip) + "\"" ;

  x = x + ",\"file_to_edit\":" + "\"" + String(file_to_edit) + "\"" ;

  x = x + ",\"Power\":" + String(power) + "}";

  const char* str = x.c_str();

  httpd_resp_send(req, str,  strlen(str));
  return res;
}

static esp_err_t which_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("which_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  String x = " {\"effect\":";

  x = x + String(effect) ;

  x = x + "}";

  const char* str = x.c_str();
  Serial.println(str);
  httpd_resp_send(req, str,  strlen(str));
  return res;
}

void reparse( char * avi_file_name, char * out_file_name, int skip_frames, int frame_start, int frame_end);
void re_index( char * xavi_file_name, char * xout_file_name) ;

char file_to_read[50];
char file_to_write[50];
int newstart;
int newend;
int newskip;
bool do_the_reparse = false;
bool do_the_reindex = false;
bool done_the_reparse = false;
bool done_the_reindex = false;

static esp_err_t reindex_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("reindex_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  char  buf[150];
  size_t buf_len;

  buf_len = httpd_req_get_url_query_len(req) + 1;

  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    //Serial.printf("Query => %s\n", buf);
    if (httpd_query_key_value(buf, "o", file_to_read, sizeof(file_to_read)) == ESP_OK) {
      Serial.printf( "Found URL query parameter => file_to_read=>%s<\n", file_to_read);
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      //Serial.printf("Query => %s\n", buf);
      if (httpd_query_key_value(buf, "n", file_to_write, sizeof(file_to_write)) == ESP_OK) {
        Serial.printf( "Found URL query parameter => file_to_write=>%s<\n", file_to_write);
      }
    }


  }

  do_the_reindex = true;

  
  while (!done_the_reindex) {
    delay(1000);
  }     
    String x = " {\"status\":\"!!!DONE!!!\" }";
    const char* str = x.c_str();
    httpd_resp_send(req, str,  strlen(str));
  
  return res;
}

httpd_req_t *reparse_req = NULL;

static esp_err_t reparse_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("reparse_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));
  
  reparse_req = req;
  
  char  buf[150];
  size_t buf_len;

  buf_len = httpd_req_get_url_query_len(req) + 1;

  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
    //Serial.printf("Query => %s\n", buf);
    if (httpd_query_key_value(buf, "o", file_to_read, sizeof(file_to_read)) == ESP_OK) {
      Serial.printf( "Found URL query parameter => file_to_read=>%s<\n", file_to_read);
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      //Serial.printf("Query => %s\n", buf);
      if (httpd_query_key_value(buf, "n", file_to_write, sizeof(file_to_write)) == ESP_OK) {
        Serial.printf( "Found URL query parameter => file_to_write=>%s<\n", file_to_write);
      }
    }
    char param[15];
    if (httpd_query_key_value(buf, "s", param, sizeof(param)) == ESP_OK) {
      int nn = atoi(param);
      if (nn >= 0 && nn <= 30000 ) {
        newstart = nn;
        Serial.printf( "Found URL query parameter => newstart=%d\n", newstart);
      }
    }
    if (httpd_query_key_value(buf, "e", param, sizeof(param)) == ESP_OK) {
      int nn = atoi(param);
      if (nn >= 0 && nn <= 30000 ) {
        newend = nn;
        Serial.printf( "Found URL query parameter => newend=%d\n", newend);
      }
    }
    if (httpd_query_key_value(buf, "k", param, sizeof(param)) == ESP_OK) {
      int nn = atoi(param);
      if (nn >= 0 && nn <= 30000 ) {
        newskip = nn;
      } else {
        newskip = 0;
      }
      Serial.printf( "Found URL query parameter => newskip=%d\n", newskip);

    }

  }

  //uint8_t* the_frame = find_a_frame ( "/JamCam0090.0001.avi", 12);

  //reparse ( file_to_read, file_to_write, newstart, newend, newskip );
  
  //String x = " {\"status\":\"This could take a minute - hold my beer\" }";
  //const char* str ;
  //str = x.c_str();
  //httpd_resp_send(req, str,  strlen(str));

  do_the_reparse = true;
  while (!done_the_reparse) {
    delay(1000);
  }     
    String x = " {\"status\":\"!!!DONE!!!\" }";
    const char* str = x.c_str();
    httpd_resp_send(req, str,  strlen(str));
  
    
  return res;
}

static esp_err_t framenum_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  Serial.print("framenum_handler, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  String x = " {\"framenum\":";

  x = x + String(effect) ;

  x = x + "}";

  const char* str = x.c_str();
  Serial.println(str);
  httpd_resp_send(req, str,  strlen(str));
  return res;
}

static esp_err_t stream_handler_avi(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];


  Serial.print("stream_handler_avi, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    if (!net_frame_avail) {
      delay(1);
    } else {

      stats_stream++;
      _jpg_buf_len = framebuffer2_len;
      _jpg_buf = framebuffer2;

      if (res == ESP_OK) {
        size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        //Serial.printf("jpg % d\n", hlen);
      }
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        //Serial.printf("jpg % d\n", _jpg_buf_len);
      }
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }

      net_frame_avail = false;

      if (res != ESP_OK) {
        break;
      }
      //Serial.printf("MJPG: % uB\n",(uint32_t)(_jpg_buf_len));
      delay(1);
    }
  }
  return res;
}

//bmp

static esp_err_t stream_handler_bmp(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  uint8_t * buf = NULL;
  size_t buf_len = 0;

  Serial.print("stream_handler_bmp, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  while (true) {

    bool bmpres;
    if (bmp_frame_avail) {
      stats_stream_bmp++;
      bmp_frame_avail = false;
      if (effect == 0) {
        for (int y = 0; y < jheight; y++) {
          for (int x = 0; x < jwidth; x++ ) {
            int ind = (y * 40 + x) * 3;
            rgbout[ ind + 2] = newImage[x][y][2];
            rgbout[ ind + 1] = newImage[x][y][1];
            rgbout[ ind + 0] = newImage[x][y][0];
          }
        }
      }
      else if (effect > 4 &&  effect < 15 )
      {
        for (int y = 0; y < disheight; y++) {
          for (int x = 0; x < diswidth; x++ ) {
            int ind = (y * 16 + x) * 3;
            rgbout[ ind + 2] = Out[x][y][2];
            rgbout[ ind + 1] = Out[x][y][1];
            rgbout[ ind + 0] = Out[x][y][0];
          }
        }
      } else {
        for (int y = 0; y < jheight; y++) {
          for (int x = 0; x < jwidth; x++ ) {
            int ind = (y * 40 + x) * 3;
            rgbout[ ind + 2] = Out[x][y][2];
            rgbout[ ind + 1] = Out[x][y][1];
            rgbout[ ind + 0] = Out[x][y][0];
          }
        }
      }

      if (effect == 0) {
        bmpres = fmt2bmp( &rgbout[0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      } else if (effect == 1) {
        // stream the bigger bmp maybe
        //int ss=millis();
        //Serial.printf("bmpres %d in %d ms\n",bmpres,millis()-ss);

        bmpres = fmt2bmp( &rgbout[0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      } else if (effect == 2) {

        bmpres = fmt2bmp( &rgbout[0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      } else if (effect == 3) {
        bmpres = fmt2bmp( &rgbout[0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      } else if (effect == 4) {
        bmpres = fmt2bmp( &rgbout[0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
        //}  ////else if (effect == 5) {
        //bmpres = fmt2bmp( &FramesOut[0][0][0], jwidth * jheight * 3, jwidth, jheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
        ////bmpres = fmt2bmp( rgb, 80 * 60 * 3, 80, 60, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      } else if (effect > 4 && effect < 15 ) {
        bmpres = fmt2bmp( &rgbout[0], diswidth * disheight * 3, diswidth, disheight, PIXFORMAT_RGB888, &_jpg_buf, &_jpg_buf_len);
      }

      if (!bmpres) {
        Serial.printf("BMP compression failed, % d\n", bmpres);
        res = ESP_FAIL;
      }

      if (res == ESP_OK) {
        size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART_BMP, _jpg_buf_len);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        //Serial.printf("jpg % d\n", hlen);
      }
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        //Serial.printf("jpg % d\n", _jpg_buf_len);
      }
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }
      if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
      } else if (_jpg_buf) {
        free(_jpg_buf);
        _jpg_buf = NULL;
      }

      if (res != ESP_OK) {
        //Serial.printf("stream bmp done\n");
        break;
      }
      //Serial.printf("MJPG: % uB\n",(uint32_t)(_jpg_buf_len));
      delay(1);
    } else {
      delay(1);
    }
  }
  //free(rgbout);
  //Serial.printf("Free\n");
  return res;
}
//bmp
void startCameraServer() {

  httpd_config_t config2 = HTTPD_DEFAULT_CONFIG();
  config2.server_port = 80;
  config2.max_uri_handlers = 16;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t which_uri = {
    .uri       = "/which",
    .method    = HTTP_GET,
    .handler   = which_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t reparse_uri = {
    .uri       = "/reparse",
    .method    = HTTP_GET,
    .handler   = reparse_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t reindex_uri = {
    .uri       = "/reindex",
    .method    = HTTP_GET,
    .handler   = reindex_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t effect_uri = {
    .uri       = "/effect",
    .method    = HTTP_GET,
    .handler   = effect_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t status_uri = {
    .uri       = "/status",
    .method    = HTTP_GET,
    .handler   = status_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t record_uri = {
    .uri       = "/record",
    .method    = HTTP_GET,
    .handler   = record_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t rec_tl_uri = {
    .uri       = "/rec_tl",
    .method    = HTTP_GET,
    .handler   = rec_tl_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t reboot_uri = {
    .uri       = "/reboot",
    .method    = HTTP_GET,
    .handler   = reboot_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t edit_uri = {
    .uri       = "/edit",
    .method    = HTTP_GET,
    .handler   = edit_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t power_uri = {
    .uri       = "/power",
    .method    = HTTP_GET,
    .handler   = power_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t delete_uri = {
    .uri       = "/delete",
    .method    = HTTP_GET,
    .handler   = delete_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t ota_uri = {
    .uri       = "/ota",
    .method    = HTTP_GET,
    .handler   = ota_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t time_uri = {
    .uri       = "/time",
    .method    = HTTP_GET,
    .handler   = time_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t find_uri = {
    .uri       = "/find",
    .method    = HTTP_GET,
    .handler   = find_handler,
    .user_ctx  = NULL
  };
  Serial.printf("Starting web server on port: '%d'\n", config2.server_port);
  if (httpd_start(&index_httpd, &config2) == ESP_OK) {
    httpd_register_uri_handler(index_httpd, &index_uri);
    httpd_register_uri_handler(index_httpd, &which_uri);
    httpd_register_uri_handler(index_httpd, &reparse_uri);
    httpd_register_uri_handler(index_httpd, &reindex_uri);
    httpd_register_uri_handler(index_httpd, &effect_uri);
    httpd_register_uri_handler(index_httpd, &status_uri);
    httpd_register_uri_handler(index_httpd, &record_uri);
    httpd_register_uri_handler(index_httpd, &rec_tl_uri);
    httpd_register_uri_handler(index_httpd, &power_uri);
    httpd_register_uri_handler(index_httpd, &reboot_uri);
    httpd_register_uri_handler(index_httpd, &edit_uri);
    httpd_register_uri_handler(index_httpd, &delete_uri);
    httpd_register_uri_handler(index_httpd, &ota_uri);
    httpd_register_uri_handler(index_httpd, &time_uri);
    httpd_register_uri_handler(index_httpd, &find_uri);
  }

  Serial.printf("after index RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  config.ctrl_port = 32123;
  config.core_id = 0;  //jan20 // jan 25 back to 0
  config.task_priority = 1;

  httpd_uri_t stream_avi_uri = {
    .uri       = "/stream_avi",
    .method    = HTTP_GET,
    .handler   = stream_handler_avi,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_bmp_uri = {
    .uri       = "/stream_bmp",
    .method    = HTTP_GET,
    .handler   = stream_handler_bmp,
    .user_ctx  = NULL
  };
  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_avi_uri);
    httpd_register_uri_handler(stream_httpd, &stream_bmp_uri);
  }
  Serial.printf("after stream RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

}

//#define esp32_touch
#ifdef esp32_touch

int touch_lockout = 0;
int touch_start = 0;
int touch_level = 0;
int multiple = 0;
int touch_index = 0;

void touch12() {

  if (touch_lockout < millis()) {
    if (touch_start == 0) {
      touch_start = millis();
    }

    multiple++;

    touch_level = touchRead(12);
    //Serial.printf("Touch %d, Time %d\n", touch_level, millis());
  } else {
  }
}
#endif

#define ext_touch
#ifdef ext_touch
int touch_index = 0;
int touch_last = 0;
bool touch_next = false;
bool touch_double = false;
int time_up = 0;

void IRAM_ATTR touch12() {
  int state = digitalRead(12);
  if (state == 0) {
    time_up = millis() - touch_last;
    if (time_up > 500) {
      Serial.printf("Long Up %d\n", millis() - touch_last);
      touch_double = true;
    } else {
      Serial.printf("First %d\n", millis() - touch_last);
      touch_next = true;
    }
  } else {
    time_up = 0;
  }

  touch_last = millis();
}
#endif
#include "esp_system.h"
#include "esp_wifi.h"

void setup() {
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.printf("after begin RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  pinMode(33, OUTPUT);             // little red led on back of chip
  digitalWrite(33, HIGH);           // turn on the red LED on the back of chip

  pinMode(4, OUTPUT);               // Blinding Disk-Avtive Light
  digitalWrite(4, LOW);             // turn off

  framebuffer = (uint8_t*)ps_malloc(48 * 1024);
  framebuffer2 = (uint8_t*)ps_malloc(48 * 1024);
  framebuffer3 = (uint8_t*)ps_malloc(48 * 1024);

  ptrVal = heap_caps_malloc(80 * 60 * 3, MALLOC_CAP_SPIRAM);            // allocate memory space for the rgb data
  rgb = (uint8_t *)ptrVal;

  //ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);            // allocate memory space for the rgb data
  rgbout = (uint8_t *)heap_caps_malloc(40 * 30 * 3, MALLOC_CAP_SPIRAM);
  //Serial.printf("Mal\n");

  big = (bool *) ps_malloc(800 * 7 * sizeof(bool));

  Serial.printf("after mall RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  baton = xSemaphoreCreateMutex();

  calculateSinTab();
  calcRainbow();
  //ver56 gfx.setGamma(2.8f, 2.8f, 2.8f);
  Serial.printf("after sintab RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  pinMode(13, OUTPUT);
  //ver56 gfx.init(13, diswidth * disheight, -1, -1);  // GPIO 13 for led matrix data line
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  power = 1000;
  FastLED.setMaxPowerInVoltsAndMilliamps (5, power);

  Serial.printf("after fastled RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  colors();
  Serial.printf("after colors\n");
  //delay(1000);
  //scroll_and_wait("Hi");
  //scroll_and_wait("A DOG WALKS INTO A BAR AND SAYS I CANT SEE ANYTHING - I'LL HAVE ANOTHER");

  Serial.printf("scroll and wait\n");

  initEffects();
  Serial.printf("after init eff RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  do_eprom_read();
  Serial.printf("after eprom RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  //colors();
  //scroll_and_wait("5"); //scroll_and_wait("EPROM");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // PIXFORMAT_RGB565;

  if (psramFound()) {
    config.frame_size =  FRAMESIZE_VGA; //FRAMESIZE_HVGA ; //FRAMESIZE_VGA; //FRAMESIZE_240X240; //FRAMESIZE_VGA;//HVGA; //QVGA; //UXGA; //240X240; //96X96;
    config.jpeg_quality = 10;
    config.fb_count = 3; // jz
  } else {
    Serial.printf("\n\nNO PSRAM !!!!!!!!!!!!!\n\n");
    //scroll ("Dude -- no pasram"); -- screen not started yet
    while (1) {
      delay(1000);
    }
  }

  esp_err_t cam_err = ESP_FAIL;
  int attempt = 5;
  while (attempt && cam_err != ESP_OK) {
    cam_err = esp_camera_init(&config);
    if (cam_err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", cam_err);
      digitalWrite(PWDN_GPIO_NUM, 1);
      delay(800);
      digitalWrite(PWDN_GPIO_NUM, 0); // power cycle the camera (OV2640)
      delay(200);
      attempt--;
    }
  }
  if (cam_err != ESP_OK) {
    Serial.printf("Camera init failed - trying a reboot\n");
    //scroll ("Camera init failed!");
    delay(10000);
    ESP.restart();
  } else {
    Serial.printf("Camera init succeeded!\n");
  }

  sensor_t * ss = esp_camera_sensor_get();
  //ss->set_hmirror(ss, 1);        // 0 = disable , 1 = enable
  ss->set_vflip(ss, 1);          // 0 = disable , 1 = enable
  Serial.printf("after camera RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  //colors();
  //scroll_and_wait("4"); //scroll_and_wait("CAM");

  esp_err_t sd_err = init_sdcard();
  if (sd_err == ESP_FAIL) {
    no_sd_card = true;
  } else {
    no_sd_card = false;
  }

  pinMode(13, OUTPUT);
  //ver56 gfx.init(13, diswidth * disheight, -1, -1);  // GPIO 13 for led matrix data line

  //colors();
  //scroll_and_wait("3"); //scroll_and_wait("SD1");

  if (!no_sd_card) {
    // 5000 stack, prio 5 same at http streamer, core 1
    xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 4000, NULL, 2, &the_camera_loop_task, 0);

    if ( the_camera_loop_task == NULL ) {
      Serial.printf("do_the_camera_loop_task failed to start! %d\n", the_camera_loop_task);
    } else {
      Serial.println("do_the_camera_loop_task started");
    }

    Serial.printf("after camera loop RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );
  }

  delay(50); // was 1000

  if (no_sd_card) {
    String x = "JAMCAM";
    x.toCharArray(softssid, x.length() + 1);
    x = "12344321";
    x.toCharArray(softpassword, x.length() + 1);

    TIMEZONE = "MST7MDT";       // no sd card so you live in mountain time!
    touch_effect[0] = 5;
    touch_effect[1] = 0;
    touch_effect[2] = 9;
    touch_effect[3] = 4;
  } else {
    read_config_file();
  }

  Serial.printf("after read config RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  //colors();
  //scroll_and_wait("2"); //scroll_and_wait("SD2");

  Serial.printf("Starting Soft AP ...\n");
  delay(50);

  Serial.printf("Name for soft ssid and hostname, and OTA: %s\n", softssid);
  WiFi.setHostname(softssid);  // must be before the mode apparantly
  WiFi.mode(WIFI_AP_STA);
  //WiFi.setHostname(softssid);
  WiFi.softAP(softssid, softpassword);
  delay(50);
  Serial.printf("Get Soft AP ...\n");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  sprintf(localip, "Wifi %s %s", softssid, WiFi.softAPIP().toString().c_str());

  Serial.printf("after softap RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );



  if (strlen(ssid) > 3 && (strcmp (ssid, "none") != 0) ) {
    Serial.printf("Starting WiFi ...\n");
    int fails = 0;
    //WiFi.setHostname(softssid);

    jMulti.addAP(ssid, password);
    Serial.printf("added %s / %s\n", ssid, password);
    if (strlen(ssid2) > 3 && (strcmp (ssid2, "none") != 0) ) {
      jMulti.addAP(ssid2, password2);
      Serial.printf("added %s / %s\n", ssid2, password2);
    }
    jMulti.run();
    //WiFi.begin(ssid, password);

    Serial.printf("after begin RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

    while (WiFi.status() != WL_CONNECTED && fails < 20) {
      delay(500);
      Serial.print(".");
      fails++;
    }
    Serial.printf("after connect RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

    if (fails < 20) {
      Serial.println("");
      no_wifi = false;
      Serial.println("WiFi connected");
      Serial.print("Camera Stream Ready! Go to: http://");
      Serial.println(WiFi.localIP());
      /*
            if (!MDNS.begin("JAMCAM")) {       // take several kb of ram
              Serial.println("Error setting up MDNS responder!");
            } else {
              Serial.printf("mDNS responder started \n");
            }
        Serial.printf("after mdns RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );
      */

      configTime(0, 0, "pool.ntp.org");
      char tzchar[60];

      TIMEZONE.toCharArray(tzchar, TIMEZONE.length() + 1);        // name of your camera for mDNS, Router, and filenames
      //Serial.println(TIMEZONE);
      Serial.printf("Char >%s<\n", tzchar);
      setenv("TZ", tzchar, 1);  // mountain time zone from #define at top
      tzset();

      Serial.printf("after time RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

      time(&now);
      while (now < 5) {        // try for 15 seconds to get the time, then give up - 10 seconds after boot
        delay(500);           // this can finish on its own time was 1000
        Serial.print("o");
        time(&now);
      }
      Serial.print("\nLocal time: "); Serial.print(ctime(&now));
      sprintf(localip, "%s", WiFi.localIP().toString().c_str());
    }
  } else {
    no_wifi = true;
    Serial.printf("No router specified");
  }

  Serial.printf("WiFi power was %d -- setting to MAX\n",  WiFi.getTxPower());
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  Serial.printf("WiFi power is %d \n",  WiFi.getTxPower());

  Serial.printf("WiFi sleep was %d -- setting to None\n",  WiFi.getSleep());
  WiFi.setSleep(WIFI_PS_NONE);
  Serial.printf("WiFi sleep is %d \n",  WiFi.getSleep());

  /*
      bool setSleep(bool enabled);
      bool setSleep(wifi_ps_type_t sleepType);
      wifi_ps_type_t getSleep();

    wifi_ps_type_t the_type;

    esp_err_t get_ps = esp_wifi_get_ps(&the_type);
    Serial.printf("The power save was: %d\n", the_type);
    Serial.printf("Set power save to %d\n", WIFI_PS_NONE);
    esp_err_t set_ps = esp_wifi_set_ps(WIFI_PS_NONE);
    esp_err_t new_ps = esp_wifi_get_ps(&the_type);
    Serial.printf("The power save is : %d\n", the_type);
  */
  Serial.printf("after wifi RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  //colors();
  //scroll_and_wait("1"); //scroll_and_wait("WiFi");

  scroll_and_wait(localip);

  // Start streaming web server
  startCameraServer();
  Serial.printf("after camera server RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  ///
  ArduinoOTA.setHostname(softssid);
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  delay(666); // was 666

  Serial.printf("after ota RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

#ifdef esp32_touch
  Serial.printf("GPIO 12 - esp32 controlled touch sensor\n");
  touchAttachInterrupt(12, touch12, 30);

#endif
#ifdef ext_touch
  Serial.printf("GPIO 12 - external touch sensor\n");
  pinMode(12, INPUT);
  attachInterrupt(12, touch12, CHANGE);
  Serial.printf("done\n");
#endif

  //colors();
  //scroll_and_wait("Touch");

  Serial.printf("after touch RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );

  if (!no_sd_card) {
    filemgr.begin();
    Serial.printf("after fileman RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );
    delete_old_stuff_flag == 1;
  }
}

int next_touch = 0;
int recent_fmt = 0;
int recent_framesize = 0;
int recent_downsize = 0;
uint16_t *rgbz;
int next_wifi_check = 45000;

int JPEGDraw888(JPEGDRAW * pDraw)
{
  uint32_t* outptr;
  outptr = (uint32_t*)pDraw->pPixels;
  int ind = 0;

  for (int y = pDraw->y; y < pDraw->y + pDraw->iHeight ; y++) {
    for (int x = pDraw->x; x < pDraw->x + pDraw->iWidth ; x++) {

      int rgbind = (y * pDraw->iWidth + x) * 3;
      rgb[ rgbind + 2] = (outptr[ind] >> 16) & 0xFF; //b;
      rgb[ rgbind + 1] = (outptr[ind] >> 8) & 0xFF;  //g;
      rgb[ rgbind + 0] = (outptr[ind]) & 0xFF;      //r;

      ind++;
    }
  }
  return 1; // continue decode
}

/*
  int JPEGDraw565(JPEGDRAW * pDraw)
  {
  int iCount;
  iCount = pDraw->iWidth * pDraw->iHeight; // number of pixels to draw in this call

  int ind = 0;
  for (int y = pDraw->y; y < pDraw->y + pDraw->iHeight; y++) {
    for (int x = pDraw->x; x < pDraw->x + pDraw->iWidth; x++) {
      //int p = image[(y * 96 + x) * 2 + 1] | (image[(y * 96 + x) * 2] << 8);
      int p = pDraw->pPixels[ind];
      rgb[ (y * 80 + x) * 3 + 0] = (p & 0b11111) << 3;
      rgb[ (y * 80 + x) * 3 + 1] = ((p >> 5) & 0b111111) << 2;
      rgb[ (y * 80 + x) * 3 + 2] = (p >> 11) << 3;

      ind++;
    }
  }

  return 1; // continue decode
  } // JPEGDraw()
*/

class_Clive*  cliveptr;
class_life* lifeptr;
class_snake* snakeptr;

// code from fastled examples
//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino

CRGBPalette16 pacifica_palette_1 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50
};
CRGBPalette16 pacifica_palette_2 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F
};
CRGBPalette16 pacifica_palette_3 =
{ 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
  0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF
};

void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0 - beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10, 38), 0 - beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10, 28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16 & p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if ( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145);
    leds[i].green = scale8( leds[i].green, 200);
    leds[i] |= CRGB( 2, 5, 7);
  }
}



void loop() {

  if (firstloop) {
    firstloop = false;
    Serial.print("loop, core ");  Serial.print(xPortGetCoreID());
    Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));
    effect = touch_effect[0];
    power = 1000;
  }

  if (ota_start) {
    ota_start = false;
    ota_active = true;
    Serial.printf(">> Starting the OTA \n");
    ArduinoOTA.begin();
  }

  if (ota_active) {
    ArduinoOTA.handle();
  }

  if (change_power) {
    change_power = false;
    FastLED.setMaxPowerInVoltsAndMilliamps (5, power);
  }

#ifdef esp32_touch
  if (touch_level > 0) {
    int touch_loop_start = millis();
    int touch_loop_level = touchRead(12);
    int loops = 0;
    while (touch_level < 35  & loops < 600) {
      touch_level = touchRead(12);
      loops++;
      delay(1);
    }
    int touch_end = millis();
    int touch_length = touch_end - touch_start;
    Serial.printf("Touch Start %d, Touch Loop Start %d\n", touch_start, touch_loop_start);
    Serial.printf("Int to start %d, Int to end %d \n", touch_loop_start - touch_start, touch_end - touch_start);
    Serial.printf("Loops %d, Multiple %d\n", loops, multiple);

    if (loops > 400) {
      Serial.printf("Toogle Recording %d\n", loops);
      start_record = (start_record + 1) % 2;
      toggle_record();
    } else {
      if (loops >= 0  || multiple >= 0) {
        Serial.printf("Next Effect %d\n", loops);
        int new_effect = ++touch_index % 4;
        effect = touch_effect[new_effect];// (effect + 1) % 12;
        next_effect();
      }
    }
    touch_lockout = millis() + 1000;
    touch_start = 0;
    touch_level = 0;
    multiple = 0;
    loops = 0;
  }
#endif
#ifdef ext_touch
  if (touch_next) {
    Serial.printf("Next Effect \n");
    int new_effect = ++touch_index % 4;
    effect = touch_effect[new_effect];
    //next_effect();       // scrolling number of the effect - lets drop that
    touch_next = false;
  }
  if (touch_double) {
    Serial.printf("Toogle Recording \n");
    start_record = (start_record + 1) % 2;
    toggle_record();
    touch_double = false;
  }
#endif
  if (next_stats < millis()) {
    float rec_fps = stats_rec / 20.0;
    float disp_fps = stats_disp / 20.0;
    float stream_fps = stats_stream / 20.0;
    float stream_bmp_fps = stats_stream_bmp / 20.0;
    float clive_fps = stats_clive / 20.0;
    float life_fps = stats_life / 20.0;
    float avg_fmt = total_fmt / stats_disp;
    if (1) {
      Serial.printf("Last 20 second stats\n");
      Serial.printf("Recording fps %5.1f\n", rec_fps);
      Serial.printf("Display   fps %5.1f\n", disp_fps);
      Serial.printf("Stream    fps %5.1f\n", stream_fps);
      Serial.printf("Str bmp   fps %5.1f\n", stream_bmp_fps);
      Serial.printf("Clive     fps %5.1f\n", clive_fps);
      Serial.printf("Life      fps %5.1f\n", life_fps);
      Serial.printf("Recent fmt    %d\n", recent_fmt);
      Serial.printf("Recent dowsz  %d\n", recent_downsize);
      Serial.printf("Recent fr sizee  %d\n", recent_framesize);
      Serial.printf("Recent fr write  %d\n", recent_frame_write);
#ifdef esp32_touch
      Serial.printf("Recent Touch     %d\n", touchRead(12));
#endif
      Serial.printf("RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );
      //Serial.printf("Internal Total heap %d, internal Free Heap %d, ", ESP.getHeapSize(), ESP.getFreeHeap());
      //Serial.printf("SPIRam Total heap   %d, SPIRam Free Heap   %d\n", ESP.getPsramSize(), ESP.getFreePsram());
      //Serial.printf("the_camera_loop high water %d\n", uxTaskGetStackHighWaterMark(the_camera_loop_task));
      //Serial.printf("the_file_loop high water %d\n", uxTaskGetStackHighWaterMark(the_file_loop_task));
      //Serial.printf("the_scroll_loop high water %d\n", uxTaskGetStackHighWaterMark(the_scroll_loop_task));
    }
    next_stats = millis() + 20000;
    stats_rec = 0;
    stats_disp = 0;
    stats_stream = 0;
    stats_stream_bmp = 0;
    stats_life = 0;
    stats_clive = 0;
  }

  framenumber++;
  int time_now = millis();

  bool got_fb = false;
  xSemaphoreTake( baton, portMAX_DELAY );
  if (framebuffer_time > (millis() - 200)) { // data from avi loop
    if (framebuffer_good) {
      framebuffer_good = false;
      framebuffer3_len = framebuffer_len;
      framebuffer3_time = framebuffer_time;
      memcpy(framebuffer3, framebuffer,  framebuffer_len);
      recent_framesize = framebuffer_len;
      got_fb = true;
    }
    xSemaphoreGive( baton );

  } else {
    xSemaphoreGive( baton );
    if (next_frame < millis()) {

      camera_fb_t *  fb = esp_camera_fb_get();    // fetch own data - must throttle this when life/clive
      if (fb) {
        framebuffer3_len = fb->len;;
        framebuffer3_time = millis();
        memcpy(framebuffer3, fb->buf,  fb->len);
        recent_framesize = fb->len;
        esp_camera_fb_return(fb);
        got_fb = true;
        if (net_frame_avail) {
          // skip this frame as netword is busy
        } else {
          memcpy(framebuffer2, framebuffer3, framebuffer3_len);
          framebuffer2_len = framebuffer3_len;
          framebuffer2_time = framebuffer3_time;
          net_frame_avail = true;
        }
      }
      next_frame = millis() + 35 ;
      next_frame_prev = millis();
    }
  }

  if (effect != prev_effect) {
    if (life_active ) {
      int pre = ESP.getFreeHeap();
      lifeptr->life_clear();
      delete lifeptr;
      lifeptr = NULL;
      Serial.printf("Saved %d\n", pre - ESP.getFreeHeap());
      life_active = false;
    }
    if (snake_active ) {
      int pre = ESP.getFreeHeap();
      snakeptr->snake_clear();
      delete snakeptr;
      snakeptr = NULL;
      Serial.printf("Saved %d\n", pre - ESP.getFreeHeap());
      snake_active = false;
    }
    if (clive_active ) {
      int pre = ESP.getFreeHeap();
      cliveptr->clive_clear();
      delete cliveptr;
      cliveptr = NULL;
      Serial.printf("Saved %d\n", pre - ESP.getFreeHeap());
      clive_active = false;
    }
  }
  prev_effect = effect;

  // calibration check here
  if (effect > 4 && calibration == 0) {

    if (effect == 11) {
      colors();
    }

    if (effect == 5) {
      if (next_life < millis()) {
        time(&now);
        tm *timeinfo = localtime(&now);

        int secs = timeinfo->tm_sec;

        if (new_weather && secs == 15 ) {
          Serial.printf("here-");
          if (!scroll_active) scroll(weather_report);
          //disptime(1);
        } else {
          disptime(1);
        }

        next_life = millis() + 1000;

        /*  code for scrolling the time - not good
          if (!scroll_active) {
          char nowtime[30];
          time(&now);
          String x = ctime(&now);
          x.substring(11, 17).toCharArray(nowtime, 6); // x.length() + 1);
          //Serial.printf(">%s<\n",nowtime);

          //do_text();
          scroll ( nowtime );
          //effect = 0;
          next_life = millis() + 500;
          }
        */
      }
    }


    if (effect == 13) {
      if (next_life < millis()) {
        tm *timeinfo = localtime(&now);

        int secs = timeinfo->tm_sec;

        disptime(2);

        next_life = millis() + 1000;

      }
    }
    if (effect == 14) {
      if (next_life < millis()) {
        tm *timeinfo = localtime(&now);

        int secs = timeinfo->tm_sec;

        disptime(5);
        //open_avi();
        next_life = millis() + 1000;

      }
    }


    if (effect == 6) {
      if (life_active) {
        int gap = time_now - last_frame;
        if (next_life < millis()) {
          lifeptr->life_update(); //life_update();
          lifeptr->life_display(); //life_display();
          next_life = millis() + 200;                // was 200
          stats_life++;
          bmp_frame_avail = true;
        }
      } else {
        int pre = ESP.getFreeHeap();
        lifeptr = new class_life(200);
        lifeptr->life_clear();
        lifeptr->life_start();
        Serial.printf("Lost %d\n", pre - ESP.getFreeHeap());
        life_active = true;
      }
    }

    if (effect == 8) {
      if (snake_active) {
        int gap = time_now - last_frame;
        if (next_snake < millis()) {
          snakeptr->snake_update(0, 0);
          snakeptr->snake_display();
          next_snake = millis() + 50;                // was 200
          stats_life++;
          bmp_frame_avail = true;
        }
      } else {
        int pre = ESP.getFreeHeap();
        snakeptr = new class_snake(50, 0, 0);
        snakeptr->snake_clear();
        snakeptr->snake_start();
        Serial.printf("Lost %d\n", pre - ESP.getFreeHeap());
        snake_active = true;
      }
    }

    if (effect == 9) {
      if (snake_active) {
        int gap = time_now - last_frame;
        if (next_snake < millis()) {
          snakeptr->snake_update(1, 0);
          snakeptr->snake_display();
          next_snake = millis() + 50;                // was 200
          stats_life++;
          bmp_frame_avail = true;
        }
      } else {
        int pre = ESP.getFreeHeap();
        snakeptr = new class_snake(50, 1, 0);
        snakeptr->snake_clear();
        snakeptr->snake_start();
        Serial.printf("Lost %d\n", pre - ESP.getFreeHeap());
        snake_active = true;
      }
    }

    if (effect == 10) {
      if (snake_active) {
        int gap = time_now - last_frame;
        if (next_snake < millis()) {
          snakeptr->snake_update(0, 1);
          snakeptr->snake_display();
          next_snake = millis() + 50;                // was 200
          stats_life++;
          bmp_frame_avail = true;
        }
      } else {
        int pre = ESP.getFreeHeap();
        snakeptr = new class_snake(50, 0, 1); //immortal and tron
        snakeptr->snake_clear();
        snakeptr->snake_start();
        Serial.printf("Lost %d\n", pre - ESP.getFreeHeap());
        snake_active = true;
      }
    }

    if (effect == 7) {
      snake_active = false;
      life_active = false;
      if (clive_active) {
        cliveptr->clive_update();
        stats_clive++;
        delay(5);   // slow it down
        if (got_fb) {
          bmp_frame_avail = true;
          got_fb = false;
        }
        //bmp_frame_avail = true;
      } else {
        int pre = ESP.getFreeHeap();
        cliveptr = new class_Clive(1);
        cliveptr->clive_clear();
        cliveptr->clive_start();
        Serial.printf("Lost %d\n", pre - ESP.getFreeHeap());
        clive_active = true;
      }
    }

    if (effect == 12) {
      if (next_life < millis()) {
        //FastLED.setMaxPowerInVoltsAndMilliamps (5, 600);
        pacifica_loop();
        int ind = 0;
        for ( int i = 0; i < 16; i++) {
          for ( int j = 0; j < 16; j++ ) {
            Out[j][i][0] = 4 * leds[ind].blue;
            Out[j][i][2] = 4 * leds[ind].red;
            Out[j][i][1] = 4 * leds[ind].green;
            ind++;
          }
        }
        bmp_frame_avail = true;
        FastLED.show();
        next_life = millis() + 100;
        /*
          int rx = random(0, 16);         // the old calm routine - handful of pixels randomly on
          int ry = random(0, 16);
          int cg = random(0, 2);
          int cb = random(0, 2);
          int cr = random(0, 2);
          for (int i = 0; i < diswidth; i++) {
          for (int j = 0; j < disheight; j++) {
            //int sample = pixelMap(i, j);
            int sample = pixelMapdis(i, j);
            if (i == rx && j == ry) {
              if (random(0, 20) == 0) {
                Out[i][j][0] = cg * 100;
                Out[i][j][2] = cr * 100;
                Out[i][j][1] = cb * 100;
                gfxsetLED(sample, cg, cr, cb);
              }
            } else {
              if (random(0, 200) == 0 ) {
                Out[i][j][2] = 255;
                Out[i][j][1] = 255;
                Out[i][j][0] = 255;
                gfxsetLED(sample, 0, 0, 0);
              }
            }
          }
          }
        */


        FastLED.show();

        if (got_fb) {
          bmp_frame_avail = true;
          got_fb = false;
        }
        delay(25);
      }
    }
  }
  else {  // effect > 4

    //FastLED.setMaxPowerInVoltsAndMilliamps (5, 1200);
    if (got_fb) {
      if (next_display < millis()) {
        size_t _jpg_buf_len = 0;
        uint8_t * _jpg_buf = NULL;
        uint8_t * _rgb_buf = NULL;
        int sta = millis();

        if (jpz.openRAM(framebuffer3, framebuffer3_len, JPEGDraw888)) {
          jpz.setPixelType(RGB8888);
          //Serial.printf("\nWidth %d, Height %d, Bpp %d\n",  jpz.getWidth(), jpz.getHeight(), jpz.getBpp());

          jpz.decode(0, 0, JPEG_SCALE_EIGHTH);
          recent_fmt = millis() - sta;
          sta = millis();
          downSampleBig ( (uint8_t *)rgb );
          recent_downsize = millis() - sta;
          jpz.close();
        }

        frameAvailable = true;
        bmp_frame_avail = true;
        next_display = millis() + 50; //jan20
        stats_disp++;
      }

      if (frameAvailable) {
        processImage();
        delay(1);
        frameAvailable = false;
      }

    } // got_fb
  } // else -- not 7 or 8

  ///

  filemgr.handleClient();

  if (do_the_reparse) {
    done_the_reparse = false;
    do_the_reparse = false;
    
    reparse ( file_to_read, file_to_write, newskip , newstart, newend );
    done_the_reparse = true;

  }
  if (do_the_reindex) {
    done_the_reindex = false;
    do_the_reindex = false;
    re_index ( file_to_read, file_to_write );
    done_the_reindex = true;
  }

  if (delete_old_stuff_flag == 1 || delete_all_files == 1 ||   millis() > next_wifi_check  ) {

    xTaskCreatePinnedToCore(
      the_file_loop,
      "the_file_loop",
      4000, // 4096,       // heap
      NULL,
      3,          // prio higher than 1
      &the_file_loop_task,
      1);         // on cpu 0 // switch to 1 so both cores hashare the http traffic // a1

    delay(50);

    if ( the_file_loop_task == NULL ) {
      Serial.printf("the_file_loop_task failed to start! %d\n", the_file_loop_task);
    } else {
      Serial.println("the_file_loop_task started");
    }
    Serial.printf("after file RAM  %7d / %7d, SPI %7d / %7d\n ", ESP.getFreeHeap(), ESP.getHeapSize(),  ESP.getFreePsram(), ESP.getPsramSize() );
  }

} // loop

void the_file_loop( void * parameter )
{
  uint32_t ulNotifiedValue;

  Serial.print("the_file_loop, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  int check_wifi = false;
  if (millis() > next_wifi_check) {
    check_wifi = true;
    next_wifi_check = millis() + 30 * 60 * 1000;
  }
  if (!no_sd_card) {
    if (delete_old_stuff_flag == 1) {
      delete_old_stuff_flag = 0;
      delete_old_stuff();
    }
    if (delete_all_files == 1) {
      delete_all_files = 0;
      delete_all();
    }
  } else {
    delete_old_stuff_flag = 0;
    delete_all_files = 0;
  }

  if (!no_wifi) {
    if (check_wifi) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("***** WiFi reconnect *****");
        WiFi.reconnect();
        delay(1000);
        sprintf(localip, "%s", WiFi.localIP().toString().c_str());
        //MDNS.begin("JAMCAM");
      }

      //  if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WiFi Status %d\n", WiFi.status());
      Serial.print("Gateway IP: ");
      Serial.println(WiFi.gatewayIP().toString());

      if (weather_api.length() > 20 && postal.length() > 3 ) {
        HTTPClient http;
        const char * headerKeys[] = {"Date"} ;
        const size_t numberOfHeaders = 1;

        String weather_url = "http://api.weatherapi.com/v1/current.json?key=" + weather_api + "&q=" + postal + "&aqi=no";

        http.begin(weather_url);
        http.collectHeaders(headerKeys, numberOfHeaders);

        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          if (httpCode == HTTP_CODE_OK || httpCode == 301) {

            if (http.hasHeader("Set-Cookie")) {
              Serial.println("YEAH");
            } else {
              Serial.println("NOPE");
            }

            String headerDate = http.header("Date");
            Serial.println(headerDate);

            String payload = http.getString();
            Serial.println(payload);
            int pos1 = payload.indexOf("temp_c\":");

            int pos2 = payload.indexOf(",", pos1);
            Serial.printf("pos1 %d, pos2 %d\n", pos1, pos2);
            String x = payload.substring(pos1 + 8, pos2);
            Serial.println(x);
            int pos3 = payload.indexOf( "tion\":{\"text\"", pos2);
            int pos4 = payload.indexOf(",", pos3);
            Serial.printf("pos3 %d, pos4 %d\n", pos3, pos4);
            String y = payload.substring(pos3 + 15, pos4 - 1);
            Serial.println(y);
            String weather = "Temp " + x + " " + y;

            weather.toUpperCase();
            weather.toCharArray(weather_report, weather.length() + 1);
            new_weather = true;

          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        Serial.printf("In the http, heap %d\n", ESP.getFreeHeap());
        http.end();
      }

      /*
            HTTPClient http;
            const char * headerKeys[] = {"date", "server"} ;
            const size_t numberOfHeaders = 2;

            // configure traged server and url
            //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
            //http.begin("http://example.com/index.html"); //HTTP
            //http.begin("http://httpbin.org/get"); //HTTP
            http.begin("http://google.ca"); //HTTP
            http.collectHeaders(headerKeys, numberOfHeaders);
            //    http.addHeader("accept", "application/json");

            int httpCode = http.GET();

            // httpCode will be negative on error
            if (httpCode > 0) {
              // HTTP header has been send and Server response header has been handled
              Serial.printf("[HTTP] GET... code: %d\n", httpCode);

              if (httpCode == HTTP_CODE_OK || httpCode == 301) {

                //if (http.hasHeader("Set-Cookie")) {
                //  Serial.println("YEAH");
                //} else {
                //  Serial.println("NOPE");
                //}

                String headerDate = http.header("Date");
                Serial.println(headerDate);
                String headerServer = http.header("Server");
                Serial.println(headerServer);

                //String payload = http.getString();
                //Serial.println(payload);

              }
            } else {
              Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
      */


      check_wifi = false;
    }
  }
  vTaskDelete(NULL);
}

/*






*/
