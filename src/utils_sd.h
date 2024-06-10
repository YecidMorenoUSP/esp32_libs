#ifndef _UTILS_USE_SD
#define _UTILS_USE_SD

#include "FS.h"
#include "SD.h"
#include "SPI.h"

extern const char *SD_TAG;
extern const char *SD_FILE_CONFIG;
extern fs::SDFS SD;


typedef struct{
    uint64_t size;
    uint64_t size_total;
    uint64_t size_used;
    uint8_t type;
} SD_info;

bool SD_init();
void SD_SPI_init();
bool SD_INSERTED();
void SD_writeFile(fs::FS &fs, const char *path, const char *message);
void SD_get_info(SD_info& __SD_info);
void SD_readFile(fs::FS &fs, const char *path);
void SD_appendFile(fs::FS &fs, const char *path, const char *message);
void SD_renameFile(fs::FS &fs, const char *path1, const char *path2);
void SD_deleteFile(fs::FS &fs, const char *path);
void SD_listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void SD_createDir(fs::FS &fs, const char *path);
void SD_removeDir(fs::FS &fs, const char *path);
void SD_testFileIO(fs::FS &fs, const char *path);

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif


#endif