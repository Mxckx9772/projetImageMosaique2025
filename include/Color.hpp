#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;

class Color {
    private:
        float* _data;

        /* Opérations scalaires */
        friend inline Color operator *(const Color &u, const float &a);
        friend inline Color operator *(const float &a, const Color &u); // Definition commutative du produit.
        friend inline Color operator /(const Color &u, const float &a);

        /* Visualisation */
        friend ostream& operator <<(ostream &os, const Color &u);
        
    public:

        /* Constructeur */
        Color();
        Color(float x);
        Color(float r, float g, float b);
        //~Color();

        /* Accesseur */
        float operator [] (const size_t i) const;
        float &operator [] (const size_t i);

        /* Norme */
        float norm() const;
        float squaredNorm() const;

        /* Normalisation */
        void normalize();
        
        /* Opérations vectorielles internes */
        inline Color operator+(const Color &other) const {
            return Color(
                _data[0] + other._data[0], 
                _data[1] + other._data[1], 
                _data[2] + other._data[2]
            );
        }

        inline Color operator-(const Color &other) const {
            return Color(
                _data[0] - other._data[0], 
                _data[1] - other._data[1], 
                _data[2] - other._data[2]
            );
        }

        inline Color operator*(const Color &other) const {
            return Color(
                _data[0] * other._data[0], 
                _data[1] * other._data[1], 
                _data[2] * other._data[2]
            );
        }

        inline Color cross(const Color &other) const {
            return Color(
                (_data[1] * other._data[2]) - (_data[2] * other._data[1]), 
                (_data[2] * other._data[0]) - (_data[0] * other._data[2]), 
                (_data[0] * other._data[1]) - (_data[1] * other._data[0])
            );
        }

        /* Opérations vectorielles externes */
        inline float dot(const Color &other) const {
            return (_data[0] * other._data[0]) + (_data[1] * other._data[1]) + (_data[2] * other._data[2]);
        }

        inline bool operator==(const Color &other) const {
            return (_data[0] == other._data[0]) && (_data[1] == other._data[1]) && (_data[2] == other._data[2]);
        };

        /* Opérations d'égalités vectorielles */
        void operator=(const Color &other);
        void operator+=(const Color &other);
        void operator-=(const Color &other);
        void operator*=(const Color &other);

        /* Opérations d'égalités scalaire */
        void operator*=(const float &a);
        void operator/=(const float &a);
    
};

/* Lois externes */
inline Color operator *(const Color &u, const float &a) {
    return Color(u._data[0] * a, u._data[1] * a, u._data[2] * a);
}
 
inline Color operator *(const float &a, const Color &u) {
    return Color(a * u._data[0], a * u._data[1], a * u._data[2]);
}
 
inline Color operator /(const Color &u, const float &a) {
    return a != 0.0f ? Color(u._data[0] / a, u._data[1] / a, u._data[2] / a) : Color();
}

#endif