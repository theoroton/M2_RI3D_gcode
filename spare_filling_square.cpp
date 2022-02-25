#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <cassert>
#include <map>
#include <vector>
#include <algorithm>
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


/**
 * Méthode qui permet de faire rotater un ensemble de points autour d'un point central
 * selon l'angle donné.
 * points : points à faire rotater
 * center : centre de rotation
 * angle : angle de rotation
 */
vector<Point> rotation_points(vector<Point> points, Point center, int angle) {

    //Ensemble des points rotatés
    vector<Point> points_rotates;

    //Pour chaque point de l'ensemble de point
    for (Point p : points) {
        //Nouveau point rotate
        Point point_rotate;
        //Calcul de la distance au point central
        double xM = p.x - center.x;
        double yM = p.y - center.y;

        //Si l'angle est positif, rotation vers la droite
        if (angle > 0) {
            //Le x rotaté est égal au x du centre + xM * cos(angle) - yM * sin(angle)
            point_rotate.x = center.x + xM * cos(abs(angle) * M_PI / 180) - yM * sin(abs(angle) * M_PI / 180);
            //Le y rotaté est égal au y du centre + xM * sin(angle) + yM * cos(angle)
            point_rotate.y = center.y + xM * sin(abs(angle) * M_PI / 180) + yM * cos(abs(angle) * M_PI / 180);

        //Si l'angle est négatif, rotation vers la gauche (rotation inverse)
        } else {
            //Le x rotaté est égal au x du centre + xM * cos(angle) + yM * sin(angle)
            point_rotate.x = center.x + xM * cos(abs(angle) * M_PI / 180) + yM * sin(abs(angle) * M_PI / 180);
            //Le y rotaté est égal au y du centre - xM * sin(angle) + yM * cos(angle)
            point_rotate.y = center.y - xM * sin(abs(angle) * M_PI / 180) + yM * cos(abs(angle) * M_PI / 180);
        }

        //Ajoute le point rotaté à la
        points_rotates.push_back(point_rotate);
    }

    //Retourne les points rotatés
    return points_rotates;
}


/**
 * Fonction qui permet de faire le découpage en ligne du bas vers le haut d'une forme.
 * Retourne l'ensemble des points (2 à 2) pour faire le découpage
 * points : points de la forme
 * space : espacement entre les lignes
 */
vector<Point> hatching(vector<Point> points, double space)
{

    //Point constituant les lignes calculées
    vector<Point> lignes;

    //Déterminer ymin, ymax des points
    double ymin =   numeric_limits<double>::max();
    double ymax = - numeric_limits<double>::max();
    for (Point p : points) {
        ymin = min(ymin, p.x);
        ymax = max(ymax, p.y);
    }

    //Déterminer hmin et hmax : ligne minimal et ligne maximal
    double hmin = floor(ymin / nw) + 1; //Numéro de la première ligne
    double hmax = floor(ymax / nw); //Numéro de la dernière ligne

    //Nombre de lignes
    int H = hmax - hmin + 1; //Max - min + 1

    //Intersections
    vector<vector<double>> H_intersects;
    //Il y aura un nombre d'intersections au nombre de lignes H (avec 2 points par ligne)
    H_intersects.resize(H);

    //Pour chaque point
    for (int i = 0; i < points.size(); i++) {
        //Récupération du point i et du point n (point i + 1)
        int n = (i+1) % points.size();
        Point pi = points[i];
        Point pn = points[n];

        //Coordoonées des points i et n
        double x_i = pi.x;      
        double y_i = pi.y;
        double x_n = pn.x;
        double y_n = pn.y;

        //Si le point i est au dessus du point n, les swape : on les veut le plus petit possible
        if (y_i > y_n) { 
            swap(y_i, y_n); //Y_n > Y_i
            swap(x_i, x_n); //X_n > X_i
        }

        //Détermination du numéro du segment minimum et maximum
        double seg_hmin = floor(y_i / nw) + 1;
        double seg_hmax = floor(y_n / nw);

        //Pour chaque segmente entre les 2 points
        for (int h = seg_hmin; h <= seg_hmax; h++){
            //Augmente le y de la taille de la buse
            double y_h = h * nw; //Y du point courant

            //Calcul du x de l'intersection
            double x_intersection = x_i + (x_n - x_i) * (y_h - y_i) / (y_n - y_i);

            //Ajout du point d'intersection à la ligne correspondante
            H_intersects[h - hmin].push_back(x_intersection);
        }
    }

    //Pour chaque ligne
    for (int h = 0; h < H_intersects.size(); h = h + (space / nw)) {
        //Trie les points du point gauche au point droit
        sort(H_intersects[h].begin(), H_intersects[h].end());

        //Calcule le segment de sortie
        double y = ymin + (h * nw); //Calcul du y : ymin + hauteur courant

        //Pour les points de la ligne d'intersection
        for (int p=0; p<H_intersects[h].size(); p+=2) {
            //Prendre le point p1 et p2
            Point p1, p2;
            //Récupération des coordonnées du point 1
            double x0 = H_intersects[h][p];
            //Récupération des coordonnées du point 2
            double x1 = H_intersects[h][p+1];

            //Coordonnées du point 1
            p1.x = x0;
            p1.y = y;
            //Coordonnées du point 2
            p2.x = x1;
            p2.y = y; 

            //Ajoute les 2 points aux lignes
            lignes.push_back(p1);
            lignes.push_back(p2);
        }
    }

    //Retourne le découpage en lignes
    return lignes;
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


    //Début du code pour générer un carré

    //Hauteur du cube
    double height = 0.6;
    //Nombre de couches
    int nb_layers = height/tau;
    //Largeur côté carré
    double L = 36;
    //Coordonnées du centre
    Point center;
    center.x = X_center;
    center.y = Y_center;
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

    //Calcul des points pour les lignes à 0°
    vector<Point> rows_angle_0 = hatching(points, spacing);

    //Rotation des points de 45°
    vector<Point> points_angle_45 = rotation_points(points, center, 45);
    //Calcul des points pour les lignes à 45°
    vector<Point> rows_rotates_angle_45 = hatching(points_angle_45, spacing);
    //Rotation des points des lignes à 45° de -45° (pour retourner dans l'état initial)
    vector<Point> rows_angle_45 = rotation_points(rows_rotates_angle_45, center, -45);

    //Rotation des points de -45°
    vector<Point> points_angle_n_45 = rotation_points(points, center, -45);
    //Calcul des points pour les lignes à -45°
    vector<Point> rows_rotates_angle_n_45 = hatching(points_angle_n_45, spacing);
    //Rotation des points des lignes à -45° de 45° (pour retourner dans l'état initial)
    vector<Point> rows_angle_n_45 = rotation_points(rows_rotates_angle_n_45, center, 45);

    //Combinaison des lignes à tracer
    vector<Point> rows;
    rows.insert(rows.end(), rows_angle_0.begin(), rows_angle_0.end());
    rows.insert(rows.end(), rows_angle_45.begin(), rows_angle_45.end());
    rows.insert(rows.end(), rows_angle_n_45.begin(), rows_angle_n_45.end());
    
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

        //Booléen pour inverse le sens et gagner du temps
        bool left_to_right = true;

        //Pour chaque 2 points des lignes d'intersection
        for (int i=0; i < rows.size(); i = i + 2){
            Point p1, p2;

            //Si on va de gauche à droite
            if (left_to_right) {
                //Récupération du premier point de la ligne d'intersection
                p1 = rows[i];
                //Récupération du deuxième point de la ligne d'intersection
                p2 = rows[i + 1];

            } else {
                //Récupération du premier point de la ligne d'intersection
                p1 = rows[i + 1];
                //Récupération du deuxième point de la ligne d'intersection
                p2 = rows[i];
            }

            //Calcul de deltaE pour la ligne d'intersection
            deltaE = compute_deltaE(p1, p2);
            //Augmente E par deltaE
            E += deltaE;

            //Déplacement G0 au premier point
            gcode_file << "G0 X" << p1.x << " Y" << p1.y << '\n';
            //Déplacement G1 au deuxième point
            gcode_file << "G1 X" << p2.x << " Y" << p2.y << " E" << E << '\n';

            //Change le sens
            left_to_right = !left_to_right;
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