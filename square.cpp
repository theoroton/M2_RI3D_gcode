#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

int main () {
    std::ifstream params;
    std::ofstream file; 

    params.open("params.gcode", ios::in | ios::binary);
    file.open("square.gcode", ios::out | ios::binary);

    file << params.rdbuf();

    float X_start = 20.0;
    float Y_start = 20.0;
    float Z_start = 0.2;

    int F_G0 = 3000;
    int F_G1 = 1200;

    float nw = 0.4;
    float tau = 0.2;
    float L = 20.0;
    float d = 1.75;

    float volumeT = nw * tau * L;
    float E = volumeT * 4/ (M_PI * d * d);

    file << "G0 X" << X_start << " Y" << Y_start << " Z" << Z_start << " F" << F_G0 << ";" << std::endl; 
    
    int nb_layers = 25;

    for (int i=1; i <= 25; i++) {

        file << "G1 X" << (X_start + L) << " E" << E << " F" << F_G1 << ";" << endl;
        file << "G92 E0.0;" << endl;

        file << "G1 Y" << (Y_start + L) << " E" << E << " F" << F_G1 << ";" << endl;
        file << "G92 E0.0;" << endl;

        file << "G1 X" << X_start << " E" << E << " F" << F_G1 << ";" << endl;
        file << "G92 E0.0;" << endl;

        file << "G1 Y" << Y_start << " E" << E << " F" << F_G1 << ";" << endl;
        file << "G92 E0.0;" << endl;

        file << "G0 Z" << (Z_start + i * tau) << ";" << endl << endl;        
    }

    file << "G0 Z8.0;" << endl;
    file << "G0 X-33 Y-10 Z0;" << endl; 


    params.close();
    file.close();
    return 0;
}