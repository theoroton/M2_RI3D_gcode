#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

//Nom du fichier à généré
string file_name = "cylinder.gcode";

//Paramètres
//Diamètre de la buse
double nw = 0.4;
//Epaisseur d'une couche
double tau = 0.2;
//Diamètre du filament
double d = 1.75;
//Vitesse de déplacement G0
int F_G0 = 3000;
//Vitesse de déplacement G1
int F_G1 = 1200;

//Structure qui représente un point
struct Point {
    double x; //Coordonnée x du point
    double y; //Coordonnée y du point
};


/**
* Fonction qui calcule deltaE à partir de 2 points
* p1 : premier point
* p2 : deuxième point
*/
double compute_deltaE(Point p1, Point p2)
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
* Fonction qui renvoie le gcode pour remettre la buse à l'origine
*/
string origin() {
    return  "G0 Z8.0\nG0 X-33 Y-10 Z0\n"; 
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


    //Début du code pour générer un cylindre

    //Hauteur dy cylindre
    double height = 10;
    //Nombre de couches
    int nb_layers = height/tau;
    //Diamètre du cylindre
    double D = 8;
    //Rayon du cylindre
    double r = D/2;
    //Coordonnées du centre du cylindre
    double X_center = 20.0;
    double Y_center = 20.0;
    //Hauteur de départ
    double Z = tau;

    //Point courant et point suivant
    Point current, next;
    //Point courant = premier point du cercle
    current.x = X_center + r;
    current.y = Y_center;

    //Initialisation E
    double E = 0;

    //Déplacement au premier point
    gcode_file << "G0 X" << current.x << " Y" << current.y << " Z" << Z << " F" << F_G0 << ";" << "\n"; 

    //Pas de l'angle : tous les 10 degrés
    float angle_step = 10;

    //Pour chaque couche
    for (int n=1; n <= nb_layers; n++) {

        //Pour chaque angle du cercle du cylindre
        for (int angle = 0; angle < 360; angle += angle_step) {       
            //Calcul du x du prochain point
            float x = X_center + cos(angle * M_PI / 180) * r;
            //Calcul du y du prochain point
            float y = Y_center + sin(angle * M_PI / 180) * r;

            //Prochain point
            next.x = x;
            next.y = y;

            //Calcul deltaE
            float deltaE = compute_deltaE(current, next);
            //Augmente E par deltaE
            E += deltaE;

            //Déplacement G1 au point suivant
            gcode_file << "G1 X" << next.x << " Y" << next.y << " E" << E << " F" << F_G1 << "\n";

            //Mise à jour point courrant
            current = next;
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