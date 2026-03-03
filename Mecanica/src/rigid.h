#pragma once

class rigid
{
public:
    float x, y, phi;
    float v_x, v_y, omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    float M, J;
    float f_x, f_y, moment;

    void aflaForteProprii()
    {
        f_x = 0;
        f_y = -(9.81f) * M;
        moment = 0;
    }
};