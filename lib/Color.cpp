#include "../include/Color.hpp"

const size_t DIM = 3;
    
/* Constructeurs */
Color::Color() {
    _data = (float*) calloc(DIM, sizeof(float));
};

Color::Color(float x) {
    _data = (float*) malloc(DIM * sizeof(float));

    for(size_t i = 0; i < DIM; i++){
        _data[i] = x;
    }
};

Color::Color(float r, float g, float b) {
    _data = (float*) malloc(DIM * sizeof(float));

   
    _data[0] = r;
    _data[1] = g;
    _data[2] = b;
};

Color::Color(const Color &other) {
    _data = (float*) malloc(DIM * sizeof(float));
    if (_data == nullptr) {
        std::cerr << "Erreur d'allocation mémoire dans le constructeur de copie de Color" << std::endl;
        exit(EXIT_FAILURE);
    }
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    _data[2] = other._data[2];
}

Color::~Color() {
    free(_data);
}

/* Accesseur */
float Color::operator [] (const size_t i) const {
    if(i >= DIM){
        cerr << "Inde_data[0] out of bound" << endl;
        exit(EXIT_FAILURE);
    }

    return _data[i];
}

float &Color::operator [] (const size_t i) {
    if(i >= DIM){
        cerr << "Inde_data[0] out of bound" << endl;
        exit(EXIT_FAILURE);
    }

    return _data[i];
}

float* Color::data() {
    return _data;
}

/* Norme */
float Color::norm() const {
    return sqrt(squaredNorm());
}

float Color::squaredNorm() const {
    return dot(*this);
}


/* Normalisation */
void Color::normalize() {
    float length = norm();

    _data[0] /= length;
    _data[1] /= length;
    _data[2] /= length;
}

/* Opérations d'égalités vectorielles */
void Color::operator= (const Color &other){
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    _data[2] = other._data[2];
}

void Color::operator+= (const Color &other){
    _data[0] += other._data[0];
    _data[1] += other._data[1];
    _data[2] += other._data[2];
}

void Color::operator-= (const Color &other){
    _data[0] -= other._data[0];
    _data[1] -= other._data[1];
    _data[2] -= other._data[2];
}

void Color::operator*= (const Color &other){
    _data[0] *= other._data[0];
    _data[1] *= other._data[1];
    _data[2] *= other._data[2];
}

/* Opérations d'égalités scalaire */
void Color::operator*= (const float &a){
    _data[0] *= a;
    _data[1] *= a;
    _data[2] *= a;
}

void Color::operator/= (const float &a){
    _data[0] /= a;
    _data[1] /= a;
    _data[2] /= a;
}

ostream& operator<<(ostream &os, const Color &u) {
    for(size_t i = 0; i < 3; i++){
        os << "| ";
        os << std::fixed << std::setprecision(3) << u[i];
        os << " |\n";
    }

    return os;
};