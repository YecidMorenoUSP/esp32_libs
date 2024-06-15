#include "utils_sd.h"
#include <freertos/semphr.h>

#define TIME_TO_ACCESS_SD 5000/portTICK_PERIOD_MS

SemaphoreHandle_t SD_Mutex;

const char *SD_TAG = "SD";
const char *SD_FILE_CONFIG = "/config.json";

volatile bool SD_STATE;

int8_t SD_sck=48;
int8_t SD_miso=41;
int8_t SD_mosi=47;
int8_t SD_ss=42;

bool SD_init_Mutex(){
  if (SD_Mutex != NULL) return true;

  SD_Mutex = xSemaphoreCreateMutex();
  if (SD_Mutex == NULL) {
      ESP_LOGI(SD_TAG,"Failed to create the mutex.\n");
      return false;
  }
  return true;
}

SPIClass *SD_spi = NULL;
void SD_SPI_init(int8_t _sck,int8_t _miso,int8_t _mosi,int8_t _ss) {
  SD_sck = _sck;
  SD_miso = _miso;
  SD_mosi = _mosi;
  SD_ss = _ss;

  SD_spi = new SPIClass();
  SD_spi->begin(_sck,_miso,_mosi,_ss);

  
}

bool SD_takeMutex(){
  return xSemaphoreTake(SD_Mutex, portMAX_DELAY) == pdTRUE;
}

void SD_releaseMutex(){
    xSemaphoreGive(SD_Mutex);
}

bool SD_init() {
  // SD_spi->begin(48, 41, 47, 42);
  if (xSemaphoreTake(SD_Mutex, portMAX_DELAY) == pdTRUE){
    if (!SD.begin(SD_ss, *SD_spi)) {
      // SD_spi->end();
      SD_STATE = false;
      xSemaphoreGive(SD_Mutex);
      return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE || cardType == CARD_UNKNOWN) {
      ESP_LOGI(SD_TAG, "No SD card attached or unknown");
      SD_STATE = false;
      xSemaphoreGive(SD_Mutex);
      return false;
    }

    delay(500);

    if (!SD.exists(SD_FILE_CONFIG))
          SD_writeFile(SD, SD_FILE_CONFIG, "{}");

    xSemaphoreGive(SD_Mutex);

    SD_STATE = true;
    return true;

  }else{
    return false;
  }
}

bool SD_INSERTED() {
  if (xSemaphoreTake(SD_Mutex, TIME_TO_ACCESS_SD) == pdTRUE){
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE || cardType == CARD_UNKNOWN) {
      SD_STATE = false;
      xSemaphoreGive(SD_Mutex);
      return false;
    }
    xSemaphoreGive(SD_Mutex);
    SD_STATE = true;
    return true;
  }else{
    return false;
  }
}


void SD_get_info(SD_info& __SD_info){
  if (xSemaphoreTake(SD_Mutex, TIME_TO_ACCESS_SD) == pdTRUE){
  __SD_info.type = SD.cardType();
  __SD_info.size = SD.cardSize();
  __SD_info.size_total = SD.totalBytes();
  __SD_info.size_used = SD.usedBytes();
    xSemaphoreGive(SD_Mutex);
  }
}

void SD_writeFile(fs::FS &fs, const char *path, const char *message) {
  // if (!SD_INSERTED()) return;
  if (xSemaphoreTake(SD_Mutex, TIME_TO_ACCESS_SD) == pdTRUE){

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI(SD_TAG, "Failed to open file for writing");
    xSemaphoreGive(SD_Mutex);
    return;
  }
  if (file.print(message)) {
    ESP_LOGI("SD", "File written");
  } else {
    ESP_LOGI("SD", "Write failed");
  }
  file.close();
  xSemaphoreGive(SD_Mutex);
  }
}

size_t SD_readFileOpennedUntil(File& file,char* buff, size_t initPos, char delimiter){
  file.seek(initPos,SeekSet);

  size_t newPos = 0;
  while (file.available()) {
    buff[newPos] = file.read();
    if (buff[newPos] == delimiter) {
        buff[newPos]='\0';
        return newPos;
        break;
    }
    newPos++;
  }

  return 0;
  
}

bool SD_cropfile(fs::FS &fs, const char *path,size_t initPos){
  if (xSemaphoreTake(SD_Mutex, TIME_TO_ACCESS_SD) == pdTRUE){

  File file = fs.open(path);

  if (!file) {
    ESP_LOGI(SD_TAG, "Failed to open file for reading");
    xSemaphoreGive(SD_Mutex);
    return false;
  }

  File tempFile = SD.open("/tempFile.txt", FILE_WRITE);
  if (!tempFile) {
      file.close();
      xSemaphoreGive(SD_Mutex);
      return false;
  }

  file.seek(initPos,SeekSet);
  while (file.available()) {
      tempFile.write(file.read());
  }

  file.close();
  tempFile.close();

  fs.remove(path);
  fs.rename("/tempFile.txt", path);
  xSemaphoreGive(SD_Mutex);
  return true;

  }else{
    return false;
  }
}

void SD_readFile(fs::FS &fs, const char *path) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    ESP_LOGI(SD_TAG, "Failed to open file for reading");
    return;
  }

  ESP_LOGI(SD_TAG, "Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void SD_appendFile(fs::FS &fs, const char *path, const char *message,char delimiter) {
  // if (!SD_INSERTED()) return;
  if (xSemaphoreTake(SD_Mutex, TIME_TO_ACCESS_SD) == pdTRUE){

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    ESP_LOGI("SD", "Failed to open file for appending");
    xSemaphoreGive(SD_Mutex);
    return;
  }
  if (file.print(message)) {
    if(delimiter != NULL)
       file.print(delimiter);
    ESP_LOGI(SD_TAG,"Appended to file: %s\n", path);
  } else {
    ESP_LOGI("SD", "Append failed");
  }
  file.close();
  xSemaphoreGive(SD_Mutex);
  }
}

void SD_renameFile(fs::FS &fs, const char *path1, const char *path2) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    ESP_LOGI("SD", "File renamed");
  } else {
    ESP_LOGI("SD", "Rename failed");
  }
}

void SD_deleteFile(fs::FS &fs, const char *path) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Deleting file: %s\n", path);
  if (fs.remove(path)) {
    ESP_LOGI("SD", "File deleted");
  } else {
    ESP_LOGI("SD", "Delete failed");
  }
}

void SD_listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    ESP_LOGI(SD_TAG, "Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    ESP_LOGI(SD_TAG, "Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      ESP_LOGI(SD_TAG, "  DIR : ");
      ESP_LOGI(SD_TAG, "%s", file.name());
      if (levels) {
        SD_listDir(fs, file.path(), levels - 1);
      }
    } else {
      ESP_LOGI(SD_TAG, "  FILE: ");
      ESP_LOGI(SD_TAG, "%s", file.name());
      ESP_LOGI(SD_TAG, "  SIZE: ");
      ESP_LOGI(SD_TAG, "%d", file.size());
    }
    file = root.openNextFile();
  }
}

void SD_createDir(fs::FS &fs, const char *path) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    ESP_LOGI(SD_TAG, "Dir created");
  } else {
    ESP_LOGI(SD_TAG, "mkdir failed");
  }
}

void SD_removeDir(fs::FS &fs, const char *path) {
  // if (!SD_INSERTED()) return;

  ESP_LOGI(SD_TAG,"Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    ESP_LOGI(SD_TAG, "Dir removed");
  } else {
    ESP_LOGI(SD_TAG, "rmdir failed");
  }
}

void SD_testFileIO(fs::FS &fs, const char *path) {
  // if (!SD_INSERTED()) return;

  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    ESP_LOGI(SD_TAG,"%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    ESP_LOGI("SD", "Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI("SD", "Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  ESP_LOGI(SD_TAG,"%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}