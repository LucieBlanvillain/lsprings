#include "lspring.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ABOVE_ABSENT 0x01
#define BELOW_ABSENT 0x03

typedef struct l_spring {
    float rate;
    float free_length;
} spring;

typedef struct l_point {
    float mass;
    float position;
    float speed;
} point;

void euler_approximation(float y, float p, float dp, float h, float *out_y, float *out_p) {
    *out_y = y + (h * p);
    *out_p = p + (h * dp);
}

void rk2_approximation(float y, float p, float dp, float dp_milieu, float h, float *out_y, float *out_p) {
    float p_milieu;
    float y_milieu;
    euler_approximation(y, p, dp, h/2, &y_milieu, &p_milieu);
    *out_y = y + h * p_milieu;
    *out_p = p + h * dp_milieu;
}

point step_point(float step_size, float time, float constant, float friction, float pulsation, point p, point p_above, point p_below, spring s_above, spring s_below, uint8_t s_presence) {
    point returned_point;
    float free_position_above, free_position_below;
    float acceleration;
    float half_step_position, half_step_speed, half_step_acceleration;

    float pulsation_1_squared = s_above.rate / p.mass;
    float pulsation_2_squared = s_below.rate / p.mass;

    free_position_above = s_presence & ABOVE_ABSENT ? 0: p_above.position - s_above.free_length;
    free_position_below = s_presence & BELOW_ABSENT ? 0: p_below.position + s_below.free_length;

    acceleration = ((constant / p.mass) * cos(pulsation * time) + pulsation_1_squared * free_position_above + pulsation_2_squared * free_position_below - 9.81) * p.position - (friction / p.mass) * p.speed;

    euler_approximation(p.position, p.speed, acceleration, step_size/2, &half_step_position, &half_step_speed);

    half_step_acceleration = ((constant / p.mass) * cos(pulsation * time) + pulsation_1_squared * free_position_above + pulsation_2_squared * free_position_below - 9.81) * half_step_position - (friction / p.mass) * half_step_speed;

    rk2_approximation(p.position, p.speed, acceleration, half_step_acceleration, step_size, &returned_point.position, &returned_point.speed);
    returned_point.mass = p.mass;

    return returned_point;
}
