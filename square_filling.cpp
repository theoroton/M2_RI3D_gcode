#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

//Nom du fichier à généré
string file_name = "square_filling.gcode";

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
        //exit(-1);
    }

    gcode_file.open(file_name.c_str(), ios::out | ios::binary);
    if (!gcode_file) {
        cerr << "Impossible d'ouvrir le fichier " << file_name << endl;
        return 0;
        //exit(-1);
    }
    gcode_file << params.rdbuf();


    //Début du code pour générer un carré

    //Nombre de couches
    int nb_layers = 25;
    //Largeur côté carré
    float L = 40;
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
    float deltaE = compute_deltaE(L);
    //Initialisation E
    float E = 0;//0;

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
    deltaE = 2 *compute_deltaE(L);
    E += deltaE;
    gcode_file << "G1 X" << xC << " Y" << yC << " E" << E << " F" << F_G1 << "\n";
    
    //Pour chaque point du carré
    for (int i = 0; i < points.size(); i++) {
        Point p = points[i];       
        //Augmente E par deltaE
        E += deltaE;
        //Déplacement G1 à ce point
        gcode_file << "G1 X" << p.x << " Y" << p.y << " E" << E << " F" << F_G1 << "\n";
 
    }

    //-----------Zigzag-----------

    //Calcul des points du carré
    //p1.x = X_start;     p1.y = X_start;
    p4.x =  X_start + L-tau;     p4.y = X_start + L+tau;

    Point curr = {p1.x+2*tau, p1.y+2*tau};
    //Déplacement au premier point interieur
    gcode_file << "G0 X" << curr.x << " Y" << curr.y << " Z" << Z << " F" << F_G0 << ";" << "\n"; 
    float L_interieur = L-3*tau;
    int counter = 0;
    //int iter_max = (L_interieur/tau)+1;
    //E = 0;
    //while(counter != iter_max){
        
    while(curr.x < p4.x && curr.y < p4.y) {
        //curr.x += tau;
        /*if(counter == 0) {
            curr.y = curr.y+L_interieur - 3*tau;
        }
        else {
        */    curr.y = (counter%2 == 0) ? curr.y+L_interieur - tau : curr.y-L_interieur + tau;
       //}
        /*if(counter == 1) {
            curr.y -=tau;
        } */
        //Augmente E par deltaE
        E += deltaE;
        //Déplacement G1 à ce point
        gcode_file << "G1 X" << curr.x << " Y" << curr.y << " E" << E << " F" << F_G1 << "\n";
        curr.x+=2*tau;
        gcode_file << "G1 X" << curr.x << " Y" << curr.y << " E" << E << " F" << F_G1 << "\n";
        /*if(curr.x != p4.x && curr.y != p4.y) {
            curr.x += tau;
            gcode_file << "G1 X" << curr.x << " Y" << curr.y << " E" << E << " F" << F_G1 << "\n";
        }*/
        counter++;
    }

    //Remet la buse à l'origine
    gcode_file << origin() << "\n";
 
    //Ferme les fichiers
    params.close();
    gcode_file.close();

    //Fin
    return 0;
}