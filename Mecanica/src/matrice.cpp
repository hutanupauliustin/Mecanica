#include "matrice.h"


    matrice::matrice() : linii(0), coloane(0), valori(nullptr) {}

    matrice::matrice(int n, int m) : linii(n), coloane(m) {
        valori = new float[n * m](); // Initializeaza cu 0
    }

    matrice::matrice(char type, int n, int m) : linii(n), coloane(m) {
        valori = new float[n * m]();
        if (type == 'I' || type == 'i' || type == '1') {
            if (n == m) {
                for (int i = 0; i < n; i++) valori[i * coloane + i] = 1.0f;
            } else {
                std::cerr << "[AVERTISMENT] Identitatea trebuie sa fie patratica.\n";
            }
        }
    }

    // 2. Copy Constructor
    matrice::matrice(const matrice &other) : linii(other.linii), coloane(other.coloane) {
        if (other.valori != nullptr) {
            valori = new float[linii * coloane];
            for (int i = 0; i < linii * coloane; i++) valori[i] = other.valori[i];
        } else {
            valori = nullptr;
        }
    }

    // 3. Move Constructor (Furtul de memorie)
    matrice::matrice(matrice &&other) noexcept : linii(other.linii), coloane(other.coloane), valori(other.valori) {
        other.linii = 0;
        other.coloane = 0;
        other.valori = nullptr;
    }

    // 4. Destructor
    matrice::~matrice() {
        delete[] valori;
        valori = nullptr;
    }

    // 5. Copy Assignment
    matrice& matrice::operator=(const matrice &B) {
        if (this != &B) {
            delete[] valori;
            linii = B.linii;
            coloane = B.coloane;
            if (B.valori != nullptr) {
                valori = new float[linii * coloane];
                for (int i = 0; i < linii * coloane; i++) valori[i] = B.valori[i];
            } else {
                valori = nullptr;
            }
        }
        return *this;
    }

    // 6. Move Assignment (Evita lag-ul pe procesor)
    matrice& matrice::operator=(matrice &&B) noexcept {
        if (this != &B) {
            delete[] valori;
            linii = B.linii;
            coloane = B.coloane;
            valori = B.valori;
            
            B.linii = 0;
            B.coloane = 0;
            B.valori = nullptr;
        }
        return *this;
    }

    // 7. Accesare elemente
    float* matrice::at(int i, int j) const {
        if (valori == nullptr) throw std::runtime_error("[EROARE MATRICE] Accesare matrice neinitializata!");
        return valori + i * coloane + j;
    }

    float& matrice::operator()(int i, int j) { return *at(i, j); }
    const float& matrice::operator()(int i, int j) const { return *at(i, j); }

    // 8. Operatii matematice
    matrice matrice::transpose() const {
        matrice T(coloane, linii);
        for (int i = 0; i < linii; i++) {
            for (int j = 0; j < coloane; j++) {
                T(j, i) = (*this)(i, j);
            }
        }
        return T;
    }

    matrice matrice::inverse() const {
        if (linii != coloane) return matrice();
        matrice B(linii, coloane);
        for (int i = 0; i < linii; i++) {
            float val = (*this)(i, i);
            B(i, i) = (std::abs(val) < 1e-9f) ? 0.0f : (1.0f / val);
        }
        return B;
    }

    matrice matrice::operator+(const matrice &B) const {
        matrice S(linii, coloane);
        for (int i = 0; i < linii * coloane; i++) S.valori[i] = valori[i] + B.valori[i];
        return S;
    }

    matrice matrice::operator-(const matrice &B) const {
        matrice S(linii, coloane);
        for (int i = 0; i < linii * coloane; i++) S.valori[i] = valori[i] - B.valori[i];
        return S;
    }

    matrice matrice::operator-() const {
        matrice S(linii, coloane);
        for (int i = 0; i < linii * coloane; i++) S.valori[i] = -valori[i];
        return S;
    }

    matrice matrice::operator*(const matrice &B) const {
        matrice P(linii, B.coloane);
        for (int i = 0; i < linii; i++) {
            for (int j = 0; j < B.coloane; j++) {
                float s = 0.0f;
                for (int k = 0; k < coloane; k++) {
                    s += (*this)(i, k) * B(k, j);
                }
                P(i, j) = s;
            }
        }
        return P;
    }

    matrice matrice::operator*(const float x) const {
        matrice R(linii, coloane);
        for (int i = 0; i < linii * coloane; i++) R.valori[i] = valori[i] * x;
        return R;
    }

    matrice matrice::operator^(const char *pow) const {
        if (pow[0] == 'T' || pow[0] == 't') return this->transpose();
        if (pow[0] == '-' && pow[1] == '1') return this->inverse();
        return *this;
    }

    void matrice::printmatrice() const {
        for (int i = 0; i < linii; i++) {
            for (int j = 0; j < coloane; j++) fprintf(stdout, "%8.4f ", (*this)(i, j));
            fprintf(stdout, "\n");
        }
    }

matrice operator*(const float x, const matrice &M) {
    return M * x;
}