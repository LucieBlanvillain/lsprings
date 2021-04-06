#include "lspring.h"

void progres_euler(float y, float p, float dp, float h, float *out_y, float *out_p) {
    *out_y = y + (h * p);
    *out_p = p + (h * dp);
}

void progres_rk(float y, float p, float dp, float dp_milieu, float h, float *out_y, float *out_p) {
    float p_milieu;
    float y_milieu;
    progres_euler(y, p, dp, h/2, &y_milieu, &p_milieu);
    *out_y = y + h * p_milieu;
    *out_p = p + h * dp_milieu;
}
