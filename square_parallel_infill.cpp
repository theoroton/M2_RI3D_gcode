#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

//Nom du fichier à généré
string file_name = "square_parallel_infill.gcode";

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

    gcode_file.open(file_name.c_str(), ios::out | ios::binary);
    if (!gcode_file) {
        cerr << "Impossible d'ouvrir le fichier " << file_name << endl;
        exit(-1);
    }
    gcode_file << params.rdbuf();

    //Ajout du zigzag
    gcode_file << zigzag() << "\n";


    //Début du code pour générer un carré

    //Hauteur du cube
    double height = 5;
    //Nombre de couches
    int nb_layers = height/tau;
    //Largeur côté carré
    double L = 40;
    //Coordonnées de départ
    Point start;
    start.x = X_center - L/2;
    start.y = Y_center - L/2;
    //Hauteur de départ
    double Z = tau;

    //Liste des points du carré
    vector<Point> points;
    //Création des points du carré
    Point p1, p2, p3, p4;

    //Calcul des points du carré
    p1.x = start.x;     p1.y = start.y;
    p2.x = start.x + L; p2.y = start.y;
    p3.x = start.x + L; p3.y = start.y + L;
    p4.x = start.x;     p4.y = start.y + L;

    //Ajout des points à la liste des points (p1 à la fin pour retourner dessus)
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p1);
  
    //Initialisation deltaE et E
    double deltaE, E = 0;

    //Pour chaque couche
    for (int n=1; n <= nb_layers; n++) {

        //Calcul de deltaE pour les côtés du carré
        deltaE = compute_deltaE(p1, p2);

        //Déplacement au premier point
        gcode_file << "G0 X" << p1.x << " Y" << p1.y << " Z" << Z << " F" << F_G0 << ";" << "\n\n"; 
        
        //Pour chaque point du carré
        for (int i = 0; i < points.size(); i++) {
            Point p_ext = points[i];       
            //Augmente E par deltaE
            E += deltaE;
            //Déplacement G1 à ce point
            gcode_file << "G1 X" << p_ext.x << " Y" << p_ext.y << " E" << E << " F" << F_G1 << "\n";
        }

        //Création des points du carré intérieur
        Point p1_i, p2_i, p3_i, p4_i;
        //Calcul des points du carré intérieur (au début, même que le carré normal)
        p1_i.x = p1.x;      p1_i.y = p1.y;
        p2_i.x = p2.x;      p2_i.y = p2.y;
        p3_i.x = p3.x;      p3_i.y = p3.y;
        p4_i.x = p4.x;      p4_i.y = p4.y;

        //-----------Parallel-----------

        //Tant que ne converge pas vers le centre
        while (p2_i.x - p1_i.x > tau) {

            //Liste des points du carré intérieur
            vector<Point> points_interieur;

            //Calcul des points du carré intérieur
            p1_i.x = p1_i.x + tau;      p1_i.y = p1_i.y + tau;
            p2_i.x = p2_i.x - tau;      p2_i.y = p2_i.y + tau;
            p3_i.x = p3_i.x - tau;      p3_i.y = p3_i.y - tau;
            p4_i.x = p4_i.x + tau;      p4_i.y = p4_i.y - tau;

            //Ajout des points à la liste des points (p1 à la fin pour retourner dessus)
            points_interieur.push_back(p2_i);
            points_interieur.push_back(p3_i);
            points_interieur.push_back(p4_i);
            points_interieur.push_back(p1_i);

            //Déplacement au premier point intérieur
            gcode_file << "G0 X" << p1_i.x << " Y" << p1_i.y << " Z" << Z << " F" << F_G0 << "\n";
                
            //Calcul de deltaE pour les côtés du carré intérieur
            deltaE = compute_deltaE(p1_i, p2_i);
            
            //Pour chaque point du carré intérieur
            for (int i = 0; i < points_interieur.size(); i++) {
                Point p_int = points_interieur[i];       
                //Augmente E par deltaE
                E += deltaE;
                //Déplacement G1 à ce point
                gcode_file << "G1 X" << p_int.x << " Y" << p_int.y << " E" << E << " F" << F_G1 << "\n";
            }
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