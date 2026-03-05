#pragma once
#include <cmath>

class rigid
{
public:
    float x, y, phi;
    float v_x, v_y, omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    float M, J;
    float f_x, f_y, moment;
    int fix;

    rigid() : x(0), y(0), phi(0), v_x(0), v_y(0), omega(0), M(1), J(1), f_x(0), f_y(0), moment(0), fix(0){}

    rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie , int fix)
        : x(x_initial), y(y_initial), phi(phi_initial), M(masa), J(momentInertie), fix(fix), v_x(0), v_y(0), omega(0), f_x(0), f_y(0), moment(0){}

    void aflaForteProprii()
    {
        if(fix == 0){
            f_x = 0;
            f_y = -(0.01f) * M;
            moment = 0;
        }
        else{
            f_x = 0;
            f_y = 0;
            moment =0;
        }
    }

    void coordPunctPeCorp(int &punct_x, int &punct_y, int d_x, int d_y){                //d_x si d_y sunt coordonatele punctului fata de sistemul de referinta mobil al corpului
        punct_x = x + d_x* cos(phi) - d_y * sin(phi);
        punct_y = y + d_x* sin(phi) + d_y * cos(phi);
    }
};