// Projet Image Mosaïque
//
// MacOS : brew install opencv
//     g++ base_code_mosaique.cpp -o base_code_mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
//     ./base_code_mosaique
//
// Linux : sudo apt update
//     sudo apt upgrade
//     sudo apt install libopencv-dev
//     sudo apt install nlohmann-json3-dev
//     g++ base_code_mosaique.cpp -o mosaique $(pkg-config --cflags --libs opencv4) -std=c++17 -ljpeg -pthread
//     ./base_code_mosaique

// Liens :
// Lien vers la banque d'image : https://drive.google.com/drive/folders/1qA-8ZMroFYy72y2pfmN4nWWo_82tpCod?usp=sharing
// Lien pour le csv.h : https://github.com/ben-strasser/fast-cpp-csv-parser/blob/master/csv.h
// Lien vers la banque prête à l'emploi : https://filesender.renater.fr/?s=download&token=300ef385-64e3-481a-9cf5-e1baf1fc9430

#include <cstdio>
#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>
#include "../src/csv.h"
#include "image_ppm.h"

using namespace std;

int TAILLE_BASE = 141988;


//-------------------------------------------------------------------------------------------------------------
// ------------------------------------------- Fonctions génériques -------------------------------------------
//-------------------------------------------------------------------------------------------------------------

// Fonction pour afficher la progression de la conversion des images
void afficherProgression(int current, int total, chrono::steady_clock::time_point start_time) {
    int largeur = 50; // Largeur de la barre
    int progress = (current * largeur) / total;
    cout << "[";
    for (int i = 0; i < largeur; i++) {
        if (i < progress) cout << "=";
        else cout << " ";
    }
    cout << "] " << (current * 100) / total << "%";
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(now - start_time).count();
    int remaining = current > 0 ? elapsed * (total - current) / current : 0;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    if (seconds < 10) cout << " - Temps restant estimé : " << minutes << "m 0" << seconds << "s\r";
    else cout << " - Temps restant estimé : " << minutes << "m " << seconds << "s\r";
    cout.flush();
}

// Fonction pour convertir une image JPEG en PPM
void convertir_jpg_en_ppm(const string& chemin_jpg, const string& chemin_ppm,int size) {
    cv::Mat img = imread(chemin_jpg, cv::IMREAD_COLOR);
    if (img.empty()) {
        cerr << "Erreur : Impossible de lire l'image " << chemin_jpg << endl;
        return;
    }
    cv::Mat img_resized;
    resize(img,img_resized,cv::Size(size,size));
    if (!imwrite(chemin_ppm, img_resized))
        cerr << "Erreur : Impossible d'écrire l'image " << chemin_ppm << endl;
}

// Fonction pour convertir une image PPM en PGM
void convertir_ppm_pgm(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW) {
    for (int i = 0; i < nH; i++)
        for (int j = 0; j < nW; j++)
            ImgOut[i * nW + j] = 0.299 * ImgIn[(i * nW + j) * 3] + 0.587 * ImgIn[(i * nW + j) * 3 + 1] + 0.114 * ImgIn[(i * nW + j) * 3 + 2];
}

//-------------------------------------------------------------------------------------------------------------
// ------------------------------------ Pré-traitement de la base d'images ------------------------------------
//-------------------------------------------------------------------------------------------------------------

void redimensionner_image(OCTET*& ImgIn, int& nH, int& nW, int taille_imagette) {
    int newH = ceil(nH / taille_imagette) * taille_imagette;
    int newW = ceil(nW / taille_imagette) * taille_imagette;
    if (newH == nH && newW == nW)
        return;
    cout << "Redimensionnement de l'image pour être un multiple de " << taille_imagette << "." << endl;
    cout << "Dimensions originales : " << nH << "x" << nW << ", nouvelles dimensions : " << newH << "x" << newW << "." << endl;
    OCTET* ImgResized;
    allocation_tableau(ImgResized, OCTET, newH * newW * 3);
    for (int i = 0; i < newH; i++)
        for (int j = 0; j < newW; j++) {
            ImgResized[(i * newW + j) * 3] = ImgIn[(i * nW + j) * 3];         // Rouge
            ImgResized[(i * newW + j) * 3 + 1] = ImgIn[(i * nW + j) * 3 + 1]; // Vert
            ImgResized[(i * newW + j) * 3 + 2] = ImgIn[(i * nW + j) * 3 + 2]; // Bleu
        }
    free(ImgIn);
    ImgIn = ImgResized;
    nH = newH;
    nW = newW;
}

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PGM ------------------------------------
            //-------------------------------------------------------------------------------------

// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
void sauver_base_Image(const unordered_map<string, unordered_map<string, vector<double>>>& base_images, const string& fichier) {
    ofstream out(fichier);
    if (!out) {
        cerr << "Erreur d'ouverture du fichier pour la sauvegarde." << endl;
        return;
    }

    // Écrire les en-têtes
    out << "Chemin,Moyenne,Histogramme\n";

    // Écrire les données
    for (const auto& pair : base_images) {
        out << pair.first; // Chemin de l'image

        // Écrire la moyenne
        const auto& moyenne = pair.second.at("moyenne");
        if (!moyenne.empty()) {
            out << "," << moyenne[0];
        } else {
            out << ",0"; // Valeur par défaut si la moyenne est vide
        }

        // Écrire l'histogramme
        const auto& histogramme = pair.second.at("histogramme");
        out << ",";
        for (size_t i = 0; i < histogramme.size(); ++i) {
            out << histogramme[i];
            if (i < histogramme.size() - 1) {
                out << ";"; // Séparateur pour les valeurs de l'histogramme
            }
        }

        out << "\n"; // Nouvelle ligne pour chaque image
    }

    cout << "Base d'images sauvegardée dans : " << fichier << endl;
}

// Méthode pour charger les données enregistrées précédemment :
unordered_map<string, unordered_map<string, vector<double>>> charger_base_images_pgm_depuis_fichier(const string& fichier) {
    unordered_map<string, unordered_map<string, vector<double>>> base_images;
    ifstream in(fichier);

    if (!in) {
        cerr << "Erreur d'ouverture du fichier pour la lecture : " << fichier << endl;
        return base_images;
    }

    cout << " - Chargement de la base d'images PGM..." << endl;

    string line;

    // ignore le nom des colonnes
    getline(in, line);

    while (getline(in, line)) {
        istringstream iss(line);
        string path, histogramme_str;
        double moyenne;

        // Lire le chemin et la moyenne
        if (getline(iss, path, ',') && iss >> moyenne && iss >> histogramme_str) {
            // Convertir l'histogramme (séparé par des points-virgules) en vecteur de doubles
            vector<double> histogramme;
            istringstream hist_stream(histogramme_str);
            string hist_value;
            while (getline(hist_stream, hist_value, ';')) {
                if (hist_value[0] != ',') {
                    histogramme.push_back(stod(hist_value));
                } else {
                    hist_value = hist_value.substr(1);
                    histogramme.push_back(stod(hist_value));
                }
            }

            // Ajouter les données dans la structure
            base_images[path]["moyenne"] = {moyenne};
            base_images[path]["histogramme"] = histogramme;
        } else
            cerr << "    - Format incorrect dans le fichier : " << line << endl;
    }

    cout << "    - Chargement terminé" << endl;
    return base_images;
}

// Méthode pour découper l'image en blocs de taille choisie : 
void decoupe_pgm(OCTET* Imgin, OCTET* ImgOut, int nH, int nW, int size) {
	if (nH%size!=0 || nW%size!=0) {
		cout << "Erreur : les dimensions de l'image ne sont paun multiple des dimensions de la mosaique" << endl;
		return;
	}
	for (int i=0; i < nH; i+=size)
		for (int j=0; j < nW; j+=size) {
			int moyenne = 0;
			for (int k=0; k < size; k++)
				for (int l=0; l < size; l++)
					moyenne += Imgin[(i+k)*nW+j+l];
			moyenne /= size*size;
			for (int k=0; k < size; k++)
				for (int l=0; l < size; l++)
					ImgOut[(i+k)*nW+j+l] = moyenne;
		}
}

// Calcul du critère moyenneur :
// int critere_img_mean_pgm(OCTET* ImgIn,int nH,int nW){
// 	int moy = 0;
// 	int nTaille = nH*nW;
//     int nTaille3 = nTaille*3;
// 	for(int i=0;i<nTaille3;i+=3){
// 		moy+= 0.299*ImgIn[i] + 0.587*ImgIn[i+1] + 0.114*ImgIn[i+2];
// 	}
// 	return moy/nTaille;
// }

//Calcul du critère moyenneur :
unordered_map<string, vector<double>> calculer_criteres_pgm(OCTET* ImgIn, int nH, int nW, double& moyenne, vector<double>& histogramme ) {
    moyenne = 0.0;
    histogramme.assign(256, 0.0);
    int nTaille = nH * nW;
    for (int i = 0; i < nTaille; i++) {
        int val = ImgIn[i];
        moyenne += val;
        histogramme[val]++;
    }
    moyenne /= nTaille;
    return {{"moyenne", {moyenne}}, {"histogramme", histogramme}};
}

// Fonction pour calculer la base d'images et stocker leurs moyennes : 
unordered_map<string,unordered_map<string, vector<double>>> charger_base_images_pgm(const string& dossier, int size) {
    unordered_map<string,unordered_map<string, vector<double>>>base_images;
    vector<filesystem::directory_entry> fichiers;

    // Collecter tous les fichiers PPM dans le dossier
    for (const auto& entry : filesystem::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".ppm") {
            fichiers.push_back(entry);
        }
    }

    // Nombre de threads à utiliser
    int num_threads = min(8u, thread::hardware_concurrency());
    cout << "Nombre de threads : " << num_threads << endl;

    // Diviser les fichiers en groupes pour chaque thread
    size_t total_fichiers = fichiers.size();
    size_t fichiers_par_thread = (total_fichiers + num_threads - 1) / num_threads;

    mutex mtx; // Mutex pour protéger l'accès à `base_images`
    mutex progress_mtx; // Mutex pour protéger l'affichage de la progression
    int cpt = 0; // Compteur partagé pour la progression
    auto start_time = chrono::steady_clock::now();

    auto traiter_fichiers = [&](size_t start, size_t end) {
        OCTET* ImgTmp;
        allocation_tableau(ImgTmp, OCTET, size * size * 3);

        for (size_t i = start; i < end; ++i) {
            const auto& entry = fichiers[i];
            string chemin = entry.path().string();

            // Lire l'image et calculer le critère
            lire_image_ppm(const_cast<char*>(chemin.c_str()), ImgTmp, size * size);
            double moyenne;
            vector<double> histogramme;

            // Protéger l'accès à `base_images` avec un mutex
            {
                auto critere = calculer_criteres_pgm(ImgTmp, size, size, moyenne, histogramme);
                lock_guard<mutex> lock(mtx);
                base_images[chemin] = critere;
            }

            // Mettre à jour la progression
            {
                lock_guard<mutex> progress_lock(progress_mtx);
                cpt++;
                afficherProgression(cpt, total_fichiers, start_time);
            }
        }

        free(ImgTmp);
    };

    // Lancer les threads
    vector<thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * fichiers_par_thread;
        size_t end = min(start + fichiers_par_thread, total_fichiers);
        if (start < end) {
            threads.emplace_back(traiter_fichiers, start, end);
        }
    }

    // Attendre la fin de tous les threads
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    cout << endl; // Pour terminer la ligne de progression proprement
    return base_images;
}


            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PPM ------------------------------------
            //-------------------------------------------------------------------------------------

// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
// O(N)
// N = taille de base_images
void sauver_base_Image_ppm(const unordered_map<string, unordered_map<string, vector<double>>>& base_images, const string& fichier) {
    ofstream out(fichier);
    if (!out) {
        cerr << "Erreur : Impossible d'ouvrir le fichier " << fichier << " pour l'écriture." << endl;
        return;
    }

    // Écrire les en-têtes
    out << "Chemin,Moyenne_R,Moyenne_G,Moyenne_B,Histogramme_R,Histogramme_G,Histogramme_B\n";

    // Écrire les données
    for (const auto& pair : base_images) {
        out << pair.first; // Chemin de l'image

        // Vérifier et écrire les moyennes
        if (pair.second.find("moyenne") != pair.second.end() && pair.second.at("moyenne").size() == 3) {
            const auto& moyenne = pair.second.at("moyenne");
            out << "," << moyenne[0] << "," << moyenne[1] << "," << moyenne[2];
        } else {
            out << ",0,0,0"; // Valeurs par défaut si les moyennes sont manquantes
        }

        // Vérifier et écrire les histogrammes
        for (const string& hist_key : {"hist_R", "hist_G", "hist_B"}) {
            out << ",";
            if (pair.second.find(hist_key) != pair.second.end()) {
                const auto& histogramme = pair.second.at(hist_key);
                for (size_t i = 0; i < histogramme.size(); ++i) {
                    out << histogramme[i];
                    if (i < histogramme.size() - 1) {
                        out << ";"; // Séparateur pour les valeurs de l'histogramme
                    }
                }
            } else {
                out << "0"; // Valeur par défaut si l'histogramme est manquant
            }
        }

        out << "\n"; // Nouvelle ligne pour chaque image
    }

    cout << "Données sauvegardées dans le fichier : " << fichier << endl;
}
// Méthode pour charger les données enregistrées précédemment :
// O(N log N)
// N = taille de base_images
unordered_map<string, unordered_map<string, vector<double>>> charger_base_images_ppm_depuis_fichier(const string& fichier) {
    unordered_map<string, unordered_map<string, vector<double>>> base_images;
    ifstream in(fichier);

    if (!in) {
        cerr << " - Erreur d'ouverture du fichier pour la lecture : " << fichier << endl;
        return base_images;
    }

    cout << " - Chargement de la base d'images PPM..." << endl;

    string line;

    // ignore le nom des colonnes
    getline(in, line);

    while (getline(in, line)) {
        istringstream iss(line);
        string path, hist_R_str, hist_G_str, hist_B_str;
        double moyenne_R, moyenne_G, moyenne_B;
        char delimiter;

        // Lire le chemin, les moyennes et les histogrammes
        if (getline(iss, path, ',') &&
            iss >> moyenne_R >> delimiter &&
            iss >> moyenne_G >> delimiter &&
            iss >> moyenne_B >> delimiter &&
            getline(iss, hist_R_str, ',') &&
            getline(iss, hist_G_str, ',') &&
            getline(iss, hist_B_str, ',')) {

            // Convertir les histogrammes (séparés par des points-virgules) en vecteurs de doubles
            vector<double> hist_R, hist_G, hist_B;
            istringstream hist_R_stream(hist_R_str), hist_G_stream(hist_G_str), hist_B_stream(hist_B_str);
            string hist_value;

            while (getline(hist_R_stream, hist_value, ';'))
                hist_R.push_back(stod(hist_value));
            while (getline(hist_G_stream, hist_value, ';'))
                hist_G.push_back(stod(hist_value));
            while (getline(hist_B_stream, hist_value, ';'))
                hist_B.push_back(stod(hist_value));

            // Ajouter les données dans la structure
            base_images[path]["moyenne"] = {moyenne_R, moyenne_G, moyenne_B};
            base_images[path]["hist_R"] = hist_R;
            base_images[path]["hist_G"] = hist_G;
            base_images[path]["hist_B"] = hist_B;
        } else {
            cerr << "    - Format incorrect dans le fichier : " << line << endl;
        }
    }

    cout << "    - Chargement terminé" << endl;
    return base_images;
}

void decoupe_ppm(OCTET* Imgin, OCTET* ImgOut, int nH, int nW, int size){
	if(nH%size!=0 || nW%size!=0) {
		cout << "Erreur : les dimensions de l'image ne sont pas un multiple des dimensions de la mosaique" << endl;
		return;
	}
	for (int i=0; i < nH; i+=size)
        for (int j=0; j<nW*3; j+=size*3) {
			int moyenneR = 0; int moyenneG = 0; int moyenneB = 0;
			for (int k=0; k < size; k++)
				for (int l=0; l < size*3; l+=3) {
					moyenneR += Imgin[(i+k)*nW*3+j+l];
                    moyenneG += Imgin[(i+k)*nW*3+j+l+1];
                    moyenneB += Imgin[(i+k)*nW*3+j+l+2];
				}
			moyenneR /= size*size; moyenneG /= size*size; moyenneB/= size*size;
			for (int k=0; k < size; k++)
				for (int l=0; l < size*3; l+=3) {
					ImgOut[(i+k)*nW*3+j+l] = moyenneR;
                    ImgOut[(i+k)*nW*3+j+l+1] = moyenneG;
                    ImgOut[(i+k)*nW*3+j+l+2] = moyenneB;
				}
		}
}

// Calcul du critère moyenneur :
// O(nH*nW)
// vector<double> critere_img_mean_ppm(OCTET* ImgIn,int nH,int nW){
// 	double moyR = 0; double moyG =0; double moyB = 0;
// 	int nTaille = nH*nW;
//     int nTaille3 = nTaille*3;
// 	for(int i=0;i<nTaille3;i+=3){
// 		moyR+= ImgIn[i];
//         moyG+= ImgIn[i+1];
//         moyB+= ImgIn[i+2];
// 	}
// 	return {moyR/nTaille,moyG/nTaille,moyB/nTaille};
// }

// Calcul du critère moyenneur :
// O(nH*nW)
unordered_map<string, vector<double>> critere_img_mean_ppm(OCTET* ImgIn, int nH, int nW) {
	vector moyenne = {0.0, 0.0, 0.0};
    vector<double> hist_R;
    vector<double> hist_G;
    vector<double> hist_B;
    hist_R.assign(256,0.0);
    hist_G.assign(256,0.0);
    hist_B.assign(256,0.0);
	int nTaille = nH*nW;
    int nTaille3 = nTaille*3;
	for (int i=0; i < nTaille3; i+=3) {
        int R = ImgIn[i];
        int G = ImgIn[i+1];
        int B = ImgIn[i+2];
		moyenne[0]+= R;
        moyenne[1]+= G;
        moyenne[2]+= B;
        hist_R[R]++;
        hist_G[G]++;
        hist_B[B]++;
	}
	for(int i=0;i<3;i++){
        moyenne[i] /= nTaille;
    }
    return {{"moyenne", moyenne}, {"hist_R", hist_R}, {"hist_G", hist_G}, {"hist_B", hist_B}};
}


// Fonction pour calculer la base d'images et stocker leurs moyennes :
unordered_map<string, unordered_map<string,vector<double>>> charger_base_images_ppm(const string& dossier,int size) {
    unordered_map<string, unordered_map<string,vector<double>>> base_images;
	OCTET* ImgTmp;
    int cpt = 0;
	allocation_tableau(ImgTmp, OCTET,size*size*3);
    auto start_time = chrono::steady_clock::now();
    for (const auto& entry : filesystem::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".ppm") {
            lire_image_ppm(const_cast<char*>(entry.path().string().c_str()), ImgTmp, size*size);
			base_images[entry.path().string()] = critere_img_mean_ppm(ImgTmp,size,size);
        }
        cpt++;
        afficherProgression(cpt, TAILLE_BASE, start_time);
    }
    return base_images;
}



// Génération de la base d'images à partir d'un dossier source
void convertir(const string& source_folder, const string& ppm_folder, int size) {
    if (filesystem::exists(ppm_folder)) {
        cout << "Le dossier " << ppm_folder << " existent déjà. Conversion ignorée." << endl;
        return;
    }
    int cpt = 0;
    filesystem::create_directories(ppm_folder);
    auto start_time = chrono::steady_clock::now();
    vector<thread> threads;
    int num_threads = min(8u, thread::hardware_concurrency());
    cout << "Nombre de threads : " << num_threads << endl;
    mutex mtx;
    condition_variable cv;
    int active_threads = 0;
    for (const auto& entry : filesystem::recursive_directory_iterator(source_folder)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".jpeg") { // Vérifier l'extension
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [&] { return active_threads < num_threads; });
                active_threads++;
                threads.emplace_back([&, img_path = entry.path().string(), cpt]() {
                    convertir_jpg_en_ppm(img_path, ppm_folder + "/" + to_string(cpt) + ".ppm", size);
                    {
                        lock_guard lock(mtx);
                        active_threads--;
                    }
                    cv.notify_one();
                });
                cpt++;
                afficherProgression(cpt, TAILLE_BASE, start_time);
            } else {
                cerr << "Fichier ignoré (non JPEG) : " << entry.path() << endl;
            }
        }
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cout << "Conversion terminée ! " << cpt << " Images créées !" << endl;
    TAILLE_BASE = cpt;
}

void generer_base_imagettes(const string& dossier_source, const string& dossier_destination) {
    if(!filesystem::exists(dossier_source)){
        cout << "Le dossier " << dossier_source << " n'existe pas." << endl;
        return;
    }
    for(int i=2; i<=7 ;i++){
        int taille_imagette = pow(2,i);
        string dossier_sortie = dossier_destination + "/imagettes_" + to_string(taille_imagette);
        cout<<"Génération de la base pour les imagettes de taille : "<<taille_imagette<<endl;
        if(!filesystem::exists(dossier_sortie)){
            convertir(dossier_source, dossier_sortie, taille_imagette);
            filesystem::create_directories(dossier_sortie);
            string fichier_base_images = dossier_sortie + "/base_images_" + to_string(taille_imagette)+".csv";
            string fichier_base_images_ppm = dossier_sortie + "/base_images_ppm_" + to_string(taille_imagette)+".csv";
            auto base_images = charger_base_images_pgm(dossier_sortie , taille_imagette);
            sauver_base_Image(base_images, fichier_base_images);
            auto base_images_ppm = charger_base_images_ppm(dossier_sortie, taille_imagette);
            sauver_base_Image_ppm(base_images_ppm, fichier_base_images_ppm);
        }
    }
}

vector<int> choix_taille_imagette(int nW) {
    vector<int> tailles_imagettes;
    vector tailles = {20,40,80};
    for (int blocs : tailles) {
        int taille_imagette = nW/blocs;
        int nv_entree = max(4, min(128, static_cast<int>(pow(2, round(log2(taille_imagette))))));
        if (find(tailles_imagettes.begin(), tailles_imagettes.end(), nv_entree) == tailles_imagettes.end())
            tailles_imagettes.push_back(nv_entree);
    }
    return tailles_imagettes;
}

// TODO problème avec les images couleurs
vector<vector<double>> calc_histograms(OCTET* ImgIn, int nr_composantes,
    int start_x, int start_y, int nH, int nW, int taille_imagette) {
    vector<vector<double>> histograms(nr_composantes);
    for (int i = 0; i < nr_composantes; i++) {
        histograms[i].resize(256);
        for (int y = 0; y < taille_imagette; y++)
            for (int x = 0; x < taille_imagette; x+=nr_composantes)
                if (start_y + y < nH && start_x + x < nW)
                    histograms[i][ImgIn[(start_y + y) * nW + (start_x + x) * nr_composantes + i]] ++;
    }
    return histograms;
}

//-------------------------------------------------------------------------------------------------------------
// ------------------------------------------ Traitement de l'image -------------------------------------------
//-------------------------------------------------------------------------------------------------------------

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PGM ------------------------------------
            //-------------------------------------------------------------------------------------

// Méthode de génération de l'image mosaïque à partir des données calculées : 
void generationImage_pgm_moyenne(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int taille_imagette,
    unordered_map<string, unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET *ImgTmp, *ImgTmp_pgm;
    allocation_tableau(ImgTmp, OCTET, taille_imagette * taille_imagette * 3);
    allocation_tableau(ImgTmp_pgm, OCTET, taille_imagette * taille_imagette);
    unordered_set<string> imagettes_used;
    auto start_time = chrono::steady_clock::now();
    for (int i = 0; i < nH; i += taille_imagette) {
        for (int j = 0; j < nW; j += taille_imagette) {
            double distanceMin = 256;
            string path;
            for (const auto& pair : base_images)
                if (imagettes_used.find(pair.first) == imagettes_used.end()) {
                    if (abs(pair.second.at("moyenne")[0] - ImgIn[i * nW + j]) < distanceMin) {
                        distanceMin = abs(pair.second.at("moyenne")[0] - ImgIn[i * nW + j]);
                        path = pair.first;
                    }
                    if (distanceMin == 0)
                        break;
                }

            imagettes_used.insert(path);
            lire_image_ppm(const_cast<char *>(path.c_str()), ImgTmp, taille_imagette * taille_imagette);
            convertir_ppm_pgm(ImgTmp, ImgTmp_pgm, taille_imagette, taille_imagette);

            for (int y = 0; y < taille_imagette; y++)
                for (int x = 0; x < taille_imagette; x++) {
                    if (i + y < nH && j + x < nW) {
                        ImgOut[(i + y) * nW + (j + x)] = ImgTmp_pgm[y * taille_imagette + x];
                    }
                }
        afficherProgression(i * nW + j, nTaille, start_time);
        }
    }
    afficherProgression(1, 1, start_time);
    cout << endl;
    free(ImgTmp);
}

double correlation_histo(const vector<double>& histoA, const vector<double>& histoB) {
    if (histoA.size() != histoB.size()) {
        cerr << "Les histogrammes doivent avoir la même taille." << endl;
        return -1;
    }

    int N = histoA.size();
    double sumA = 0, sumB = 0;
    double sumA2 = 0, sumB2 = 0;
    double sumAB = 0;

    // Calcul des sommes nécessaires
    for (int i = 0; i < N; ++i) {
        sumA += histoA[i];
        sumB += histoB[i];
        sumA2 += histoA[i] * histoA[i];
        sumB2 += histoB[i] * histoB[i];
        sumAB += histoA[i] * histoB[i];
    }

    // Moyennes des histogrammes
    double meanA = sumA / N;
    double meanB = sumB / N;

    // Calcul de la corrélation de Pearson
    double numerator = sumAB - N * meanA * meanB;
    double denominator = std::sqrt((sumA2 - N * meanA * meanA) * (sumB2 - N * meanB * meanB));

    // Si le dénominateur est nul, retourner 0 (pas de corrélation)
    if (denominator == 0) {
        cerr << "Erreur : Dénominateur nul, la corrélation est indéfinie." << endl;
        return 0;
    }

    return numerator / denominator;
}

void generationImage_pgm_histo(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int taille_imagette,
    unordered_map<string, unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET *ImgTmp, *ImgTmp_pgm;
    allocation_tableau(ImgTmp, OCTET, taille_imagette * taille_imagette * 3);
    allocation_tableau(ImgTmp_pgm, OCTET, taille_imagette * taille_imagette);
    unordered_set<string> imagettes_used;
    auto start_time = chrono::steady_clock::now();
    for (int i = 0; i < nH; i += taille_imagette) {
        for (int j = 0; j < nW; j += taille_imagette) {
            double corrMax = 0;
            string path;
            vector<vector<double>> histograms = calc_histograms(ImgIn, 1, j, i, nH, nW, taille_imagette);

            for (const auto& pair : base_images)
                if (imagettes_used.find(pair.first) == imagettes_used.end()) {
                    double corr = 0;
                    corr = correlation_histo(pair.second.at("histogramme"), histograms[0]);
                    if (corr > corrMax) {
                        corrMax = corr;
                        path = pair.first;
                    }
                    if (corrMax == 1.0f)
                        break;
                }

            lire_image_ppm(const_cast<char *>(path.c_str()), ImgTmp, taille_imagette * taille_imagette);
            convertir_ppm_pgm(ImgTmp, ImgTmp_pgm, taille_imagette, taille_imagette);

            for (int y = 0; y < taille_imagette; y++)
                for (int x = 0; x < taille_imagette; x++) {
                    if (i + y < nH && j + x < nW) {
                        ImgOut[(i + y) * nW + (j + x)] = ImgTmp_pgm[y * taille_imagette + x];
                    }
                }

        afficherProgression(i * nW + j, nTaille, start_time);
        }
    }
    afficherProgression(1, 1, start_time);
    cout << endl;
    free(ImgTmp);
}

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PPM ------------------------------------
            //-------------------------------------------------------------------------------------

// Méthode de génération de l'image mosaïque à partir des données calculées : 
//O( nH*nW*M / size^2 )
void generationImage_ppm_moyenne(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int taille_imagette,
    unordered_map<string,unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET* ImgTmp;
    // Allocation mémoire pour ImgTmp (une imagette complète)
    allocation_tableau(ImgTmp, OCTET, taille_imagette * taille_imagette * 3);
    OCTET* ImgResized ;
    allocation_tableau(ImgResized,OCTET, taille_imagette * taille_imagette * 3);
    unordered_set<string> uset;
    auto start_time = chrono::steady_clock::now();
    // Boucles pour parcourir l'image d'entrée par blocs de 'size x size'
    for (int i = 0; i < nH; i += taille_imagette) {
        for (int j = 0; j < nW; j += taille_imagette) {
            int distanceMin = 256;
            string path;

            // Trouver l'image la plus proche en couleur moyenne
            for (const auto& pair : base_images) {
                if (uset.find(pair.first) == uset.end()) {
                    double distance = sqrt(
                        pow(pair.second.at("moyenne")[0] - ImgIn[(i * nW + j) * 3], 2) +
                        pow(pair.second.at("moyenne")[1] - ImgIn[(i * nW + j) * 3 + 1], 2) +
                        pow(pair.second.at("moyenne")[2] - ImgIn[(i * nW + j) * 3 + 2], 2)
                    );

                    if (distance < distanceMin) {
                        distanceMin = distance;
                        path = pair.first;
                    }

                    if (distanceMin == 0) {
                        break;
                    }
                }
            }
            uset.insert(path);

            // Lire l'image sélectionnée et stocker dans ImgTmp
            lire_image_ppm(const_cast<char*>(path.c_str()), ImgTmp, taille_imagette * taille_imagette);

            // Insérer l'image redimensionnée dans ImgOut à la position (i, j)
            for (int y = 0; y < taille_imagette; y++) {
                for (int x = 0; x < taille_imagette; x++) {
                    int imgOutIdx = ((i + y) * nW + (j + x)) * 3;
                    int resizedIdx = (y * taille_imagette + x) * 3;

                    // Récupérer les valeurs RGB et les insérer en RGB dans ImgOut
                    ImgOut[imgOutIdx] = ImgTmp[resizedIdx];        // Rouge
                    ImgOut[imgOutIdx + 1] = ImgTmp[resizedIdx + 1]; // Vert
                    ImgOut[imgOutIdx + 2] = ImgTmp[resizedIdx + 2]; // Bleu
                }
            }
            afficherProgression(i * nW + j, nTaille, start_time);
        }
    }
    afficherProgression(1, 1, start_time);
    cout << endl;
    free(ImgResized);
    free(ImgTmp);
}

void generationImage_ppm_histo(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int taille_imagette,
    unordered_map<string,unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET* ImgTmp;
    // Allocation mémoire pour ImgTmp (une imagette complète)
    allocation_tableau(ImgTmp, OCTET, taille_imagette * taille_imagette * 3);
    OCTET* ImgResized ;
    allocation_tableau(ImgResized,OCTET, taille_imagette * taille_imagette * 3);
    unordered_set<string> imagettes_used;
    auto start_time = chrono::steady_clock::now();
    // Boucles pour parcourir l'image d'entrée par blocs de 'size x size'
    for (int i = 0; i < nH; i += taille_imagette) {
        for (int j = 0; j < nW; j += taille_imagette) {
            double corrMoyMax = 0;
            string path;
            vector<vector<double>> histograms = calc_histograms(ImgIn, 3, j, i, nH, nW, taille_imagette);

            for (const auto& pair : base_images) {
                if (imagettes_used.find(pair.first) == imagettes_used.end()) {
                    double corrR = correlation_histo(pair.second.at("hist_R"), histograms[0]);
                    double corrG = correlation_histo(pair.second.at("hist_G"), histograms[1]);
                    double corrB = correlation_histo(pair.second.at("hist_B"), histograms[2]);
                    double corrMoy = (corrR + corrG + corrB) / 3;
                    if (corrMoy > corrMoyMax) {
                        corrMoyMax = corrMoy;
                        path = pair.first;
                    }
                    // if (corrMoy == 1.0f)
                        // break;
                }
            }

            // cout << path << endl;
            lire_image_ppm(const_cast<char*>(path.c_str()), ImgTmp, taille_imagette * taille_imagette);

            for (int y = 0; y < taille_imagette; y++) {
                for (int x = 0; x < taille_imagette; x++) {
                    int imgOutIdx = ((i + y) * nW + (j + x)) * 3;
                    int resizedIdx = (y * taille_imagette + x) * 3;

                    // Récupérer les valeurs RGB et les insérer en RGB dans ImgOut
                    ImgOut[imgOutIdx] = ImgTmp[resizedIdx];        // Rouge
                    ImgOut[imgOutIdx + 1] = ImgTmp[resizedIdx + 1]; // Vert
                    ImgOut[imgOutIdx + 2] = ImgTmp[resizedIdx + 2]; // Bleu
                }
            }
            afficherProgression(i * nW + j, nTaille, start_time);
        }
    }
    afficherProgression(1, 1, start_time);
    cout << endl;
    free(ImgResized);
    free(ImgTmp);
}


//-------------------------------------------------------------------------------------------------------------
// -------------------------------------------- Qualité de l'image --------------------------------------------
//-------------------------------------------------------------------------------------------------------------

double EQM_pgm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = 0.0;
    int nTaille = nH * nW;
    
    for (int i = 0; i < nTaille; i++) {
        eqm += pow(img1[i] - img2[i], 2);
    }
    
    eqm /= nH * nW * 3;
    return eqm;
    }
    
double PSNR_pgm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = EQM_pgm(img1, img2, nH, nW);
    if (eqm == 0) {
        return INFINITY;  // Les images sont identiques
    }
    double psnr = 10 * log10(pow(255.0,2) / eqm);
    return psnr;
}

double EQM_ppm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = 0.0;
    int nTaille = nH * nW * 3; // Chaque pixel a 3 canaux (R, G, B)
    
    for (int i = 0; i < nTaille; i++) {
        eqm += pow(img1[i] - img2[i], 2);
    }
    
    eqm /= nTaille; // Diviser par le nombre total de valeurs (R, G, B)
    return eqm;
}

double PSNR_ppm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = EQM_ppm(img1, img2, nH, nW);
    if (eqm == 0) {
        return INFINITY;  // Les images sont identiques
    }
    double psnr = 10 * log10(pow(255.0, 2) / eqm);
    return psnr;
}


// Main pour effectuer toutes les étapes de génération de l'image mosaïque :
int main()
{
    OCTET *ImgIn_ppm, *ImgOut_ppm, *ImgIn_pgm, *ImgOut_pgm, *ImgOut2;
    string ImgInName, ImgOutName;
    int nH, nW, nTaille, taille_imagette;
    bool continuer = true;
    unordered_map<string,unordered_map<string, vector<double>>> base_images_ppm;
    unordered_map<string,unordered_map<string, vector<double>>> base_images_pgm;

    cout << "Bienvenue dans cette aplication de génération d'images mosaïque" << endl;
	filesystem::create_directories("out");
	cout << "Chargement de la base d'images... " << endl;
    auto start_time = chrono::steady_clock::now();
	generer_base_imagettes("in/base_images","out/imagettes_ppm");
    auto elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
    int minutes = static_cast<int>(elapsed / 60);
    int seconds = static_cast<int>(elapsed % 60);
    cout << "Chargement de la base terminé en : " << minutes << "m " << seconds << "s" << endl;

    do {
        // TODO faire choisir le critère
        string critere = "histo";
        // TODO refaire fonctionner les images PGM
        do {
            cout << "Veuillez donner une image à transformer en mosaïque :" << endl;
            cin >> ImgInName;
            if (filesystem::exists("./in/"+ImgInName))
                break;
            cout << "L'image n'existe pas." << endl;
        } while (true);
        ImgOutName = ImgInName.substr(0, ImgInName.size()-4);
        lire_nb_lignes_colonnes_image_ppm(const_cast<char*>(("./in/"+ImgInName).c_str()), &nH, &nW);
        nTaille = nH * nW;
        allocation_tableau(ImgIn_ppm, OCTET, nTaille*3);
        lire_image_ppm(const_cast<char*>(("./in/"+ImgInName).c_str()), ImgIn_ppm, nTaille);

        // TODO boucle while tant qu'on a pas choisi une option valide
        vector tailles_imagettes{4, 8, 16, 32, 64, 128};
        vector<int> tailles_imagettes_reco = choix_taille_imagette(nW);
        cout << "Tailles d'imagettes recommandées : " << endl;
        vector<string> tailles_imagettes_str = {" - hautes qualité : "," - moyennes qualité : "," - basse qualité : "};
        for (int i=0; i < tailles_imagettes_reco.size(); i++)
            cout << tailles_imagettes_str[i] << tailles_imagettes_reco[i] << endl;
        cout << "Autre tailles possible : ";
        // TODO vérifier que l'image n'est pas plus petite que taille_imagettes
        for (int taille_imagettes : tailles_imagettes)
            if (find(tailles_imagettes_reco.begin(), tailles_imagettes_reco.end(), taille_imagettes) == tailles_imagettes_reco.end())
                cout << taille_imagettes << " ";
        cout << endl;
        cout << "Veuillez choisir une taille d'imagette : " << endl;
        cin >> taille_imagette;

        redimensionner_image(ImgIn_ppm, nH,nW, taille_imagette);
        // redimensionne l'image que si nécessaire
        if (nTaille != nH * nW) {
            nTaille = nH * nW;
            ecrire_image_ppm(const_cast<char*>(("./out/"+ImgOutName+"_redim.ppm").c_str()), ImgIn_ppm, nH, nW);
        }
        allocation_tableau(ImgIn_pgm, OCTET, nTaille);
        allocation_tableau(ImgOut_pgm, OCTET, nTaille);
        allocation_tableau(ImgOut_ppm, OCTET, nTaille*3);
        allocation_tableau(ImgOut2, OCTET, nTaille*3);
        convertir_ppm_pgm(ImgIn_ppm, ImgIn_pgm, nH, nW);

        cout << "Récupération des critères des imagettes :" << endl;
        string fichier_base_images_pgm = "out/imagettes_ppm/imagettes_"+to_string(taille_imagette)+"/base_images_"+to_string(taille_imagette)+".csv";
        string fichier_base_images_ppm = "out/imagettes_ppm/imagettes_"+to_string(taille_imagette)+"/base_images_ppm_"+to_string(taille_imagette)+".csv";
        if (filesystem::exists(fichier_base_images_pgm))
            base_images_pgm = charger_base_images_pgm_depuis_fichier(fichier_base_images_pgm);
        else
            base_images_pgm = charger_base_images_pgm(fichier_base_images_pgm, taille_imagette);
        if (filesystem::exists(fichier_base_images_ppm))
            base_images_ppm = charger_base_images_ppm_depuis_fichier(fichier_base_images_ppm);
        else
            base_images_ppm = charger_base_images_ppm(fichier_base_images_ppm, taille_imagette);

        if (critere == "moyenne") {
            cout << "Découpe de l'image en grille (PPM)" << endl;
            string image_decoupe = "out/"+ImgOutName+"_decoupe.ppm";
            decoupe_ppm(ImgIn_ppm, ImgOut_ppm, nH, nW, taille_imagette);
            ecrire_image_ppm(const_cast<char*>(image_decoupe.c_str()), ImgOut_ppm, nH, nW);
            cout << "Découpe de l'image en grille (PGM)" << endl;
            std:string image_decoupe_pgm = "out/"+ImgOutName+"_decoupe.pgm";
            decoupe_pgm(ImgIn_pgm, ImgOut_pgm, nH, nW, taille_imagette);
            ecrire_image_pgm(const_cast<char*>(image_decoupe_pgm.c_str()), ImgOut_pgm, nH, nW);
        }

        cout << "Génération de l'image mosaïque (PGM)" << endl;
        start_time = chrono::steady_clock::now();
        if (critere == "moyenne")
            generationImage_pgm_moyenne(ImgOut_pgm, ImgOut_pgm, nH, nW, taille_imagette, base_images_pgm);
        else
            generationImage_pgm_histo(ImgIn_pgm, ImgOut_pgm, nH, nW, taille_imagette, base_images_pgm);
        string image_mosaique_pgm = "out/"+ImgOutName+"_mosaique.pgm";
        ecrire_image_pgm(const_cast<char*>(image_mosaique_pgm.c_str()), ImgOut_pgm, nH, nW);
        elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
        minutes = static_cast<int>(elapsed / 60);
        seconds = static_cast<int>(elapsed % 60);
        cout << "Mosaïque PGM générée en : " << minutes << "m " << seconds << "s" << endl;
        cout << "Génération de l'image mosaïque (PPM)" << endl;
        start_time = chrono::steady_clock::now();
        string image_mosaique_ppm = "out/"+ImgOutName+"_mosaique.ppm";
        if (critere == "moyenne")
            generationImage_ppm_moyenne(ImgOut_ppm, ImgOut2, nH, nW, taille_imagette, base_images_ppm);
        else
            generationImage_ppm_histo(ImgIn_ppm, ImgOut2, nH, nW, taille_imagette, base_images_ppm);
        ecrire_image_ppm(const_cast<char*>(image_mosaique_ppm.c_str()), ImgOut2, nH, nW);
        elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
        minutes = static_cast<int>(elapsed / 60);
        seconds = static_cast<int>(elapsed % 60);
        cout << "Mosaïque PPM générée en : "<< minutes << "m " << seconds << "s" << endl;

        cout << "Qualité de l'image (PGM) :" << endl;
        double eqm_pgm = EQM_pgm(ImgIn_pgm, ImgOut_pgm, nH, nW);
        double psnr_pgm = PSNR_pgm(ImgIn_pgm, ImgOut_pgm, nH, nW);
        cout << " - EQM : " << eqm_pgm << endl;
        cout << " - PSNR : " << psnr_pgm << endl;

        cout << "Qualité de l'image (PPM) :" << endl;
        double eqm_ppm = EQM_ppm(ImgIn_ppm, ImgOut2, nH, nW);
        double psnr_ppm = PSNR_ppm(ImgIn_ppm, ImgOut2, nH, nW);
        cout << " - EQM : " << eqm_ppm << endl;
        cout << " - PSNR : " << psnr_ppm << endl;

        // cout << "Voulez-vous continuer ? [O/n]" << endl;
        // string reponse;
        // cin >> reponse;
        // if (reponse=="n" || reponse=="N" || reponse=="non" || reponse=="Non")
            continuer = false;
    } while (continuer);

    free(ImgIn_ppm);
    free(ImgOut_ppm);
    free(ImgIn_pgm);
    free(ImgOut_pgm);
    free(ImgOut2);

	return 0;
}
