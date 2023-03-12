#include "sd_manager.h"

SD_Manager sd_manager_instance;

// for doing manipulations on strings
static constexpr size_t work_buffer_size{32};
char work_buffer[work_buffer_size];

void SD_Manager::start()
{
    // start the SD card
    SdSpiConfig sd_config{SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(SD_SPI_MHZ)};

    while (!sd_card.begin(sd_config))
    {
        if (use_usb)
        {
            SERIAL_USB->println(F("ERR Cannot start SD card"));
        }

        // watchdog reboot
        while (true)
        {
        }
    }

    delay(100);
    wdt.restart();

    // open the file using the filename that is already set
    if (!sd_file.open(sd_filename, O_RDWR | O_CREAT))
    {
        if (use_usb)
        {
            Serial.println(F("ERR cannot open file"));
        }

        while (true){};
    }

    delay(100);
    wdt.restart();

    // at this point, ready to write etc to file
}

void SD_Manager::stop()
{
    // flush to the SD card to make sure all is written
    sd_file.flush();
    delay(100);
    wdt.restart();

    // close the file
    if (!sd_file.close())
    {
        if (use_usb)
        {
            SERIAL_USB->println(F("ERR cannot close file"));
        }
        while (true)
        {
        };
    }
    delay(100);
    wdt.restart();

    // stop the SD card, stop SPI etc so that ready to sleep, restart, etc
    sd_card.end();
    delay(100);
    wdt.restart();
}

void SD_Manager::update_filename(GNSS_simple_fix const &fix_in)
{
    // YYYY- (5 chars)
    snprintf(&(sd_filename[0]), 4 + 1, "%04u", fix_in.year);
    sd_filename[4] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[5]), 2 + 1, "%02u", fix_in.month);
    sd_filename[7] = '-';
    // DD- (3 chars)
    snprintf(&(sd_filename[8]), 2 + 1, "%02u", fix_in.day);
    sd_filename[10] = '-';
    // HH- (3 chars)
    snprintf(&(sd_filename[11]), 2 + 1, "%02u", fix_in.hour);
    sd_filename[13] = '-';
    // MM. (3 chars)
    snprintf(&(sd_filename[14]), 2 + 1, "%02u", fix_in.minute);
    sd_filename[16] = '.';
    // .dat
    sd_filename[17] = 'd';
    sd_filename[18] = 'a';
    sd_filename[19] = 't';
    sd_filename[20] = '\0';
}

void SD_Manager::update_filename()
{
    kiss_calendar_time crrt_calendar_time;
    posix_to_calendar(board_time_manager.get_posix_timestamp(), &crrt_calendar_time);

    // YYYY- (5 chars)
    snprintf(&(sd_filename[0]), 4 + 1, "%04u", crrt_calendar_time.year);
    sd_filename[4] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[5]), 2 + 1, "%02u", crrt_calendar_time.month);
    sd_filename[7] = '-';
    // DD- (3 chars)
    snprintf(&(sd_filename[8]), 2 + 1, "%02u", crrt_calendar_time.day);
    sd_filename[10] = '-';
    // HH- (3 chars)
    snprintf(&(sd_filename[11]), 2 + 1, "%02u", crrt_calendar_time.hour);
    sd_filename[13] = '-';
    // MM. (3 chars)
    snprintf(&(sd_filename[14]), 2 + 1, "%02u", crrt_calendar_time.minute);
    sd_filename[16] = '.';
    // .dat
    sd_filename[17] = 'd';
    sd_filename[18] = 'a';
    sd_filename[19] = 't';
    sd_filename[20] = '\0';
}

void SD_Manager::log_boot(void){
    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nBOOT\n\n\n\nGNSS_binary\n\n"));
    sd_file.write((void *)(&current_fix_start), sizeof(GNSS_simple_fix));
    sd_file.print(F("\n\nGNSS_binary_done\n\n\n\n"));
    sd_file.print(F("GNSS_string\n\n"));
    sd_file.println(current_fix_start.year);
    sd_file.println(current_fix_start.month);
    sd_file.println(current_fix_start.day);
    sd_file.println(current_fix_start.hour);
    sd_file.println(current_fix_start.minute);
    sd_file.println(current_fix_start.second);
    sd_file.println(current_fix_start.latitude);
    sd_file.println(current_fix_start.lat_NS);
    sd_file.println(current_fix_start.longitude);
    sd_file.print(current_fix_start.lon_EW);
    sd_file.write("\n\nGNSS_string_done\n\n\n\n");
    delay(100);
    wdt.restart();
    sd_file.print(F("BOOT_done\n\n"));

    stop();
    delay(100);
    wdt.restart();
}

void SD_Manager::log_data(void){
    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nDATA\n\n\n\nGNSS_start\n\n"));
    wdt.restart();
    sd_file.write((void *)(&current_fix_start), sizeof(GNSS_simple_fix));
    wdt.restart();
    sd_file.print(F("\n\nGNSS_start_done\n\n\n\nGNSS_end\n\n"));
    wdt.restart();
    sd_file.write((void *)(&current_fix_end), sizeof(GNSS_simple_fix));
    wdt.restart();
    sd_file.print(F("\n\nGNSS_end_done\n\n\n\n"));
    wdt.restart();
    delay(100);
    wdt.restart();

    sd_file.print(F("loopidx_start_string\n\n"));
    sd_file.print(loop_idx);
    sd_file.write("\n\nloopidx_done\n\n\n\n");

    sd_file.print(F("GNSS_start_string\n\n"));
    sd_file.println(current_fix_start.year);
    sd_file.println(current_fix_start.month);
    sd_file.println(current_fix_start.day);
    sd_file.println(current_fix_start.hour);
    sd_file.println(current_fix_start.minute);
    sd_file.println(current_fix_start.second);
    sd_file.println(current_fix_start.latitude);
    sd_file.println(current_fix_start.lat_NS);
    sd_file.println(current_fix_start.longitude);
    sd_file.print(current_fix_start.lon_EW);
    sd_file.write("\n\nGNSS_start_string_done\n\n\n\n");

    sd_file.print(F("GNSS_end_string\n\n"));
    sd_file.println(current_fix_end.year);
    sd_file.println(current_fix_end.month);
    sd_file.println(current_fix_end.day);
    sd_file.println(current_fix_end.hour);
    sd_file.println(current_fix_end.minute);
    sd_file.println(current_fix_end.second);
    sd_file.println(current_fix_end.latitude);
    sd_file.println(current_fix_end.lat_NS);
    sd_file.println(current_fix_end.longitude);
    sd_file.print(current_fix_end.lon_EW);
    sd_file.write("\n\nGNSS_end_string_done\n\n\n\n");

    // write((void *)(samples.data()), sizeof(uint16_t) * samples.max_size());

    sd_file.print(F("accX_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(accX.data()), sizeof(uint16_t) * accX.max_size());
    wdt.restart();
    sd_file.print(F("\n\naccX_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("accY_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(accY.data()), sizeof(uint16_t) * accY.max_size());
    wdt.restart();
    sd_file.print(F("\n\naccY_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("accZ_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(accZ.data()), sizeof(uint16_t) * accZ.max_size());
    wdt.restart();
    sd_file.print(F("\n\naccZ_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("pitch_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(pitch.data()), sizeof(uint16_t) * pitch.max_size());
    wdt.restart();
    sd_file.print(F("\n\npitch_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("roll_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(roll.data()), sizeof(uint16_t) * roll.max_size());
    wdt.restart();
    sd_file.print(F("\n\nroll_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("accD_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(accD.data()), sizeof(uint32_t) * accD.max_size());
    wdt.restart();
    sd_file.print(F("\n\naccD_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("press1_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(press1.data()), sizeof(uint32_t) * press1.max_size());
    wdt.restart();
    sd_file.print(F("\n\npress1_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("press2_array\n\n"));
    wdt.restart();
    sd_file.write((void *)(press2.data()), sizeof(uint32_t) * press2.max_size());
    wdt.restart();
    sd_file.print(F("\n\npress2_array_done\n\n\n\n"));
    wdt.restart();

    sd_file.print(F("DATA-done\n\n"));

    stop();
    delay(100);
    wdt.restart();
}