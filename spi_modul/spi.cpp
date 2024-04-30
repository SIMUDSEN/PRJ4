#include "spi.h"

using namespace std;

#define MODULE_DEBUG 0

// Diverse globale variable
int file_descriptor;
const char *file_path = "/dev/spi_drv0";
char header;
char command;
char sendData[2];
char id[2];

// Enumerations and definitions
enum header{
    HEADER_CONFIG_GENERAL = 0x00,
    HEADER_CONFIG_SPECIFIC = 0x01,
    HEADER_START_MEASUREMENT = 0x02,
    HEADER_GET_DATA = 0x03,
    HEADER_GET_DATA_SIZE = 0x04
};

enum command{
    COMMAND_CONFIG_GENERAL = 0x00,
    COMMAND_CONFIG_SPECIFIC = 0x01,
    COMMAND_START_MEASUREMENT = 0x02,
    COMMAND_DATATYPE_ANGLE = 0x03,
    COMMAND_DATATYPE_DIST_0 = 0x04,
    COMMAND_DATATYPE_DIST_1 = 0x05,
    COMMAND_DATATYPE_DIST_2 = 0x06,
    COMMAND_DATATYPE_DIST_3 = 0x07,
    COMMAND_DATATYPE_DIST_4 = 0x08,
    COMMAND_DATATYPE_DIST_5 = 0x09,
    COMMAND_DATATYPE_DIST_6 = 0x0A,
    COMMAND_DATATYPE_DIST_7 = 0x0B,
    COMMAND_DATATYPE_TEMP_0 = 0x0C,
    COMMAND_DATATYPE_TEMP_1 = 0x0D,
    COMMAND_DATATYPE_TEMP_2 = 0x0E,
    COMMAND_DATATYPE_TEMP_3 = 0x0F,
    COMMAND_DATATYPE_TEMP_4 = 0x10,
    COMMAND_DATATYPE_TEMP_5 = 0x11,
    COMMAND_DATATYPE_TEMP_6 = 0x12,
    COMMAND_DATATYPE_TEMP_7 = 0x13,
    COMMAND_CONFIRM_COMPLETED = 0x14,
    COMMAND_GET_DATA_SIZE = 0x15,
};

enum confirmData{
    DATA_CONFIRM_FINISHED = 0x01,
    DATA_CONFIRM_RECEIVED = 0x02
};

#define POLLING_FREQUENCY 100
#define POLLING_DELAY_US 1000000/POLLING_FREQUENCY
#define REQUIRE_CONFIRM 0

// Private functions
void write_to_spi(int requireConfirm);
void get_single_data(enum command dataType, char* dataBuffer);
void dataChar_to_dataFloat(struct measurement_total* dataChar, struct measurement_total_float* dataFloat);

// Function definitions

int initialize_spi(){
    file_descriptor = open(file_path, O_RDWR);
    return file_descriptor;
}

void write_to_spi(int requireConfirm = 0){
    char sendCode[2] = {header, command};
    char receiveString[256];
    char tempData[4];

    do
    {
        write(file_descriptor, &sendCode, sizeof(sendCode));
        read(file_descriptor, receiveString, sizeof(receiveString));

        if (MODULE_DEBUG)
            cout << "Received string: " << receiveString << endl;

        for (size_t i = 0; i < 4; i++){
            char intBuf[3] = {receiveString[i*3], receiveString[i*3+1], receiveString[i*3+2]};
            tempData[i] = (char)(atoi(intBuf));
        }

        if (MODULE_DEBUG)
            cout << "Received chars: " << (int)tempData[0] << " " << (int)tempData[1] << " " << (int)tempData[2] << " " << (int)tempData[3] << endl;

    } while ((tempData[1] != DATA_CONFIRM_RECEIVED) & (requireConfirm == 1));

    sendData[0] = tempData[0];
    sendData[1] = tempData[1];
    id[0] = tempData[2];
    id[1] = tempData[3];
}

void get_single_data(enum command dataType, char* dataBuffer){
    command = dataType;
    write_to_spi();
    dataBuffer[0] = sendData[0];
    dataBuffer[1] = sendData[1];
}

int config_general(){
    // Send startkommando
    header = HEADER_CONFIG_GENERAL;
    command = COMMAND_CONFIG_GENERAL;
    write_to_spi();

    // Kommando til at bekræfte at konfigurationen er gennemført
    command = COMMAND_CONFIRM_COMPLETED;

    // Kontinuerlig polling, for at spørge om konfigurationen er gennemført
    while (1)
    {
        write_to_spi();
        if (sendData[1] == DATA_CONFIRM_FINISHED)
            return 1;
        usleep(POLLING_DELAY_US);
    }
    return 0;
}

int config_specific(){
    // Send startkommando
    header = HEADER_CONFIG_SPECIFIC;
    command = COMMAND_CONFIG_SPECIFIC;
    write_to_spi();

    // Kommando til at bekræfte at konfigurationen er gennemført
    command = COMMAND_CONFIRM_COMPLETED;

    // Kontinuerlig polling, for at spørge om konfigurationen er gennemført
    while (1)
    {
        write_to_spi();
        if (sendData[0] == DATA_CONFIRM_FINISHED)
            return 1;
        usleep(POLLING_DELAY_US);
    }
    return 0;
}

int start_measurement(){
    // Send startkommando
    header = HEADER_START_MEASUREMENT;
    command = COMMAND_START_MEASUREMENT;
    write_to_spi(REQUIRE_CONFIRM);

    // Kommando til at bekræfte at målingen er gennemført
    command = COMMAND_CONFIRM_COMPLETED;

    // Kontinuerlig polling, for at spørge om målingen er gennemført
    while (1)
    {
        write_to_spi();
        if (sendData[1] == DATA_CONFIRM_FINISHED)
            return 1;
        usleep(POLLING_DELAY_US);
    }
    return 0;
}

void dataChar_to_dataFloat(struct measurement_total* dataChar, struct measurement_total_float* dataFloat){
    dataFloat->length = dataChar->length;
    dataFloat->measurements = new measurement_single_float[dataChar->length];

    for (size_t i = 0; i < dataChar->length; i++){
        dataFloat->measurements[i].angle = (float)((int)(dataChar->measurements[i].angle[0] << 8 |dataChar->measurements[i].angle[1]))/10;
        dataFloat->measurements[i].dist_0 = (float)((int)(dataChar->measurements[i].dist_0[0] << 8 |dataChar->measurements[i].dist_0[1]))/100;
        dataFloat->measurements[i].dist_1 = (float)((int)(dataChar->measurements[i].dist_1[0] << 8 |dataChar->measurements[i].dist_1[1]))/100;
        dataFloat->measurements[i].dist_2 = (float)((int)(dataChar->measurements[i].dist_2[0] << 8 |dataChar->measurements[i].dist_2[1]))/100;
        dataFloat->measurements[i].dist_3 = (float)((int)(dataChar->measurements[i].dist_3[0] << 8 |dataChar->measurements[i].dist_3[1]))/100;
        dataFloat->measurements[i].dist_4 = (float)((int)(dataChar->measurements[i].dist_4[0] << 8 |dataChar->measurements[i].dist_4[1]))/100;
        dataFloat->measurements[i].dist_5 = (float)((int)(dataChar->measurements[i].dist_5[0] << 8 |dataChar->measurements[i].dist_5[1]))/100;
        dataFloat->measurements[i].dist_6 = (float)((int)(dataChar->measurements[i].dist_6[0] << 8 |dataChar->measurements[i].dist_6[1]))/100;
        dataFloat->measurements[i].dist_7 = (float)((int)(dataChar->measurements[i].dist_7[0] << 8 |dataChar->measurements[i].dist_7[1]))/100;
        dataFloat->measurements[i].temp_0 = (float)((int)(dataChar->measurements[i].temp_0[0] << 8 |dataChar->measurements[i].temp_0[1]))/10;
        dataFloat->measurements[i].temp_1 = (float)((int)(dataChar->measurements[i].temp_1[0] << 8 |dataChar->measurements[i].temp_1[1]))/10;
        dataFloat->measurements[i].temp_2 = (float)((int)(dataChar->measurements[i].temp_2[0] << 8 |dataChar->measurements[i].temp_2[1]))/10;
        dataFloat->measurements[i].temp_3 = (float)((int)(dataChar->measurements[i].temp_3[0] << 8 |dataChar->measurements[i].temp_3[1]))/10;
        dataFloat->measurements[i].temp_4 = (float)((int)(dataChar->measurements[i].temp_4[0] << 8 |dataChar->measurements[i].temp_4[1]))/10;
        dataFloat->measurements[i].temp_5 = (float)((int)(dataChar->measurements[i].temp_5[0] << 8 |dataChar->measurements[i].temp_5[1]))/10;
        dataFloat->measurements[i].temp_6 = (float)((int)(dataChar->measurements[i].temp_6[0] << 8 |dataChar->measurements[i].temp_6[1]))/10;
        dataFloat->measurements[i].temp_7 = (float)((int)(dataChar->measurements[i].temp_7[0] << 8 |dataChar->measurements[i].temp_7[1]))/10;
    }
}

struct measurement_total_float get_data(){
    int dataSize;

    // Først skal vi finde ud af hvor meget data vi skal hente
    header = HEADER_GET_DATA_SIZE;
    command = COMMAND_GET_DATA_SIZE;
    write_to_spi();
    dataSize = sendData[0] << 8 | sendData[1];
    cout << "Data size: " << dataSize << endl;

    // Så skal vi hente dataen
    struct measurement_total measurements;
    measurements.length = dataSize;
    measurements.measurements = new measurement_single[dataSize];

    struct measurement_total_float measurements_float;

    for (size_t i = 0; i < dataSize; i++)
    {
        struct measurement_single measurement;
        header = HEADER_GET_DATA;
        
        // Modtag samtlige datatyper
        get_single_data(COMMAND_DATATYPE_ANGLE, measurement.angle);
        get_single_data(COMMAND_DATATYPE_DIST_0, measurement.dist_0);
        get_single_data(COMMAND_DATATYPE_DIST_1, measurement.dist_1);
        get_single_data(COMMAND_DATATYPE_DIST_2, measurement.dist_2);
        get_single_data(COMMAND_DATATYPE_DIST_3, measurement.dist_3);
        get_single_data(COMMAND_DATATYPE_DIST_4, measurement.dist_4);
        get_single_data(COMMAND_DATATYPE_DIST_5, measurement.dist_5);
        get_single_data(COMMAND_DATATYPE_DIST_6, measurement.dist_6);
        get_single_data(COMMAND_DATATYPE_DIST_7, measurement.dist_7);
        get_single_data(COMMAND_DATATYPE_TEMP_0, measurement.temp_0);
        get_single_data(COMMAND_DATATYPE_TEMP_1, measurement.temp_1);
        get_single_data(COMMAND_DATATYPE_TEMP_2, measurement.temp_2);
        get_single_data(COMMAND_DATATYPE_TEMP_3, measurement.temp_3);
        get_single_data(COMMAND_DATATYPE_TEMP_4, measurement.temp_4);
        get_single_data(COMMAND_DATATYPE_TEMP_5, measurement.temp_5);
        get_single_data(COMMAND_DATATYPE_TEMP_6, measurement.temp_6);
        get_single_data(COMMAND_DATATYPE_TEMP_7, measurement.temp_7);

        measurement.measurement_id[0] = id[0];
        measurement.measurement_id[1] = id[1];

        //Indsæt datapunkt i totale measurement
        measurements.measurements[i] = measurement;
    }

    dataChar_to_dataFloat(&measurements, &measurements_float);

    delete(measurements.measurements);

    return measurements_float;
}
