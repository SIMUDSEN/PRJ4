#include <iostream>
#include "spi.h"

using namespace std;

int main() {
    // Your code here
    cout << "Main, calling initialize_spi()..." << endl;
    initialize_spi();
    cout << "Main, calling config_general()..." << endl;
    config_general();
    cout << "Main, calling start_measurement()..." << endl;
    start_measurement();
    cout << "Main, calling get_data()..." << endl;
    struct measurement_total_float data = get_data();
    cout << "Data length: " << data.length << endl;

    for (size_t i = 0; i < data.length; i++)
    {
        cout << endl << endl << endl;
        cout << "Measurement " << i << endl;
        cout << "Angle: " << data.measurements[i].angle << endl;
        cout << "Dist_0: " << data.measurements[i].dist_0 << endl;
        cout << "Dist_1: " << data.measurements[i].dist_1 << endl;
        cout << "Dist_2: " << data.measurements[i].dist_2 << endl;
        cout << "Dist_3: " << data.measurements[i].dist_3 << endl;
        cout << "Dist_4: " << data.measurements[i].dist_4 << endl;
        cout << "Dist_5: " << data.measurements[i].dist_5 << endl;
        cout << "Dist_6: " << data.measurements[i].dist_6 << endl;
        cout << "Dist_7: " << data.measurements[i].dist_7 << endl;
        cout << "Temp_0: " << data.measurements[i].temp_0 << endl;
        cout << "Temp_1: " << data.measurements[i].temp_1 << endl;
        cout << "Temp_2: " << data.measurements[i].temp_2 << endl;
        cout << "Temp_3: " << data.measurements[i].temp_3 << endl;
        cout << "Temp_4: " << data.measurements[i].temp_4 << endl;
        cout << "Temp_5: " << data.measurements[i].temp_5 << endl;
        cout << "Temp_6: " << data.measurements[i].temp_6 << endl;
        cout << "Temp_7: " << data.measurements[i].temp_7 << endl;
    }
    
    
    return 0;
}