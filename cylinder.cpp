#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

//Nom du fichier à généré
string file_name = "cylinder.gcode";

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
//X du centre
float X_center = 100.0;
//Y du centre
float Y_center = 100.0;

//Structure qui représente un point
struct Point {
    float x; //Coordonnée x du point
    float y; //Coordonnée y du point
};


//Point qui sert de centre pour l'impression
Point center;


/**
* Fonction qui calcule deltaE à partir de 2 points
* p1 : premier point
* p2 : deuxième point
*/
float compute_deltaE(Point p1, Point p2)
{
    //Calcul dx
    float dx = p2.x - p1.x;
    //Calcul dy
    float dy = p2.y - p1.y;

    //Calcul L (distance euclidienne entre les 2 points)
    float L = sqrt(dx*dx + dy*dy);

    //Calcul VE
    float VE = L * tau * nw;
    
    //Calcul deltaE
    float deltaE = VE / (M_PI * (d*d / 4));

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


/**
 * Fonction qui insert un zigzag pour purger le nozzle
 */
string zigzag() {
    ostringstream zigzag;
    Point deb, haut, bas;
    deb.x = 10.0;
    deb.y = 10.0;
    float E = 0.0;
    float deltaE;
    zigzag << "G0 X" << deb.x << " Y" << deb.y << " Z" << tau << " F" << F_G0 << "\n\n";
    //8 fois
    for (int i=1; i <= 8; i++) {
        haut.x = deb.x;         haut.y = deb.y + 10.0;
        bas.x  = deb.x + 10.0;  bas.y  = deb.y;

        //Calcul deltaE
        deltaE = compute_deltaE(deb, haut) * 2;
        //Augmente E par deltaE
        E += deltaE;

        zigzag << "G1 X" << haut.x << " Y" << haut.y << " E" << E << " F" << F_G1 << "\n";

        //Calcul deltaE
        deltaE = compute_deltaE(haut, bas) * 2;
        //Augmente E par deltaE
        E += deltaE;

        zigzag << "G1 X" << bas.x << " Y" << bas.y << " E" << E << " F" << F_G1 << "\n";
        deb.x = bas.x; 
        deb.y = bas.y;
    }
    zigzag << "G92 E0.0\n";

    return zigzag.str();
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

    //Ajout du zigzag
    gcode_file << zigzag() << "\n";


    //Début du code pour générer un cylindre

    //Centre du cercle du cylindre
    center.x = 100.0;
    center.y = 100.0;
    //Hauteur dy cylindre
    float height = 5;
    //Nombre de couches
    int nb_layers = height/tau;
    //Diamètre du cylindre
    float D = 20;
    //Rayon du cylindre
    float r = D/2;
    //Hauteur de départ
    float Z = tau;

    //Point premier, courant et point suivant
    Point current, next;
    //Point courant = premier point du cercle
    current.x = center.x + r;
    current.y = center.y;

    //Initialisation E
    float E = 0;

    //Déplacement au premier point
    gcode_file << "G0 X" << current.x << " Y" << current.y << " Z" << Z << " F" << F_G0 << "\n\n"; 

    //Pas de l'angle
    float angle_step = 360/(D*2);

    //Pour chaque couche
    for (int n=1; n <= nb_layers; n++) {

        //Pour chaque angle du cercle du cylindre
        for (int angle = angle_step; angle <= 360; angle += angle_step) {       
            //Calcul du x du prochain point
            float x = center.x + cos(angle * M_PI / 180) * r;
            //Calcul du y du prochain point
            float y = center.y + sin(angle * M_PI / 180) * r;

            //Prochain point
            next.x = x;
            next.y = y;

            //Calcul deltaE (* 1.4 pour le cylindre)
            float deltaE = compute_deltaE(current, next) * 1.4;
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