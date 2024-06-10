#include "utils_sd.h"

const char *SD_TAG = "SD";
const char *SD_FILE_CONFIG = "/config.json";

volatile bool SD_STATE;

SPIClass *SD_spi = NULL;
void SD_SPI_init() {
  SD_spi = new SPIClass();
  SD_spi->begin(48, 41, 47, 42);
}


bool SD_init() {
  // SD_spi->begin(48, 41, 47, 42);
  if (!SD.begin(42, *SD_spi)) {
    // SD_spi->end();
    SD_STATE = false;
    return false;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE || cardType == CARD_UNKNOWN) {
    ESP_LOGI(SD_TAG, "No SD card attached or unknown");
    SD_STATE = false;
    return false;
  }

  delay(500);

  if (!SD.exists(SD_FILE_CONFIG))
        SD_writeFile(SD, SD_FILE_CONFIG, "{}");

  SD_STATE = true;
  return true;
}

bool SD_INSERTED() {
  if (!SD_STATE) return false;

  for (int i = 0; i < 2; i++) {
    if (SD.exists(SD_FILE_CONFIG)) return true;
  }
  SD_init();
  for (int i = 0; i < 2; i++) {
    if (SD.exists(SD_FILE_CONFIG)) return true;
  }
  return false;
}


void SD_get_info(SD_info& __SD_info){
  __SD_info.type = SD.cardType();
  __SD_info.size = SD.cardSize();
  __SD_info.size_total = SD.totalBytes();
  __SD_info.size_used = SD.usedBytes();
}

void SD_writeFile(fs::FS &fs, const char *path, const char *message) {
  // if (!SD_INSERTED()) return;

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI(SD_TAG, "Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI("SD", "File written");
  } else {
    ESP_LOGI("SD", "Write failed");
  }
  file.close();
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

void SD_appendFile(fs::FS &fs, const char *path, const char *message) {
  // if (!SD_INSERTED()) return;

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    ESP_LOGI("SD", "Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(SD_TAG,"Appended to file: %s\n", path);
  } else {
    ESP_LOGI("SD", "Append failed");
  }
  file.close();
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