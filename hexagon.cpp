#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

//Nom du fichier à généré
string file_name = "hexagon.gcode";

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
//X du centre
double X_center = 100.0;
//Y du centre
double Y_center = 100.0;

//Structure qui représente un point
struct Point {
    double x; //Coordonnée x du point
    double y; //Coordonnée y du point
};


/**
* Fonction qui calcule deltaE pour un hexagone à partir des 2 points d'un de ces côtés
* p1 : premier point d'un côté de l'hexagone
* p2 : deuxième point d'un côté de l'hexagone
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
    //Gcode pour retourner à l'origine
    string origin = "G0 X-33 Y-10 Z20\n";
    origin.append("G0 Z0\n");

    //Retourne le retour à l'origine
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
    double E = 0.0;
    double deltaE;
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


    //Début du code pour générer un hexagone

    //Hauteur de l'hexagone
    double height = 15;
    //Nombre de couches
    int nb_layers = height/tau;
    //Rayon du cercle circonscrit R
    double R = 10;
    //Hauteur de départ
    double Z = tau;  
    //Point qui sert de centre pour l'impression
    Point center;

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
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    points.push_back(p6);
    points.push_back(p1);

    //Calcul de deltaE (deltaE constant car tous les côtés sont de même longueur dans un hexagone) (* 1.2 pour l'hexagone)
    double deltaE = compute_deltaE(p1, p2) * 1.2;
    //Initialisation E
    double E = 0;

    //Déplacement au premier point
    gcode_file << "G0 X" << p1.x << " Y" << p1.y << " Z" << Z << " F" << F_G0 << ";" << "\n\n"; 

    //Pour chaque couche
    for (int n=1; n <= nb_layers; n++) {

        //Pour chaque point de l'hexagone
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