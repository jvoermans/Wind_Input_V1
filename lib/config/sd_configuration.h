#ifndef SD_CONFIGURATION_H
#define SD_CONFIGURATION_H

#include "SdFat.h"

// which kind of card format is used
// this is what works on my 32 GB SD card
typedef SdFs sd_t;
typedef FsFile file_t;
// may need to use ExFat so that can have large SD cards
// typedef SdExFat sd_t;
// typedef ExFile file_t;

#endif