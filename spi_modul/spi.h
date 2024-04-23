#include <fcntl.h> // filestream
#include <iostream> // cout
#include <stdlib.h>
#include <unistd.h>

// Structs
struct measurement_single{
    char angle[2];
    char dist_0[2];
    char dist_1[2];
    char dist_2[2];
    char dist_3[2];
    char dist_4[2];
    char dist_5[2];
    char dist_6[2];
    char dist_7[2];
    char temp_0[2];
    char temp_1[2];
    char temp_2[2];
    char temp_3[2];
    char temp_4[2];
    char temp_5[2];
    char temp_6[2];
    char temp_7[2];
    char measurement_id[2];
};

struct measurement_total{
    struct measurement_single* measurements;
    int length;
};


// Function declarations
int initialize_spi();
int config_general();
int config_specific();
int start_measurement();
struct measurement_total get_data();

