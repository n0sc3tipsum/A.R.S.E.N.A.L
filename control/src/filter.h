#ifndef FILTER_H
#define FILTER_H
extern float dt;

float CompFilter(float accel, float gyro, float coeff, float prev){
    return coeff * (prev + gyro * dt) + (1-coeff) *accel;
}

float LPF(float current, float prev, float AlphaLPF){
    return AlphaLPF * current + (1- AlphaLPF) * prev;
}



#endif