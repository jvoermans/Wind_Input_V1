#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "sd_configuration.h"

#include "gnss_manager.h"

#include <SPI.h>
#include "SdFat.h"

#include "watchdog_manager.h"

#include "data_manager.h"

class SD_Manager{
    public:
        // filename is in the format YYYY-MM-DD-HH-MM.dat
        void update_filename(GNSS_simple_fix const & fix_in);

        // write a boot log message
        void log_boot(void);

        // write a full data file
        void log_data(void);

    private:
        // make everything ready to use
        // start the SD card, SPI etc
        // open file
        void start();

        // stop the SD logging, to be ready to sleep etc
        // close file
        // stop SD card, SPI etc
        void stop();

        char sd_filename[24];  // 21 should be enough, but a bit of margin and alignment

        file_t sd_file;
        sd_t sd_card;
};

extern SD_Manager sd_manager_instance;

#endif