/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

// #include "FS.h"
// #include "SD.h"
// #include "SPI.h"

#include "utils_sd.h"

SD_info _SD_info;

void setup() {
  delay(1000);
  Serial.begin(115200);

  ESP_LOGI("INIT", "\n\n");

  SD_init_Mutex();
  SD_SPI_init(14,2,15,13);

  //esp32484840s0
  // SD_SPI_init(48,41,47,42);
  
  if (SD_init()) {
    ESP_LOGI(SD_TAG, "OK");
  } else {
    ESP_LOGI(SD_TAG, "FAIL");
  }
  
  SD_get_info(_SD_info);

  ESP_LOGI(SD_TAG,"SD Card Type: %d\n", _SD_info.type);
  ESP_LOGI(SD_TAG,"SD Card Size: %lluMB\n", _SD_info.size / (1024 * 1024));
  ESP_LOGI(SD_TAG,"Total space: %lluMB\n", _SD_info.size_total / (1024 * 1024));
  ESP_LOGI(SD_TAG,"Used space: %lluMB\n", _SD_info.size_used / (1024 * 1024));

  if (SD_INSERTED()) {
    ESP_LOGI(SD_TAG, "Exist OK");
  } else {
    ESP_LOGI(SD_TAG, "Exist FAIL");
  }  

  SD_appendFile(SD, "/verbose.log", "World!\n");

  // listDir(SD, "/", 0);
  // createDir(SD, "/mydir");
  // listDir(SD, "/", 0);
  // removeDir(SD, "/mydir");
  // listDir(SD, "/", 2);
  // writeFile(SD, "/hello.txt", "Hello ");
  // appendFile(SD, "/hello.txt", "World!\n");
  // readFile(SD, "/hello.txt");
  // deleteFile(SD, "/foo.txt");
  // renameFile(SD, "/hello.txt", "/foo.txt");
  // readFile(SD, "/foo.txt");
  // testFileIO(SD, "/test.txt");

}

void loop() {
}
