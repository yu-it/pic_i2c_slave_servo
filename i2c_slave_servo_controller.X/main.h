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
        unsigned B0                 :1;
        unsigned B1                 :1;
        unsigned B2                 :1;
        unsigned B3                 :1;
        unsigned B4                 :1;
        unsigned B5                 :1;
        unsigned B6                 :1;
        unsigned B7                 :1;
    
} BitIndicator;
typedef struct {
    unsigned int dir;
    unsigned int power;
    volatile unsigned char* PWML_register_for_power_control;
    volatile unsigned char* PWMH_register_for_power_control;
    volatile unsigned char* LAT_register_for_dir_control1;
    unsigned int LAT_register_for_dir_control1_offset;
    volatile unsigned char* LAT_register_for_dir_control2;
    unsigned int LAT_register_for_dir_control2_offset;
} mortor_descriptor;
typedef struct {
    unsigned int max_frequency;
    unsigned int min_frequency;
    unsigned int dir;
    unsigned int power;
    unsigned int current_freq;
    unsigned int actual_outputting_freq;
    volatile unsigned char* PWML_register;
    volatile unsigned char* PWMH_register;
    unsigned int power_control_work;
    
} servo_descriptor;

#define as_signed_flg(V) (V == 0 ? 0 : V - 2)

/*
 * set_register_flg 
 * レジスタのフラグを設定する
 * →建てたいフラグ以外の値も巻き込む処理になってしまっている
void set_register_flg2(unsigned char* regaddr, unsigned int regoffset, unsigned int val) {
    //成功
    int test1 = 2;
    char test2 = 3;
    char reg_value = *regaddr;
    char selector = (0xff - (1 << regoffset));
    char target_to_val = (val << regoffset);
    char selected_value = reg_value & selector;
    char desired_val = selected_value | target_to_val;
    *regaddr =  desired_val;
}   
 */
#define set_servo1min_com 1
#define set_servo1max_com 2
#define set_servo2min_com 3
#define set_servo2max_com 4
#define set_servo1dir_com 5
#define set_servo2dir_com 6
#define set_servo1pow_com 7
#define set_servo2pow_com 8
#define set_mor1dir_com 9
#define set_mor2dir_com 10
#define set_mor_pow_com 11
#define set_arm_mode(V) arm_mode = V;
#define set_arm_mode_com 12

#define set_arm_mode(V) arm_mode = V;
#define set_arm_mode_com 12
extern /*volatile*/ int arm_mode;


void CLCInit();
void Init();
void optimize_arm_angle();
void i2c_handler_impl(unsigned int com, unsigned int data);
void init_struct();
void (*i2c_handler)(unsigned, unsigned) = i2c_handler_impl;
void calc_desired_status();
void apply_desired_status();


#ifdef	__cplusplus

extern "C" {
#endif
    
    


     
#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */
