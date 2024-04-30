#include <fcntl.h> // filestream
#include <iostream> // cout
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

struct measurement_single_float{
    float angle;
    float dist_0;
    float dist_1;
    float dist_2;
    float dist_3;
    float dist_4;
    float dist_5;
    float dist_6;
    float dist_7;
    float temp_0;
    float temp_1;
    float temp_2;
    float temp_3;
    float temp_4;
    float temp_5;
    float temp_6;
    float temp_7;
    int measurement_id;
};

struct measurement_total_float{
    struct measurement_single_float* measurements;
    int length;
};

// Function declarations
int initialize_spi();
int config_general();
int config_specific();
int start_measurement();
struct measurement_total_float get_data();

