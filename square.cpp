#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

//Nom du fichier à généré
string file_name = "square.gcode";

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
* Fonction qui calcule deltaE pour un carré
* L : taille du côté du carré
*/
double compute_deltaE(double L)
{
    //Calcul VE
    double VE = L * tau * nw;
    
    //Calcul deltaE
    double deltaE = VE / (M_PI * (d*d / 4));

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
        exit(-1);
    }

    gcode_file.open(file_name, ios::out | ios::binary);
    if (!gcode_file) {
        cerr << "Impossible d'ouvrir le fichier " << file_name << endl;
        exit(-1);
    }
    gcode_file << params.rdbuf();


    //Début du code pour générer un carré

    //Nombre de couches
    int nb_layers = 25;
    //Largeur côté carré
    float L = 20;
    //Coordonnées de départ
    float X_start = 20.0;
    float Y_start = 20.0;
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
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);

    //Calcul de deltaE (deltaE constant car on a un carré)
    float deltaE = compute_deltaE(L);
    //Initialisation E
    float E = 0;

    //Déplacement au premier point
    gcode_file << "G0 X" << p1.x << " Y" << p1.y << " Z" << Z << " F" << F_G0 << ";" << "\n"; 

    //Pour chaque couche
    for (int n=1; n <= nb_layers; n++) {

        //Pour chaque point du carré
        for (Point p : points) {       
            //Augmente E par deltaE
            E += deltaE;
            //Déplacement G1 à ce point
            gcode_file << "G1 X" << p.x << " Y" << p.y << " E" << E << " F" << F_G1 << "\n";
        }

        //Passage à la couche suivante
        Z += tau;
        gcode_file << "G0 Z" << Z << "\n\n";        
    }

    //Remet la buse à l'origine
    gcode_file << origin() << "\n";
 
    //Ferme les fichiers
    params.close();
    gcode_file.close();

    //Fin
    return 0;
}