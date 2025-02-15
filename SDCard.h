
#include "mbed.h"
#include "MSB_Config.h"

#include "SDBlockDevice.h"
#include "FATFileSystem.h"

class SDCard{
    private:
        SDBlockDevice sd;
        DigitalIn sd_detect;
    public:
        // Constructor
        SDCard(PinName mosi,PinName miso,PinName sclk,PinName cs, PinName detect);
        // Write's test data to a file
        int write_test();
        // Reads data from the test file and prints to terminal
        int read_test();
        // Write data from an array to a file
        int write_file(char* filename, char* text_to_write,bool append = true, bool print_debug = true);
        // Reads the data from a file and prints it to the terminal
        int print_file(char* filename,bool print_debug = true);
        // Reads data from a file and copies it to an array
        int copy_file(char* filename, char* dest, int dest_size, bool print_debug = true);

        bool card_inserted();

};

