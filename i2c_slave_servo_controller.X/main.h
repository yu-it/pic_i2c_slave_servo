/* 
 * File:   main.h
 * Author: yuusuke.ito
 *
 * Created on 2017/04/08, 15:13
 */

#include "I2C.h"
#ifndef MAIN_H
#define	MAIN_H
typedef struct {
    unsigned int servo1_max;
    unsigned int servo1_mid;
    unsigned int servo1_min;
    unsigned int servo2_max;
    unsigned int servo2_mid;
    unsigned int servo2_min;
} mechanical_characteristic;
extern /*volatile*/ mechanical_characteristic mech_char;
typedef struct {
    unsigned int servo1_dir;
    unsigned int servo2_dir;
    unsigned int servo1_pow;
    unsigned int servo2_pow;
    unsigned int mor1_dir;
    unsigned int mor2_dir;
    unsigned int mor_pow;

} mortion_delta;
extern /*volatile*/ mortion_delta mor_delta;
typedef struct  {
    unsigned int servo1_angle;
    unsigned int servo2_angle;
    unsigned int mor1_dir;
    unsigned int mor2_dir;
    unsigned int mor_pow;
} current_status;
extern /*volatile*/ current_status cur_stat;

#define as_signed_flg(V) (V == 0 ? 0 : V - 2)

#define set_servo1min(V) mech_char.servo1_min = V;mech_char.servo1_mid = (mech_char.servo1_min + mech_char.servo1_max) / 2;if(cur_stat.servo1_angle < V) {cur_stat.servo1_angle = V;}
#define set_servo1min_com 1
#define set_servo1max(V) mech_char.servo1_max = V;mech_char.servo1_mid = (mech_char.servo1_min + mech_char.servo1_max) / 2;if(cur_stat.servo1_angle > V) {cur_stat.servo1_angle = V;}
#define set_servo1max_com 2

#define set_servo2min(V) mech_char.servo2_min = V;mech_char.servo2_mid = (mech_char.servo2_min + mech_char.servo2_max) / 2;if(cur_stat.servo2_angle < V) {cur_stat.servo2_angle = V;}
#define set_servo2min_com 3
#define set_servo2max(V) mech_char.servo2_max = V;mech_char.servo2_mid = (mech_char.servo2_min + mech_char.servo2_max) / 2;if(cur_stat.servo2_angle > V) {cur_stat.servo2_angle = V;}
#define set_servo2max_com 4

#define set_servo1dir(V) mor_delta.servo1_dir = V;
#define set_servo1dir_com 5
#define set_servo2dir(V) mor_delta.servo2_dir = V;
#define set_servo2dir_com 6

#define set_servo1pow(V) mor_delta.servo1_pow = V;
#define set_servo1pow_com 7
#define set_servo2pow(V) mor_delta.servo2_pow = V;
#define set_servo2pow_com 8

    
#define set_mor1dir(V) mor_delta.mor1_dir = V;
#define set_mor1dir_com 9
#define set_mor2dir(V) mor_delta.mor2_dir = V;
#define set_mor2dir_com 10
#define set_mor_pow(V) mor_delta.mor_pow = V;
#define set_mor_pow_com 11
     
#define set_arm_mode(V) arm_mode = V;
#define set_arm_mode_com 12
extern /*volatile*/ int arm_mode;


void CLCInit();
void Init();
void optimize_arm_angle();
void apply_delta2status();
void apply_status2mech();
void apply_status2mech();
void i2c_handler_impl(unsigned int com, unsigned int data);
void init_struct();
void (*i2c_handler)(unsigned, unsigned) = i2c_handler_impl;

#ifdef	__cplusplus

extern "C" {
#endif
    
    


     
#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

