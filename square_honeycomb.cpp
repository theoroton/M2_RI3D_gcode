#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

using namespace std;

//Nom du fichier à généré
string file_name = "square_honeycomb.gcode";

//Paramètres
//Diamètre de la buse
float nw = 0.4;
//Epaisseur d'une couche
float tau = 0.2;
//Diamètre du filament
float d = 1.75;
//Vitesse de déplacement G0
int F_G0 = 3000;
//Vitesse de déplacement G1
int F_G1 = 1200;

//Structure qui représente un point
struct Point {
    float x; //Coordonnée x du point
    float y; //Coordonnée y du point
};

/**
* Fonction qui calcule deltaE pour un hexagone à partir des 2 points d'un de ses côtés
* p1 : premier point d'un côté de l'hexagone
* p2 : deuxième point d'un côté de l'hexagone
*/
double compute_deltaE_hexa(Point p1, Point p2)
{
    //Calcul dx
    double dx = p2.x - p1.x;
    //Calcul dy
    double dy = p2.y - p1.y;

    //Calcul L (distance euclidienne entre les 2 points)
    double L = sqrt(dx*dx + dy*dy);

    //Calcul VE
    double VE = L * tau * nw;
    
    //Calcul deltaE
    double deltaE = VE / (M_PI * (d*d / 4));

    //Retourne deltaE
    return deltaE;
}


/**
* Fonction qui calcule deltaE pour un carré
* L : taille du côté du carré
*/
double compute_deltaE(double L)
{
    //Calcul VE
    double VE = L * tau * nw;
    
    //Calcul deltaE
    double deltaE = VE / (M_PI * (d*d / 4));//VE*4/(M_PI*d*d);//

    //Retourne deltaE
    return deltaE;
}


/**
* Fonction qui renvoie le gcode pour remettre la buse à l'origine
*/
string origin() {
    string origin = "G0 X-33 Y-10 Z20\n";
    origin.append("G0 Z0\n");
    
    return  origin; 
}

//Dessine un hexagone centré en X_center, Y_center
string draw_hexagone(int X_center, int Y_center, float Z){
    std::stringstream ss;

    //Radius hexagone
    float R = 3;

    //Calcul du rayon du cercle inscrit r
    double r = cos(30 * M_PI / 180) * R;
    //Calcul de la hauteur entre le centre et un point de l'hexagone n'étant pas sur la même abscisse
    double h = sin(30 * M_PI / 180) * R;

    //Liste des points de l'hexagone
    vector<Point> points;
    //Création des points de l'hexagone
    Point p1, p2, p3, p4, p5, p6;

    //Calcul des points de l'hexagone
    p1.x = X_center;        p1.y = Y_center + R;
    p2.x = X_center + r;    p2.y = Y_center + h;
    p3.x = X_center + r;    p3.y = Y_center - h;
    p4.x = X_center;        p4.y = Y_center - R;
    p5.x = X_center - r;    p5.y = Y_center - h;
    p6.x = X_center - r;    p6.y = Y_center + h;

    //Ajout des points à la liste des points (p1 à la fin pour retourner dessus)
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    points.push_back(p6);
    points.push_back(p1);
    points.push_back(p2);

    double deltaE = compute_deltaE_hexa(p1, p2) * 1.2;
    double E = 0;

    //Déplacement au premier point
    ss << "G0 X" << p1.x << " Y" << p1.y << " Z" << Z << " F" << F_G0 << ";" << "\n"; 

    //Pour chaque point de l'hexagone
        for (int i = 0; i < points.size(); i++) {
            Point p = points[i];       
            //Augmente E par deltaE
            E += deltaE;
            //Déplacement G1 à ce point
            ss << "G1 X" << p.x << " Y" << p.y << " E" << E <<" Z" << Z <<  " F" << F_G1 << "\n";
        }

    return ss.str();
}

string draw_line_hexagone(int X_center, int Y_center,float L, float Z){
    int R = 3;
    int x = X_center;
    int y = Y_center;

    string code = draw_hexagone(x, y, Z);

    for(int i=0; i<L/(R*2)+1; i++){                                       //Lignes
        for(int j=0; j<L/(R*2); j++){                                    //Colonnes
            x = X_center + j*R*2;
            code = code + draw_hexagone(x, y, Z);
        }
        x = X_center;
        y = Y_center + i*R*2;
    }
    return code;
}


//Main
int main () {
    //Fichier des paramètres
    std::ifstream params;
    //Fichier gcode généré
    std::ofstream gcode_file; 

    //Ouverture des fichiers
    params.open("params.gcode", ios::in | ios::binary);
    if (!params) {
        cerr << "Impossible d'ouvrir le fichier params.gcode" << endl;
        return 0;
    }

    gcode_file.open(file_name.c_str(), ios::out | ios::binary);
    if (!gcode_file) {
        cerr << "Impossible d'ouvrir le fichier " << file_name << endl;
        return 0;
    }
    gcode_file << params.rdbuf();


    //Début du code pour générer un carré

    int height = 36;
    //Nombre de couches
    int nb_layers = height/tau;
    //Largeur côté carré
    float L = height;
    //Coordonnées de départ
    float X_start = 80.0;
    float Y_start = 50.0;
    //Hauteur de départ
    float Z = tau;

    //Liste des points du carré
    vector<Point> points;
    //Création des points du carré
    Point p1, p2, p3, p4;

    //Calcul des points du carré
    p1.x = X_start;     p1.y = Y_start;
    p2.x = X_start + L; p2.y = Y_start;
    p3.x = X_start + L; p3.y = Y_start + L;
    p4.x = X_start;     p4.y = Y_start + L;

    //Ajout des points à la liste des points
    points.push_back(p1);
    points.push_back(p4);
    points.push_back(p3);
    points.push_back(p2);
    points.push_back(p1);
    //points.push_back(p1);

    //Calcul de deltaE (deltaE constant car on a un carré)
    float deltaE = 1.8*compute_deltaE(L);
    float deltaEL = 1.8*compute_deltaE(2*tau);
    float deltaEH = 1.8*compute_deltaE(L-2*tau);
    //float deltaEstart = compute_deltaE(L);
    //float deltaEH = compute_deltaE(L);
    //float deltaEL = compute_deltaE(L);
    //Initialisation E
    float E = 0;

    //Déplacement au premier point
    int xC = 40;
    int yC = 10;
    gcode_file << "G0 X"<<xC<<" Y"<<yC<<" Z" << Z << " F" << F_G0 << ";" << "\n"; 
    
    for (int i = 1; i < 10; i++) {
        if(i%2 != 0) {
            yC += 10;
        } else {
            xC += 10;
            yC -= 10;
        }
        E += deltaE;
        gcode_file << "G1 X" << xC << " Y" << yC << " E" << E << " F" << F_G1 << "\n";
    }
    
    int count = 0;
    //nb_layers = 1;
    while(count < nb_layers) {
        if(count != 0){
            gcode_file << "G0 X"<<p1.x<<" Y"<<p1.y<<" Z" << Z << " F" << F_G0 << ";" << "\n";
            Z-=tau;
        }
        //Pour chaque point du carré
        for (int i = 0; i < points.size(); i++) {
            Point p = points[i];       
            //Augmente E par deltaE
            E += deltaE;
            //Déplacement G1 à ce point
            gcode_file << "G1 X" << p.x << " Y" << p.y << " E" << E << " F" << F_G1 << "\n";
    
        }

        //Dessin hexagone
        int x_centre = X_start + 3;
        int y_centre = Y_start + 3;
        gcode_file << "G0 X" << x_centre << " Y" << y_centre << " Z" << Z << " F" << F_G0 << ";" << "\n";
        string code = draw_line_hexagone(x_centre, y_centre, L, Z);
        gcode_file << code;
        Z+=2*tau;
        count++;
        E -= deltaE; 
    }

    //Remet la buse à l'origine
    gcode_file << origin() << "\n";
 
    //Ferme les fichiers
    params.close();
    gcode_file.close();

    //Fin
    return 0;
}