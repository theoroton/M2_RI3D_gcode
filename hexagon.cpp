#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <math.h>

using namespace std;

struct Point {
    float x;
    float y;
};

int main () {
    std::ifstream params;
    std::ofstream file; 

    params.open("params.gcode", ios::in | ios::binary);
    file.open("hexagon.gcode", ios::out | ios::binary);

    file << params.rdbuf();

    float X_centre = 20.0;
    float Y_centre = 20.0;
    float Z_start = 0.2;
    float R = 10;
    float height = 5;
    float r = cos(30 * M_PI / 180) * R;
    float op = sin(30 * M_PI / 180) * R;

    vector<Point> points;

    Point p1, p2, p3, p4, p5, p6;
    p1.x = X_centre; p1.y = Y_centre + R;
    points.push_back(p1);
    p2.x = X_centre + r; p2.y = Y_centre + op;
    points.push_back(p2);
    p3.x = X_centre + r; p3.y = Y_centre - op;
    points.push_back(p3);
    p4.x = X_centre; p4.y = Y_centre - R;
    points.push_back(p4);
    p5.x = X_centre - r; p5.y = Y_centre - op;
    points.push_back(p5);
    p6.x = X_centre - r; p6.y = Y_centre + op;
    points.push_back(p6);

    int F_G0 = 3000;
    int F_G1 = 1200;

    float nw = 0.4;
    float tau = 0.2;
    float L = 20.0;
    float d = 1.75;

    float volumeT = nw * tau * L;
    float E = volumeT * 4/ (M_PI * d * d);

    int nb_layers = height/tau;

    file << "G0 X" << p6.x << " Y" << p6.y << " Z" << Z_start << " F" << F_G0 << ";" << std::endl; 
    

    for (int i=1; i <= nb_layers; i++) {
     
        for (int index = 0; index < points.size(); index++) {
            Point p = points.at(index);

            file << "G1 X" << p.x << " Y" << p.y << " E" << E << " F" << F_G1 << ";" << endl;
            file << "G92 E0.0;" << endl;
        }

        file << "G0 Z" << (Z_start + i * tau) << ";" << endl << endl;  
    }

    file << "G0 Z8.0;" << endl;
    file << "G0 X-33 Y-10 Z0;" << endl; 


    params.close();
    file.close();
    return 0;
}