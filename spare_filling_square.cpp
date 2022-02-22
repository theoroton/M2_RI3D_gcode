#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

//Nom du fichier à généré
string file_name = "spare_filling_square.gcode";

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
//Spacing donné
double spacing = 4;

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
    return deltaE*2.5;
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
        deltaE = compute_deltaE(deb, haut);
        //Augmente E par deltaE
        E += deltaE;

        zigzag << "G1 X" << haut.x << " Y" << haut.y << " E" << E << " F" << F_G1 << "\n";

        //Calcul deltaE
        deltaE = compute_deltaE(haut, bas);
        //Augmente E par deltaE
        E += deltaE;

        zigzag << "G1 X" << bas.x << " Y" << bas.y << " E" << E << " F" << F_G1 << "\n";
        deb.x = bas.x; 
        deb.y = bas.y;
    }
    zigzag << "G92 E0.0\n";

    return zigzag.str();
}


/**
 * Fonction qui permet de faire un set de lignes d'un point de début
 * à un point de fin avec l'angle donné
 */
string set_of_lines(Point start, Point end, double angle, double L, double &E) {
    //Set de lignes
    ostringstream set_of_lines;

    //Point courant
    Point curr;
    //Point courant initialisé au point de start
    curr.x = start.x; curr.y = start.y;

    //Spacing total initial
    double spacing_total = spacing;

    cout << "Debut : " << start.x << " - " << start.y << endl;
    cout << "Fin : " << end.x << " - " << end.y << endl;

    //Tant qu'on le spacing totale ne dépasse pas la largeur du carré
    while (spacing_total <= L - spacing) {
        //Points du segment courant
        Point p1, p2;

        //Si l'angle est positif, on va du point en haut à gauche au point en bas à droite
        if (angle > 0) {       
            //Coordonnées du point courant
            curr.x = curr.x + spacing;
            curr.y = curr.y - spacing;

            //Distance calculée
            double dist, dx, dy, d;

            //Première moitié
            if (spacing_total <= L/2) {
                //Calcul par rapport au début
                dx = abs(curr.x - start.x);
                dy = abs(curr.y - start.y);
            //Deuxième moitié
            } else {
                //Calcul par rapport à la fin
                dx = abs(curr.x - end.x);
                dy = abs(curr.y - end.y);
            }

            //Calcul de la distance aux points p1 et p2
            dist = sqrt(dx * dx + dy * dy);
            d = dist * sin(45 * M_PI / 180) * 2;

            //Première moitié : moitié supérieure gauche
            if (spacing_total <= L/2) {
                //Calcul des coordoonées des points p1 et p2
                p1.x = start.x;         p1.y = start.y - d;
                p2.x = start.x + d;     p2.y = start.y;
            //Deuxième moitié : moitié inférieure droite
            } else {
                //Calcul des coordoonées des points p1 et p2
                p1.x = end.x - d;       p1.y = end.y;
                p2.x = end.x;           p2.y = end.y + d;
            }

        //Si l'angle est négatif, on va du point en bas à gauche au point en haut à droite 
        } else if (angle < 0){
            curr.x = curr.x + spacing;
            curr.y = curr.y + spacing;

            //Distance calculée
            double dist, dx, dy, d;

            //Première moitié
            if (spacing_total <= L/2) {
                //Calcul par rapport au début
                dx = abs(curr.x - start.x);
                dy = abs(curr.y - start.y);
            //Deuxième moitié
            } else {
                //Calcul par rapport à la fin
                dx = abs(curr.x - end.x);
                dy = abs(curr.y - end.y);
            }

            //Calcul de la distance aux points p1 et p2
            dist = sqrt(dx * dx + dy * dy);
            d = dist * sin(45 * M_PI / 180) * 2;

            //Première moitié : moitié supérieure gauche
            if (spacing_total <= L/2) {
                //Calcul des coordoonées des points p1 et p2
                p1.x = start.x;         p1.y = start.y + d;
                p2.x = start.x + d;     p2.y = start.y;
            //Deuxième moitié : moitié inférieure droite
            } else {
                //Calcul des coordoonées des points p1 et p2
                p1.x = end.x - d;       p1.y = end.y;
                p2.x = end.x;           p2.y = end.y - d;
            }
        
        //Si l'angle est nul
        } else {
            curr.x = curr.x;
            curr.y = curr.y - spacing;

            //Calcul des coordonnées des points p1 et p2
            p1.x = curr.x;         p1.y = curr.y;
            p2.x = curr.x + L;     p2.y = curr.y;
        }

        //Augmentation du spacing total
        spacing_total += spacing;

        //Calcul de deltaE pour les côtés du carré
        double deltaE = compute_deltaE(p1, p2);
        //Augmente E par deltaE
        E += deltaE;

        cout << "Courant : " << curr.x << " - " << curr.y << endl;
        cout << "P1 : " << p1.x << " - " << p1.y << endl;
        cout << "P2 : " << p2.x << " - " << p2.y << endl << endl;

        //Déplacement G0 au point P1
        set_of_lines << "G0 X" << p1.x << " Y" << p1.y << " F" << F_G0 << ";" << "\n";
        //Déplacement G1 au point P2
        set_of_lines << "G1 X" << p2.x << " Y" << p2.y << " E" << E << " F" << F_G1 << "\n";
    }
    set_of_lines << "\n";

    return set_of_lines.str();
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
            Point p = points[i];       
            //Augmente E par deltaE
            E += deltaE;
            //Déplacement G1 à ce point
            gcode_file << "G1 X" << p.x << " Y" << p.y << " E" << E << " F" << F_G1 << "\n";
        }
        gcode_file << "\n";

        //Premier set avec un angle de 45°
        gcode_file << set_of_lines(p4, p2, 45, L, E);
        //Deuxième set avec un angle de -45°
        gcode_file << set_of_lines(p1, p3, -45, L, E);
        //Troisième set avec un angle de 0°
        gcode_file << set_of_lines(p4, p1, 0, L, E);

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