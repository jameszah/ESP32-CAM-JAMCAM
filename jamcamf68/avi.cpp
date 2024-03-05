/*

 
   https://github.com/jameszah/ESP32-CAM-JAMCAM
    
  heavily modified plus new avi editing functions, from: 

  https://github.com/jameszah/CameraWebServerRecorder

   which is a heavily modifed and simplfied rework of:

  https://github.com/jameszah/ESP32-CAM-Video-Recorder-junior

  which is a heavily modifed and simplfied rework of:

  https://github.com/jameszah/ESP32-CAM-Video-Recorder

  James Zahary
  Aug 14, 2022
  Mar 2, 2024 - file editing and re-indexing, file-a-frame etc
  
  jameszah/ESP32-CAM-JAMCAM is licensed under the
  GNU General Public License v3.0
  jameszah/CameraWebServerRecorder is licensed under the
  GNU General Public License v3.0
*/

#include "esp_camera.h"
#include "sensor.h"

#include <SD_MMC.h>

#define AVIOFFSET 240 // AVI main header length

int framesize = FRAMESIZE_VGA; //FRAMESIZE_240X240; // FRAMESIZE_HD;
int quality = 12;
int avi_length;    // set below
int frame_interval ; // set below
int speed_up_factor = 1;
camera_fb_t * fb_curr = NULL;
long current_frame_time;
long last_frame_time;
int start_record = 0;
extern int file_number ;
extern int file_group ;
int freespace = 0;
extern int delete_old_stuff_flag ;
extern SemaphoreHandle_t baton;

extern uint8_t* framebuffer;
extern bool framebuffer_good ;

extern int framebuffer_len;
extern long framebuffer_time ;
//extern uint8_t framebuffer2[8 * 1024 ];
extern uint8_t* framebuffer2;
extern int framebuffer2_len ;
extern long framebuffer2_time;
extern bool net_frame_avail ;

#define fbs 4 //  how many kb of static ram for psram -> sram buffer for sd write
uint8_t framebuffer_static[fbs * 1024 + 20];
File avifile;
File idxfile;
File outfile;

char avi_file_name[100];

static int i = 0;
uint16_t frame_cnt = 0;
uint16_t remnant = 0;
uint32_t length = 0;
uint32_t startms;
uint32_t elapsedms;
uint32_t uVideoLen = 0;

int bad_jpg = 0;
int extend_jpg = 0;
int normal_jpg = 0;
int which_core = 0;
int which_core_samples = 0;

#define blinking 0

TaskHandle_t the_camera_loop_task;

long avi_start_time = 0;
long avi_end_time = 0;
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
unsigned long idx_offset = 0;

uint8_t zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
uint8_t dc_buf[4] = {0x30, 0x30, 0x64, 0x63};    // "00dc"
uint8_t dc_and_zero_buf[8] = {0x30, 0x30, 0x64, 0x63, 0x00, 0x00, 0x00, 0x00};

uint8_t avi1_buf[4] = {0x41, 0x56, 0x49, 0x31};    // "AVI1"
uint8_t idx1_buf[4] = {0x69, 0x64, 0x78, 0x31};    // "idx1"


struct frameSizeStruct {
  uint8_t frameWidth[2];
  uint8_t frameHeight[2];
};

//  data structure from here https://github.com/s60sc/ESP32-CAM_MJPEG2SD/blob/master/avi.cpp, extended for ov5640

static const frameSizeStruct frameSizeData[] = {
  {{0x60, 0x00}, {0x60, 0x00}}, // FRAMESIZE_96X96,    // 96x96
  {{0xA0, 0x00}, {0x78, 0x00}}, // FRAMESIZE_QQVGA,    // 160x120
  {{0xB0, 0x00}, {0x90, 0x00}}, // FRAMESIZE_QCIF,     // 176x144
  {{0xF0, 0x00}, {0xB0, 0x00}}, // FRAMESIZE_HQVGA,    // 240x176
  {{0xF0, 0x00}, {0xF0, 0x00}}, // FRAMESIZE_240X240,  // 240x240   4
  {{0x40, 0x01}, {0xF0, 0x00}}, // FRAMESIZE_QVGA,     // 320x240   5
  {{0x90, 0x01}, {0x28, 0x01}}, // FRAMESIZE_CIF,      // 400x296   6
  {{0xE0, 0x01}, {0x40, 0x01}}, // FRAMESIZE_HVGA,     // 480x320   7
  {{0x80, 0x02}, {0xE0, 0x01}}, // FRAMESIZE_VGA,      // 640x480   8
  //
  {{0x20, 0x03}, {0x58, 0x02}}, // FRAMESIZE_SVGA,     // 800x600   9
  {{0x00, 0x04}, {0x00, 0x03}}, // FRAMESIZE_XGA,      // 1024x768  10
  {{0x00, 0x05}, {0xD0, 0x02}}, // FRAMESIZE_HD,       // 1280x720  11
  {{0x00, 0x05}, {0x00, 0x04}}, // FRAMESIZE_SXGA,     // 1280x1024 12
  {{0x40, 0x06}, {0xB0, 0x04}}, // FRAMESIZE_UXGA,     // 1600x1200 13
  // 3MP Sensors
  {{0x80, 0x07}, {0x38, 0x04}}, // FRAMESIZE_FHD,      // 1920x1080 14
  {{0xD0, 0x02}, {0x00, 0x05}}, // FRAMESIZE_P_HD,     //  720x1280 15
  {{0x60, 0x03}, {0x00, 0x06}}, // FRAMESIZE_P_3MP,    //  864x1536 16
  {{0x00, 0x08}, {0x00, 0x06}}, // FRAMESIZE_QXGA,     // 2048x1536 17
  // 5MP Sensors
  {{0x00, 0x0A}, {0xA0, 0x05}}, // FRAMESIZE_QHD,      // 2560x1440 18
  {{0x00, 0x0A}, {0x40, 0x06}}, // FRAMESIZE_WQXGA,    // 2560x1600 19
  {{0x38, 0x04}, {0x80, 0x07}}, // FRAMESIZE_P_FHD,    // 1080x1920 20
  {{0x00, 0x0A}, {0x80, 0x07}}  // FRAMESIZE_QSXGA,    // 2560x1920 21

};

const int avi_header[AVIOFFSET] PROGMEM = {
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x4E, 0x46, 0x4F,
  0x10, 0x00, 0x00, 0x00, 0x6A, 0x61, 0x6D, 0x65, 0x73, 0x7A, 0x61, 0x68, 0x61, 0x72, 0x79, 0x20,
  0x76, 0x35, 0x35, 0x20, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};


//
// Reads an uint32_t in Big Endian at current file position
//
int read_quartet( File fd) {

  uint8_t y[4];
  size_t i1_err = fd.read(y , 4);
  uint32_t value = y[0] | y[1] << 8 | y[2] << 16 | y[3] << 24;
  return value;
}

//
// Writes an uint32_t in Big Endian at current file position
//
static void inline print_quartet(unsigned long i, File fd) {

  uint8_t y[4];
  y[0] = i % 0x100;
  y[1] = (i >> 8) % 0x100;
  y[2] = (i >> 16) % 0x100;
  y[3] = (i >> 24) % 0x100;
  size_t i1_err = fd.write(y , 4);
}

//
// Writes an uint32_t in Big Endian at current file position
//
static void inline print_dc_quartet(unsigned long i, File fd) {

  uint8_t y[8];
  y[0] = 0x30;       // "00dc"
  y[1] = 0x30;
  y[2] = 0x64;
  y[3] = 0x63;

  y[4] = i % 0x100;
  y[5] = (i >> 8) % 0x100;
  y[6] = (i >> 16) % 0x100;
  y[7] = (i >> 24) % 0x100;
  size_t i1_err = fd.write(y , 8);
}

//
// Writes 2 uint32_t in Big Endian at current file position
//
static void inline print_2quartet(unsigned long i, unsigned long j, File fd) {

  uint8_t y[8];
  y[0] = i % 0x100;
  y[1] = (i >> 8) % 0x100;
  y[2] = (i >> 16) % 0x100;
  y[3] = (i >> 24) % 0x100;
  y[4] = j % 0x100;
  y[5] = (j >> 8) % 0x100;
  y[6] = (j >> 16) % 0x100;
  y[7] = (j >> 24) % 0x100;
  size_t i1_err = fd.write(y , 8);
}

// https://github.com/espressif/arduino-esp32/blob/master/libraries/SD_MMC/README.md
esp_err_t init_sdcard() {
  esp_err_t ret = ESP_FAIL;
  pinMode(13, INPUT_PULLUP);
  int succ = SD_MMC.begin("/sdcard", true);
  if (succ) {

    pinMode(4, OUTPUT);               // Blinding Disk-Avtive Light
    digitalWrite(4, LOW);             // turn off
    pinMode(13, OUTPUT);
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
    ret = ESP_OK;

  } else {
    Serial.printf("Failed to mount SD card VFAT filesystem. \n");
    Serial.println("Do you have an SD Card installed?");
    Serial.println("Check pin 12 and 13, not grounded, or grounded with 10k resistors!\n\n");
  }
  return ret;
}

camera_fb_t *  get_good_jpeg() {

  camera_fb_t * fb;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera Capture Failed");
    delay(1000);
  }
  return fb;
}



/*
  for (int i = 240 ; i < 256; i=i+4){
  Serial.printf("%d: ",i);
  avifile.seek( i , SeekSet);
  xx2 = read_quartet( avifile);
  if (i == 244) flen = xx2;
  }

  for (int i = 240+flen ; i < 240+flen+16; i=i+4){
  Serial.printf("%d: ",i);
  avifile.seek( i , SeekSet);
  xx2 = read_quartet( avifile);
  if (i == 248) flen = xx2;
  }
*/


int once = 0;
void reparse( char * avi_file_name, char * out_file_name, int skip_frames, int frame_start, int frame_end);
void open_avi() {
  reparse ( "/JamCam0090.0001.avi", "/JamCam0190.0001a.avi", 9, 0, 0 );
}


struct oneframe {
  uint8_t* the_frame;
  int the_frame_length;
  int the_frame_number;
  int the_frame_total;
};


oneframe find_a_frame (char * avi_file_name, int frame_num) {
  oneframe x;
  avifile = SD_MMC.open(avi_file_name, "r"); // avifile = SD_MMC.open(avi_file_name, "w");
  if (!avifile) {
    Serial.printf("Could not open %s file\n", avi_file_name);
    x.the_frame = NULL;
    return x;

  }  else  {
    //Serial.printf("File open: %s\n", avi_file_name);
    avifile.seek( 0x30 , SeekSet);
    int frame_cnt = read_quartet( avifile);
    Serial.printf("Frames %d\n", frame_cnt);

    //int frame_num = 0.01 * frame_pct * frame_cnt;
    //Serial.printf("Frames pct %d, Frame num %d \n", frame_pct, frame_num);

    if (frame_cnt < frame_num) {
      Serial.printf("Only %d frames, less than %d frame_num -- start at 0\n", frame_cnt, frame_num);
      frame_num = 0;
    }

    avifile.seek( 0xe8 , SeekSet);
    int index_start = read_quartet( avifile);
    Serial.printf("Len of movi / index_start %d\n", index_start);

   //bool success = avifile.seek(  , SeekEnd);
    
    Serial.printf("Len of file %d\n",avifile.size());
    Serial.printf("Seek %d\n",  index_start + 244 + frame_num * 16 + 8);
    
    if (avifile.size() < index_start + 244 + frame_num * 16 + 8 , SeekSet) {
       Serial.printf("File too small / broken %s file\n", avi_file_name);
       x.the_frame = NULL;
       return x;      
    }
    bool success = avifile.seek( index_start + 244 + frame_num * 16 + 8 , SeekSet);
    if (!success) {
       Serial.printf("File incomplete %s file\n", avi_file_name);
       x.the_frame = NULL;
       return x;
      
    }
    int index_frame_start = read_quartet( avifile);
    int index_frame_length = read_quartet( avifile);

    avifile.seek( index_frame_start + 236 , SeekSet);
    int the_oodc = read_quartet (avifile);   
    if (the_oodc != 1667510320) {
       Serial.printf("No frame %s file, num %d\n", avi_file_name, frame_num);
       x.the_frame = NULL;
       return x;
    }
    int the_len = read_quartet (avifile);   
    Serial.printf("frame len %d \n", the_len);
    
    //Serial.printf("Your frame is %d bytes, at address %d or %X\n", index_frame_length, index_frame_start, index_frame_start);
    uint8_t* framebuffer;
    framebuffer = (uint8_t*)ps_malloc(48 * 1024);

    avifile.seek( index_frame_start + 244 , SeekSet);

    size_t err = avifile.read(framebuffer, index_frame_length);

    x.the_frame = framebuffer;
    x.the_frame_length = index_frame_length;
    x.the_frame_number = frame_num;
    x.the_frame_total = frame_cnt;
    return x;

  } // else yes to no avi file
}

void reparse( char * avi_file_name, char * out_file_name, int skip_frames) {
  reparse(  avi_file_name,  out_file_name, skip_frames, 0, 0) ;
}

void reparse( char * avi_file_name, char * out_file_name, int skip_frames, int frame_start, int frame_end) {

  //once++;
  //if (once > 1) return;

  extern uint8_t* framebuffer;
  uint16_t remnant = 0;

  //  const char * avi_file_name = "/JamCam0090.0001.avi";
  const char * idx_file_name = "/idx.tmp"; // "/JamCam0190.0001.idx";
  //  const char * out_file_name = "/JamCam0190.0001a.avi";

#define fbs 4 //  how many kb of static ram for psram -> sram buffer for sd write
  uint8_t framebuffer_static[fbs * 1024 + 20];

  avifile = SD_MMC.open(avi_file_name, "r"); // avifile = SD_MMC.open(avi_file_name, "w");
  idxfile = SD_MMC.open(idx_file_name, "w"); //idxfile = SD_MMC.open("/idx.tmp", "w");
  outfile = SD_MMC.open(out_file_name, "w"); //idxfile = SD_MMC.open("/idx.tmp", "w");

  if (avifile) {
    Serial.printf("File open: %s\n", avi_file_name);
  }  else  {
    Serial.printf("Could not open %s file\n", avi_file_name);
  }

  if (idxfile)  {
    Serial.printf("File open: %s\n", idx_file_name);
  }  else  {
    Serial.printf("Could not open file %s\n", idx_file_name);
  }

  if (!avifile) {
    return;
  } else {
    //size_t err = avifile.read( framebuffer_static, 240);

    avifile.seek( 0x24 , SeekSet);
    int max_bytes_per_sec  = read_quartet( avifile);
    Serial.printf("Max bytes per sec %d\n", max_bytes_per_sec);

    avifile.seek( 0x30 , SeekSet);
    int frame_cnt = read_quartet( avifile);
    Serial.printf("Frames %d\n", frame_cnt);


    if (frame_cnt < frame_start) {
      Serial.printf("Only %d frames, less than %d frame_start -- start at 0\n", frame_cnt, frame_end);
      frame_start = 0;
    }
    // if frame_end is 0, or too high, it will go to frame_cnt,
    if (frame_cnt < frame_end || frame_end == 0) {
      Serial.printf("Only %d frames, less than %d frame_end -- end at max frames\n", frame_cnt, frame_end);
      frame_end = frame_cnt;
    }

    int num_out_frames = frame_cnt / (skip_frames + 1);
    Serial.printf("Original %d frames, skipping %d, so %d output frames\n", frame_cnt, skip_frames, num_out_frames );


    //avifile.seek( 0x8c , SeekSet);
    //int frame_cnt8c = read_quartet( avifile);

    avifile.seek( 0x84 , SeekSet);
    int iAttainedFPS = read_quartet( avifile);
    Serial.printf("fps %d\n", iAttainedFPS);

    avifile.seek( 0xe8 , SeekSet);
    int index_start = read_quartet( avifile);
    Serial.printf("Len of movi %d\n", index_start);

    Serial.printf("-----------------\n");

    avifile.seek( 0 , SeekSet);
    size_t err = avifile.read(framebuffer_static, AVIOFFSET);
    Serial.printf("avi read header %d\n", err);
    size_t err2 = outfile.write(framebuffer_static, AVIOFFSET);
    Serial.printf("avi write header %d\n", err2);
    outfile.seek( 240 , SeekSet);


    Serial.printf("-----------------\n");

    int xx2;
    int flen;
    int prev_frame_length = 0;
    int next_frame_start = 240;
    int new_frame_length;
    int prev_frame_start = 0;
    int index_frame_length;
    int index_frame_start;
    int idx_offset = 4;
    int movi_size = 0;
    int frame_cnt_out = 0;


    for (int frame_num = frame_start; frame_num < frame_end; frame_num = frame_num + skip_frames + 1) {
      
      avifile.seek( index_start + 244 + frame_num * 16 + 8 , SeekSet);
      index_frame_start = read_quartet( avifile);
      index_frame_length = read_quartet( avifile);

      // back to looking in file
      /*
        avifile.seek( next_frame_start + 4  , SeekSet);
        new_frame_length = read_quartet( avifile);

        prev_frame_start = next_frame_start;
        next_frame_start = prev_frame_start + new_frame_length + 8;
      */

      //prev_frame_length = new_frame_length;
      
      if (frame_num < 5 || frame_num % 500 == 0) {
        Serial.printf("Frame %4d, index len %9d, frame len %9d, index start %9d, frame start %9d\n", frame_num, index_frame_length, new_frame_length, index_frame_start + 236 , prev_frame_start);
      }

      //if (frame_num % skip_frames == 0 ) {
      
      
      if (frame_num < 5 || frame_cnt_out % 100 == 0) {
        Serial.printf("Frame %4d, index len %9d, frame len %9d, index start %9d, frame start %9d\n", frame_num, index_frame_length, new_frame_length, index_frame_start + 236 , prev_frame_start);
        Serial.printf("in %d, out %d\n", frame_num, frame_cnt_out);
      }
      
      avifile.seek( index_frame_start + 244 , SeekSet);

      remnant = (4 - (index_frame_length & 0x00000003)) & 0x00000003;
      int index_frame_length_rem = index_frame_length + remnant;

      int left_to_write = index_frame_length_rem;

      print_dc_quartet( index_frame_length_rem, outfile);

      while (left_to_write > 0) {
        if (left_to_write > fbs * 1024) {
          size_t err = avifile.read(framebuffer_static, fbs * 1024);
          size_t err2 = outfile.write(framebuffer_static, fbs * 1024);
          //if (frame_num < 5 || frame_num % 100 == 0) {
          //  Serial.printf("frame %4d, write %5d bytes before addr %9d, from %9d\n", frame_num, fbs * 1024, outfile.position(), avifile.position());
          //}
          left_to_write = left_to_write - fbs * 1024;
        } else {
          size_t err = avifile.read(framebuffer_static, left_to_write);
          size_t err2 = outfile.write(framebuffer_static, left_to_write);
          //if (frame_num < 5 || frame_num % 100 == 0) {
          // Serial.printf("frame %4d, write %5d bytes before addr %9d, from %9d\n", frame_num, left_to_write , outfile.position(), avifile.position()); 
          //}
          left_to_write = 0;
        }
      }

      movi_size += index_frame_length;
      movi_size += remnant;

      print_2quartet(idx_offset, index_frame_length, idxfile);

      idx_offset = idx_offset + index_frame_length_rem + 8;
      
      frame_cnt_out++;
      
    }
    //Serial.printf("frame %4d, outfile %9d, avifile %9d, idxfile %9d\n", frame_cnt , outfile.position(), avifile.position(), avifile.position());

    idxfile.close();
    size_t i1_err = outfile.write(idx1_buf, 4);
    if (!i1_err) Serial.printf("idx write\n");

    print_quartet(frame_cnt_out * 16, outfile);

    idxfile = SD_MMC.open(idx_file_name, "r");
    if (idxfile)  {
      Serial.printf("File open: %s\n", idx_file_name);
    }  else  {
      Serial.printf("Could not open file %s\n", idx_file_name);
    }

    char * AteBytes;
    AteBytes = (char*) malloc (8);

    for (int i = 0; i < frame_cnt_out; i++) {
      size_t res = idxfile.readBytes( AteBytes, 8);
      if (!res) Serial.printf("idx read\n");
      size_t i1_err = outfile.write(dc_and_zero_buf, 8);
      if (!i1_err) Serial.printf("dc write\n");
      //size_t i2_err = outfile.write(zero_buf, 4);
      //if (!i2_err) Serial.printf("zero write\n");
      size_t i3_err = outfile.write((uint8_t *)AteBytes, 8);
      if (!i3_err) Serial.printf("ate write\n");
    }

    free(AteBytes);

    outfile.seek( 4 , SeekSet);         //shit
    print_quartet(movi_size + 240 + 16 * frame_cnt_out + 8 * frame_cnt_out, outfile);
    
    avifile.seek( 0xe8 , SeekSet);
    int lom = read_quartet( avifile);
    Serial.printf("Len of movi was %d, now is %d\n", lom, movi_size);
    
    outfile.seek( 0xe8 , SeekSet);
    // shit print_quartet (movi_size, outfile);
    print_quartet(movi_size + frame_cnt_out * 8 + 4, outfile);
     
    avifile.seek( 0x30 , SeekSet);
    int fc = read_quartet( avifile);
    Serial.printf("Frames was %d, now is %d\n", fc, frame_cnt_out);
    outfile.seek( 0x30 , SeekSet);
    print_quartet (frame_cnt_out, outfile);
    outfile.seek( 0x8c , SeekSet);
    print_quartet (frame_cnt_out, outfile);

    avifile.seek( 0x84 , SeekSet);
    int fps = read_quartet( avifile);
    Serial.printf("fps was %d\n", fps);

    float fnewfps = ( 1.0f * fps ) / (1 + skip_frames)   ;
    int newfps = round(fnewfps);
  
    Serial.printf("newfps is %f, %d\n", fnewfps, newfps);
    avifile.seek( 0x84 , SeekSet);
    print_quartet(newfps, avifile);

    avifile.seek( 0x20 , SeekSet);
    int us_per_frame = read_quartet( avifile);
    Serial.printf("us_per_frame was %d\n", us_per_frame);

    float newus = 1000000.0f / fnewfps;
    uint32_t new_us_per_frame = round (newus);
    
  
    Serial.printf("new_us_per_frame is %f, %d\n", newus, new_us_per_frame);
    avifile.seek( 0x20 , SeekSet);
    print_quartet(new_us_per_frame, avifile);


    idxfile.close();
    avifile.close();
    outfile.close();

    int xx = SD_MMC.remove(idx_file_name);
  }
}

void re_index( char * avi_file_name, char * out_file_name) {

  //once++;
  //if (once > 1) return;

  extern uint8_t* framebuffer;
  uint16_t remnant = 0;

  // JamCam0005.0037.avi
  //const char * avi_file_name = "JamCam0090.0001.avi";
  const char * idx_file_name = "/idx.tmp"; // "/JamCam0190.0001.idx";
  //const char * out_file_name = "/JamCam0090.0001new.avi";

#define fbs 4 //  how many kb of static ram for psram -> sram buffer for sd write
  uint8_t framebuffer_static[fbs * 1024 + 20];

  avifile = SD_MMC.open(avi_file_name, "r"); // avifile = SD_MMC.open(avi_file_name, "w");
  idxfile = SD_MMC.open(idx_file_name, "w"); //idxfile = SD_MMC.open("/idx.tmp", "w");
  outfile = SD_MMC.open(out_file_name, "w"); //idxfile = SD_MMC.open("/idx.tmp", "w");

  if (avifile) {
    Serial.printf("File open: %s\n", avi_file_name);
  }  else  {
    Serial.printf("Could not open %s file\n", avi_file_name);
  }

  if (idxfile)  {
    Serial.printf("File open: %s\n", idx_file_name);
  }  else  {
    Serial.printf("Could not open file %s\n", idx_file_name);
  }

  if (!avifile) {
    return;
  } else {
    //size_t err = avifile.read( framebuffer_static, 240);

    avifile.seek( 0x24 , SeekSet);
    int max_bytes_per_sec  = read_quartet( avifile);
    Serial.printf("Max bytes per sec %d\n", max_bytes_per_sec);

    avifile.seek( 0x30 , SeekSet);
    int frame_cnt = read_quartet( avifile);
    Serial.printf("Frames %d\n", frame_cnt);

/*
    if (frame_cnt < frame_start) {
      Serial.printf("Only %d frames, less than %d frame_start -- start at 0\n", frame_cnt, frame_end);
      frame_start = 0;
    }
    // if frame_end is 0, or too high, it will go to frame_cnt,
    if (frame_cnt < frame_end || frame_end == 0) {
      Serial.printf("Only %d frames, less than %d frame_end -- end at max frames\n", frame_cnt, frame_end);
      frame_end = frame_cnt;
    }
*/
    int num_out_frames = frame_cnt;   // / (skip_frames + 1);
    Serial.printf("Original %d frames, so %d output frames\n", frame_cnt, num_out_frames );


    //avifile.seek( 0x8c , SeekSet);
    //int frame_cnt8c = read_quartet( avifile);

    avifile.seek( 0x84 , SeekSet);
    int iAttainedFPS = read_quartet( avifile);
    Serial.printf("fps %d\n", iAttainedFPS);

    avifile.seek( 0xe8 , SeekSet);
    int index_start = read_quartet( avifile);
    Serial.printf("Len of movi %d\n", index_start);

    Serial.printf("-----------------\n");

    avifile.seek( 0 , SeekSet);
    size_t err = avifile.read(framebuffer_static, AVIOFFSET);
    Serial.printf("avi read header %d\n", err);
    size_t err2 = outfile.write(framebuffer_static, AVIOFFSET);
    Serial.printf("avi write header %d\n", err2);
    outfile.seek( 240 , SeekSet);


    Serial.printf("-----------------\n");

    int xx2;
    int flen;
    int prev_frame_length = 0;
    int next_frame_start = 240;
    int new_frame_length;
    int prev_frame_start = 240;
    int index_frame_length;
    int index_frame_start;
    int idx_offset = 4;
    int movi_size = 0;
    int frame_cnt_out = 0;
    int frame_num = 0;
    
    avifile.seek( next_frame_start  , SeekSet);  //240

    bool one_more_frame = true;
    
    //for (int frame_num = 0; frame_num < frame_cnt; frame_num = frame_num + 1) {
    
    while (one_more_frame) {
        
      //i avifile.seek( index_start + 244 + frame_num * 16 + 8 , SeekSet);
      //i index_frame_start = read_quartet( avifile);
      //i index_frame_length = read_quartet( avifile);

      //avifile.seek( next_frame_start  , SeekSet);   // start at 240, then read everything - 2 quart + frame, then repeat
      
      int the_oodc = read_quartet (avifile);   //240
      if (the_oodc == 1667510320) {
        //Serial.printf("%d, good frame, num %d\n",the_oodc,frame_num);
      } else {
        Serial.printf("%d, bad frame, num %d\n",the_oodc,frame_num);
        break;
      }
      
      
      //avifile.seek( next_frame_start + 4  , SeekSet);
      new_frame_length = read_quartet( avifile); //244
      
      index_frame_length = new_frame_length; // reuse the variable
      
      prev_frame_start = next_frame_start;
      index_frame_start = prev_frame_start; // reuse
      
      next_frame_start = prev_frame_start + new_frame_length + 8;
      
      //prev_frame_length = new_frame_length;
      
      if (frame_num < 5 || frame_num % 500 == 0) {
        Serial.printf("Frame %4d, index len %9d, frame len %9d, index start %9d, frame start %9d\n", frame_num, index_frame_length, new_frame_length, index_frame_start + 236 , prev_frame_start);
      }
      
      if (frame_num < 5 || frame_cnt_out % 100 == 0) {
        Serial.printf("Frame %4d, index len %9d, frame len %9d, index start %9d, frame start %9d\n", frame_num, index_frame_length, new_frame_length, index_frame_start + 236 , prev_frame_start);
        Serial.printf("in %d, out %d\n", frame_num, frame_cnt_out);
      }
      
      //avifile.seek( index_frame_start + 244 , SeekSet); // already 248

      remnant = (4 - (index_frame_length & 0x00000003)) & 0x00000003;
      int index_frame_length_rem = index_frame_length + remnant;

      int left_to_write = index_frame_length_rem;

      print_dc_quartet( index_frame_length_rem, outfile);

      while (left_to_write > 0) {
        if (left_to_write > fbs * 1024) {
          size_t err = avifile.read(framebuffer_static, fbs * 1024);
          size_t err2 = outfile.write(framebuffer_static, fbs * 1024);
          left_to_write = left_to_write - fbs * 1024;
        } else {
          size_t err = avifile.read(framebuffer_static, left_to_write);
          size_t err2 = outfile.write(framebuffer_static, left_to_write);
          left_to_write = 0;
        }
      }

      movi_size += index_frame_length;
      movi_size += remnant;

      print_2quartet(idx_offset, index_frame_length, idxfile);

      idx_offset = idx_offset + index_frame_length_rem + 8;
      
      frame_cnt_out++;
      frame_num++;
      
    }  // every frame in file

    
    //Serial.printf("frame %4d, outfile %9d, avifile %9d, idxfile %9d\n", frame_cnt , outfile.position(), avifile.position(), avifile.position());

    idxfile.close();
    size_t i1_err = outfile.write(idx1_buf, 4);
    if (!i1_err) Serial.printf("idx write\n");

    print_quartet(frame_cnt_out * 16, outfile);

    idxfile = SD_MMC.open(idx_file_name, "r");
    if (idxfile)  {
      Serial.printf("File open: %s\n", idx_file_name);
    }  else  {
      Serial.printf("Could not open file %s\n", idx_file_name);
    }

    char * AteBytes;
    AteBytes = (char*) malloc (8);

    for (int i = 0; i < frame_cnt_out; i++) {
      size_t res = idxfile.readBytes( AteBytes, 8);
      if (!res) Serial.printf("idx read\n");
      size_t i1_err = outfile.write(dc_and_zero_buf, 8);
      if (!i1_err) Serial.printf("dc write\n");
      //size_t i2_err = outfile.write(zero_buf, 4);
      //if (!i2_err) Serial.printf("zero write\n");
      size_t i3_err = outfile.write((uint8_t *)AteBytes, 8);
      if (!i3_err) Serial.printf("ate write\n");
    }

    free(AteBytes);

    outfile.seek( 4 , SeekSet);         //shit
    print_quartet(movi_size + 240 + 16 * frame_cnt_out + 8 * frame_cnt_out, outfile);
    
    avifile.seek( 0xe8 , SeekSet);
    int lom = read_quartet( avifile);
    Serial.printf("Len of movi was %d, now is %d\n", lom, movi_size);
    
    outfile.seek( 0xe8 , SeekSet);
    // shit print_quartet (movi_size, outfile);
    print_quartet(movi_size + frame_cnt_out * 8 + 4, outfile);
     
    avifile.seek( 0x30 , SeekSet);
    int fc = read_quartet( avifile);
    Serial.printf("Frames was %d, now is %d\n", fc, frame_cnt_out);
    outfile.seek( 0x30 , SeekSet);
    print_quartet (frame_cnt_out, outfile);
    outfile.seek( 0x8c , SeekSet);
    print_quartet (frame_cnt_out, outfile);

    //avifile.seek( 0x84 , SeekSet);
    //int fps = read_quartet( avifile);
    //Serial.printf("fps was %d\n", fps);

    //float fnewfps = ( 1.0f * fps ) / (1 + skip_frames)   ;
    //int newfps = round(fnewfps);
  
    //Serial.printf("newfps is %f, %d\n", fnewfps, newfps);
    //avifile.seek( 0x84 , SeekSet);
    //print_quartet(newfps, avifile);

    //avifile.seek( 0x20 , SeekSet);
    //int us_per_frame = read_quartet( avifile);
    //Serial.printf("us_per_frame was %d\n", us_per_frame);

    //float newus = 1000000.0f / fnewfps;
    //uint32_t new_us_per_frame = round (newus);
    
  
    //Serial.printf("new_us_per_frame is %f, %d\n", newus, new_us_per_frame);
    //avifile.seek( 0x20 , SeekSet);
    //print_quartet(new_us_per_frame, avifile);


    idxfile.close();
    avifile.close();
    outfile.close();

    int xx = SD_MMC.remove(idx_file_name);
  }
}

static void start_avi() {
  //#define BUFFSIZE 512
  //uint8_t buf[BUFFSIZE];

#define BUFFSIZE 256
  uint8_t buf[BUFFSIZE];

  struct tm timeinfo;
  time_t now;

  Serial.println("Starting an avi ");

  which_core = 0;
  which_core_samples = 0;

  time(&now);
  localtime_r(&now, &timeinfo);
  char strftime_buf[64];

  strftime(strftime_buf, sizeof(strftime_buf), "%F_%H.%M.%S", &timeinfo);

  sprintf(avi_file_name, "/%s%04d.%04d.avi",  "JamCam", file_group, file_number);

  file_number++;

  avifile = SD_MMC.open(avi_file_name, "w");
  idxfile = SD_MMC.open("/idx.tmp", "w");

  if (avifile) {
    Serial.printf("File open: %s\n", avi_file_name);
  }  else  {
    Serial.println("Could not open file");
  }

  if (idxfile)  {
    //Serial.printf("File open: %s\n", "//idx.tmp");
  }  else  {
    Serial.println("Could not open file /idx.tmp");
  }

  for ( i = 0; i < AVIOFFSET; i++) {
    char ch = pgm_read_byte(&avi_header[i]);
    buf[i] = ch;
  }

  memcpy(buf + 0x40, frameSizeData[framesize].frameWidth, 2);
  memcpy(buf + 0xA8, frameSizeData[framesize].frameWidth, 2);
  memcpy(buf + 0x44, frameSizeData[framesize].frameHeight, 2);
  memcpy(buf + 0xAC, frameSizeData[framesize].frameHeight, 2);

  size_t err = avifile.write(buf, AVIOFFSET);

  uint8_t ex_fps = 1;
  if (frame_interval == 0) {
    if (framesize >= 11) {
      ex_fps = 12.5 * speed_up_factor ;
    } else {
      ex_fps = 25.0 * speed_up_factor;
    }
  } else {
    ex_fps = round(1000.0 / frame_interval * speed_up_factor);
  }

  avifile.seek( 0x84 , SeekSet);
  print_quartet((int)ex_fps, avifile);

  avifile.seek( AVIOFFSET, SeekSet);

  Serial.print(F("\nRecording "));
  Serial.print(avi_length);
  Serial.println(" seconds.");

  startms = millis();

  jpeg_size = 0;
  movi_size = 0;
  uVideoLen = 0;
  idx_offset = 4;

  avifile.flush();

} // end of start avi

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  another_save_avi saves another frame to the avi file, uodates index
//           -- pass in a fb pointer to the frame to add
//
extern int stats_rec;
extern int recent_frame_write;
static void another_save_avi(camera_fb_t * fb ) {
  which_core +=  xPortGetCoreID(); //xTaskGetCoreID(NULL);
  which_core_samples++;
  stats_rec++;
  int fblen;
  fblen = fb->len;

  int fb_block_length;
  uint8_t* fb_block_start;

  jpeg_size = fblen;

  remnant = (4 - (jpeg_size & 0x00000003)) & 0x00000003;

  long bw = millis();
  int frame_write_start = millis();

  framebuffer_static[0] = 0x30;       // "00dc"
  framebuffer_static[1] = 0x30;
  framebuffer_static[2] = 0x64;
  framebuffer_static[3] = 0x63;

  int jpeg_size_rem = jpeg_size + remnant;

  framebuffer_static[4] = jpeg_size_rem % 0x100;
  framebuffer_static[5] = (jpeg_size_rem >> 8) % 0x100;
  framebuffer_static[6] = (jpeg_size_rem >> 16) % 0x100;
  framebuffer_static[7] = (jpeg_size_rem >> 24) % 0x100;

  fb_block_start = fb->buf;

  if (fblen > fbs * 1024 - 8 ) {                     // fbs is the size of frame buffer static
    fb_block_length = fbs * 1024;
    fblen = fblen - (fbs * 1024 - 8);
    memcpy(framebuffer_static + 8, fb_block_start, fb_block_length - 8);
    fb_block_start = fb_block_start + fb_block_length - 8;

  } else {
    fb_block_length = fblen + 8  + remnant;
    memcpy(framebuffer_static + 8, fb_block_start,  fblen);
    fblen = 0;
  }

  size_t err = avifile.write(framebuffer_static, fb_block_length);

  if (err != fb_block_length) {
    Serial.print("Error on avi write: err = "); Serial.print(err);
    Serial.print(" len = "); Serial.println(fb_block_length);
    start_record = 0;
  }

  while (fblen > 0) {

    if (fblen > fbs * 1024) {
      fb_block_length = fbs * 1024;
      fblen = fblen - fb_block_length;
    } else {
      fb_block_length = fblen  + remnant;
      fblen = 0;
    }

    memcpy(framebuffer_static, fb_block_start, fb_block_length);

    size_t err = avifile.write(framebuffer_static,  fb_block_length);

    if (err != fb_block_length) {
      Serial.print("Error on avi write: err = "); Serial.print(err);
      Serial.print(" len = "); Serial.println(fb_block_length);
      start_record = 0;
    }

    fb_block_start = fb_block_start + fb_block_length;
    //delay(0);
  }


  movi_size += jpeg_size;
  uVideoLen += jpeg_size;

  print_2quartet(idx_offset, jpeg_size, idxfile);

  idx_offset = idx_offset + jpeg_size + remnant + 8;

  movi_size = movi_size + remnant;

  avifile.flush();
  delay(0); // from above to not give up time slice

  recent_frame_write = millis() - frame_write_start;

} // end of another_pic_avi

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  end_avi writes the index, and closes the files
//

static void end_avi() {

  unsigned long current_end = avifile.position();

  Serial.println("End of avi - closing the files");

  if (frame_cnt <  5 ) {
    Serial.println("Recording screwed up, less than 5 frames, forget index\n");
    idxfile.close();
    avifile.close();
    int xx = SD_MMC.remove("/idx.tmp");
    int yy = SD_MMC.remove(avi_file_name);

  } else {

    elapsedms = millis() - startms;

    float fRealFPS = (1000.0f * (float)frame_cnt) / ((float)elapsedms) * speed_up_factor;

    float fmicroseconds_per_frame = 1000000.0f / fRealFPS;
    uint8_t iAttainedFPS = round(fRealFPS) ;
    uint32_t us_per_frame = round(fmicroseconds_per_frame);

    //Modify the MJPEG header from the beginning of the file, overwriting various placeholders

    avifile.seek( 4 , SeekSet);
    print_quartet(movi_size + 240 + 16 * frame_cnt + 8 * frame_cnt, avifile);

    avifile.seek( 0x20 , SeekSet);
    print_quartet(us_per_frame, avifile);

    unsigned long max_bytes_per_sec = (1.0f * movi_size * iAttainedFPS) / frame_cnt;

    avifile.seek( 0x24 , SeekSet);
    print_quartet(max_bytes_per_sec, avifile);

    avifile.seek( 0x30 , SeekSet);
    print_quartet(frame_cnt, avifile);

    avifile.seek( 0x8c , SeekSet);
    print_quartet(frame_cnt, avifile);

    avifile.seek( 0x84 , SeekSet);
    print_quartet((int)iAttainedFPS, avifile);

    avifile.seek( 0xe8 , SeekSet);
    print_quartet(movi_size + frame_cnt * 8 + 4, avifile);

    Serial.println(F("\n*** Video recorded and saved ***\n"));

    Serial.printf("Recorded %5d frames in %5d seconds\n", frame_cnt, elapsedms / 1000);
    Serial.printf("File size is %u bytes\n", movi_size + 12 * frame_cnt + 4);
    Serial.printf("Adjusted FPS is %5.2f\n", fRealFPS);
    Serial.printf("Max data rate is %lu bytes/s\n", max_bytes_per_sec);
    Serial.printf("Frame duration is %d us\n", us_per_frame);
    Serial.printf("Average frame length is %d bytes\n", uVideoLen / frame_cnt);

    Serial.printf("Writng the index, %d frames\n", frame_cnt);
    Serial.printf("Which Core %d, samples %d, Avg %3.2f\n", which_core, which_core_samples, 1.0 * which_core / which_core_samples);
    avifile.seek( current_end , SeekSet);

    idxfile.close();

    size_t i1_err = avifile.write(idx1_buf, 4);

    print_quartet(frame_cnt * 16, avifile);

    idxfile = SD_MMC.open("/idx.tmp", "r");

    if (idxfile)  {
      //Serial.printf("File open: %s\n", "//idx.tmp");
    }  else  {
      Serial.println("Could not open index file");
    }

    char * AteBytes;
    AteBytes = (char*) malloc (8);

    for (int i = 0; i < frame_cnt; i++) {
      size_t res = idxfile.readBytes( AteBytes, 8);
      size_t i1_err = avifile.write(dc_buf, 4);
      size_t i2_err = avifile.write(zero_buf, 4);
      size_t i3_err = avifile.write((uint8_t *)AteBytes, 8);
    }

    free(AteBytes);

    idxfile.close();
    avifile.close();

    int xx = SD_MMC.remove("/idx.tmp");
    sprintf(avi_file_name, "%s",  "Not recording");
  }

  Serial.println("---");
}

void the_camera_loop (void* pvParameter) {

  Serial.print("the camera loop, core ");  Serial.print(xPortGetCoreID());
  Serial.print(", priority = "); Serial.println(uxTaskPriorityGet(NULL));

  //init_sdcard();
  frame_cnt = 0;
  speed_up_factor = 1;
  avi_length = 600;
  frame_interval = 35; //40 - couple ms

  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
  uint64_t total =  SD_MMC.totalBytes();
  uint64_t used =  SD_MMC.usedBytes();
  freespace = (int)((total - used) / (1024 * 1024));

  start_record = 0;

  delay(1000);

  sprintf(avi_file_name, "%s",  "Not recording");

  while (1) {

    // if (frame_cnt == 0 && start_record == 0)  // do nothing
    // if (frame_cnt == 0 && start_record == 1)  // start a movie
    // if (frame_cnt > 0 && start_record == 0)   // stop the movie
    // if (frame_cnt > 0 && start_record != 0)   // another frame

    ///////////////////  NOTHING TO DO //////////////////
    if (frame_cnt == 0 && start_record == 0) {

      //Serial.println("Do nothing");
      delay(50);

      ///////////////////  START A MOVIE  //////////////////
    } else if (frame_cnt == 0 && start_record == 1) {

      //Serial.println("Ready to start");

      avi_start_time = millis();
      Serial.printf("\nStart the avi ... at %d\n", avi_start_time);
      Serial.printf("Framesize %d, length %d seconds\n\n", framesize, avi_length);

      frame_cnt++;
      fb_curr = get_good_jpeg();

      start_avi();

      another_save_avi( fb_curr);

      xSemaphoreTake( baton, portMAX_DELAY );
      framebuffer_len = fb_curr->len;
      memcpy(framebuffer, fb_curr->buf, fb_curr->len);
      framebuffer_time = millis();
      framebuffer_good = true;

      xSemaphoreGive( baton );

      esp_camera_fb_return(fb_curr);

      if (blinking) digitalWrite(33, frame_cnt % 2);                // blink

      ///////////////////  END THE MOVIE //////////////////
    } else if (  (frame_cnt > 0 && start_record == 0) ||  millis() > (avi_start_time + avi_length * 1000)) { // end the avi

      Serial.println("End the Avi");

      end_avi();                                // end the movie

      if (blinking) digitalWrite(33, HIGH);          // light off

      delay(50);

      avi_end_time = millis();

      float fps = 1.0 * frame_cnt / ((avi_end_time - avi_start_time) / 1000) ;

      Serial.printf("End the avi at %d.  It was %d frames, %d ms at %.2f fps...\n", millis(), frame_cnt, avi_end_time, avi_end_time - avi_start_time, fps);
      Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
      Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
      uint64_t total =  SD_MMC.totalBytes();
      uint64_t used =  SD_MMC.usedBytes();
      freespace = (int)((total - used) / (1024 * 1024));
      delete_old_stuff_flag = 1;
      frame_cnt = 0;             // start recording again on the next loop

      ///////////////////  ANOTHER FRAME  //////////////////
    } else if (frame_cnt > 0 && start_record != 0) {  // another frame of the avi

      //Serial.println("Another frame");

      current_frame_time = millis();
      if (current_frame_time - last_frame_time < frame_interval) {
        delay(frame_interval - (current_frame_time - last_frame_time));             // delay for timelapse
      }
      last_frame_time = millis();

      frame_cnt++;
      fb_curr = get_good_jpeg();
      another_save_avi( fb_curr);

      xSemaphoreTake( baton, portMAX_DELAY );
      framebuffer_len = fb_curr->len;
      memcpy(framebuffer, fb_curr->buf, fb_curr->len);
      framebuffer_time = millis();
      framebuffer_good = true;
      //xSemaphoreGive( baton );
      ///
      if (net_frame_avail) {
        // skip this frame as netword is busy
      } else {
        //memcpy(&framebuffer2, framebuffer3, framebuffer3_len);
        memcpy(framebuffer2, fb_curr->buf, fb_curr->len);
        //5
        framebuffer2_len = fb_curr->len;
        framebuffer2_time = millis();
        net_frame_avail = true;
      }
      ///
      xSemaphoreGive( baton );

      esp_camera_fb_return(fb_curr);

      if (blinking) digitalWrite(33, frame_cnt % 2);

    }
    delay(0);
  }
}
