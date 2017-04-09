/* 
 * File:   main.c
 * Author: yuusuke.ito
 *
 * 
 * 
 * 
 * 1  --VDD         |  20 VSS
 * 2  --RA5         |  19 RA0
 * 3  --RA4         |  18 RA1
 * 4  --RA3         |  17 RA2 pwm3
 * 5  --RC5 mor1_1  |  16 RC0 srv2
 * 6  --RC4 srv1    |  15 RC1
 * 7  --RC3 pwm3    |  14 RC2 sens1
 * 8  --RC6 mor1_2  |  13 RB4 SDA
 * 9  --RC7 mor2_1  |  12 RB5 sens2
 * 10 --RB7 mor2_2  |  11 RB6 SCL
 * 
 * 17はモータのパワーコントローラ * 
 * i2cset -y 1 [ADD] [mem_offset] [command] [value] [ex-value]
 * i2cset -y 1 0x12 0x00 0x0a 0x11
 * Created on 2017/04/01, 11:25
 */
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <pic16F1508.h>
#include "main.h"
/***** コンフィギュレーションの設定 *********/
/*
__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_ON & MCLRE_ON & CP_OFF
	& BOREN_ON & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
__CONFIG(WRT_OFF & STVREN_OFF & LVP_OFF);
*/
// コンフィギュレーション１の設定
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset) aa
#pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)aaa
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled) zzz

/** グローバル定数変数定義 **/
unsigned int HWidth, VWidth, DFlag, Mode, i;
unsigned int Interval, AutoFlag, AutoInt, Level, NewLvl;
const unsigned int MAX = 1000;
const unsigned int MIN = 250;
//#define  _XTAL_FREQ     8000000      // クロック周波数500kHz
unsigned int current_1, current_4;


/*volatile*/ int arm_mode = 0;    
/*volatile*/ mechanical_characteristic mech_char;
/*volatile*/ mortion_delta mor_delta;
/*volatile*/ current_status cur_stat;


/*
 * 
 */

void Init() {
    OSCCON = 0x73;                      // 内蔵8MHz
    /** 入出力ポートの設定 ***/
    ANSELA = 0x00;
    ANSELB = 0x00;                      
    ANSELC = 0x00;                      
    TRISA = 0;                       // すべて出力
    TRISB = 0;                       // すべて一旦出力
    TRISC = 0;                       // すべて出力
    
    TRISCbits.TRISC2 = 1;            //センサー1
    TRISBbits.TRISB5 = 1;            //センサー2

    /** プルアップイネーブル **/
    WPUA = 0b0;
    /* タイマ0の設定　20.04msec周期 */
    OPTION_REG = 0x07;                  // Int. 1/256, プルアップ有効化
    TMR0 = 99;                          // 20msec         
    /* PWM1,4の初期設定 2.048msec周期*/
    /* Duty値設定範囲  450 to 1023 (0.9ms to 2.05ms width) */
    
    PWM1CON = 0x80;                     // PWM1オン Output無効
    PWM4CON = 0x80;                     // PWM4オン Output無効
    //PWM2CON = 0xC0;                     // PWM3オン OutputOn

    /* タイマ２の設定 */
    T2CON = 0x06;                       // 1/16  2MHz/16/256 = 2.048msec周期
    PR2 = 0xFF;                         // 10bit分解能
    /** CLCの初期設定  **/
    CLCInit();
     /* 変数リセット */
    /* 割り込み許可 */
    TMR0IE = 1;                         // タイマ0割り込み許可
    //TMR2IE = 1;                         // タイマ0割り込み許可
    PEIE = 1;                           // 周辺許可
    GIE = 1;                            // グローバル許可
    setUpI2CSlave();
}
void init_struct() {
    mech_char.servo1_max = 0;
    mech_char.servo1_mid = 0;
    mech_char.servo1_min = 0;
    mech_char.servo2_max = 0;
    mech_char.servo2_mid = 0;
    mech_char.servo2_min = 0;
    mor_delta.servo1_dir = 2;
    mor_delta.servo2_dir = 2;
    mor_delta.servo1_pow = 0;
    mor_delta.servo2_pow = 0;
    mor_delta.mor1_dir = 2;
    mor_delta.mor2_dir = 2;
    mor_delta.mor_pow = 500;
    cur_stat.servo1_angle = 0;
    cur_stat.servo2_angle = 0;
    cur_stat.mor1_dir = 2;
    cur_stat.mor2_dir = 2;
    cur_stat.mor_pow = 500;
    
}
int main(int argc, char** argv) {

    Init();
    init_struct();
    while(1) {
        
        if (arm_mode) {
            optimize_arm_angle();
        }
        apply_delta2status();
        
        apply_status2mech();
        
    }
}

/*************************************
* CLC初期設定関数
*************************************/
void CLCInit(void){
   /* Timer0の20msec周期でPWM3とPWM4の出力を1回のみ有効化 */
    /* CLC1 初期設定 */
    CLC1GLS0 = 0x08;
    CLC1GLS1 = 0x00;                    // RSフリップフロップ
    CLC1GLS2 = 0x02;                    // Timer0でセット
    CLC1GLS3 = 0x00;                    // Timer2でリセット
    CLC1SEL0 = 0x17;
    CLC1SEL1 = 0x60;
    CLC1POL  = 0x00;
    CLC1CON  = 0xC3;
    /* CLC2 初期設定 */
    CLC2GLS0 = 0x08;
    CLC2GLS1 = 0x20;                    // AND回路
    CLC2GLS2 = 0x00;                    // CLC3出力とPWM1出力のAND
    CLC2GLS3 = 0x00;
    CLC2SEL0 = 0x62;
    CLC2SEL1 = 0x66;
    CLC2POL  = 0x0C;
    CLC2CON  = 0xC2;
    /* CLC3 初期設定 */
    CLC3GLS0 = 0x02;
    CLC3GLS1 = 0x08;                    // Dタイプフリップフロップ
    CLC3GLS2 = 0x00;                    // CLC1の出力をD入力
    CLC3GLS3 = 0x00;                    // Timer2がクロック
    CLC3SEL0 = 0x47;
    CLC3SEL1 = 0x50;
    CLC3POL  = 0x00;
    CLC3CON  = 0x84;
    /* CLC4 初期設定 */
    CLC4GLS0 = 0x08;
    CLC4GLS1 = 0x80;                    // AND回路
    CLC4GLS2 = 0x00;                    // CLC3出力とPWM4出力のAND
    CLC4GLS3 = 0x00;
    CLC4SEL0 = 0x62;
    CLC4SEL1 = 0x37;
    CLC4POL  = 0x0C;
    CLC4CON  = 0xC2;
}

void interrupt isr(void){
    if(TMR0IF){                             // タイマ0割り込みか？
        TMR0IF = 0;                         // フラグクリア
        TMR0 = 96;                          // 時間再設定
    }
    I2Cinterrupt();
    //PORTC6
    //PORTC7
    
}

void i2c_handler_impl(unsigned int com, unsigned int data) {

         if (com == set_servo1min_com) {
             set_servo1min(data);
         } else if (com == set_servo1max_com) {
             set_servo1max(data);
         } else if (com == set_servo2min_com) {
             set_servo2min(data);
         } else if (com == set_servo2max_com) {
             set_servo2max(data);
         } else if (com == set_servo1dir_com) {
             set_servo1dir(data);
         } else if (com == set_servo2dir_com) {
             set_servo2dir(data);
         } else if (com == set_servo1pow_com) {
             set_servo1pow(data);
         } else if (com == set_servo2pow_com) {
             set_servo2pow(data);
         } else if (com == set_mor1dir_com) {
             set_mor1dir(data);
         } else if (com == set_mor2dir_com) {
//             if (1 == data) {
//                 LATAbits.LATA5 = 1;
//             }
             set_mor2dir(data);
         } else if (com == set_mor_pow_com) {
             set_mor_pow(data);
         } else if (com == set_arm_mode_com) {
             set_arm_mode(data);
         }

     /*
     switch(com) {
         case set_servo1min_com:
             set_servo1min(data);
             break;
         case set_servo1max_com:
             set_servo1max(data);
             break;
         case set_servo2min_com:
             set_servo2min(data);
             break;
         case set_servo2max_com:
             set_servo2max(data);
             break;
         case set_servo1dir_com:
             set_servo1dir(data);
             break;
         case set_servo2dir_com:
             set_servo2dir(data);
             break;
         case set_servo1pow_com:
             set_servo1pow(data);
             break;
         case set_servo2pow_com:
             set_servo2pow(data);
             break;
         case set_mor1dir_com:
             set_mor1dir(data);
             break;
         case set_mor2dir_com:
             set_mor2dir(data);
             break;
         case set_mor_pow_com:
             set_mor_pow(data);
             break;
         case set_arm_mode_com:
             set_arm_mode(data);
             break;

     }
    */         
 }
void optimize_arm_angle(){};
int tmp_counter_1 = 0;
int tmp_counter_2 = 0;
void apply_delta2status(){
    //mor_delta
    //cur_stat
    tmp_counter_1 ++;
    int tmp_new_angle = 0;
    if (tmp_counter_1 > mor_delta.servo1_pow) {
        tmp_counter_1 = 0;
        if ((mor_delta.servo1_dir - 2) == 0) {
            cur_stat.servo1_angle = 0;
            
        } else {
            tmp_new_angle = cur_stat.servo1_angle + ((mor_delta.servo1_dir - 2));
            if ( mech_char.servo1_min > tmp_new_angle) {
                cur_stat.servo1_angle = mech_char.servo1_min;
                mor_delta.servo1_dir = 2;
            } else if (tmp_new_angle > mech_char.servo1_max) {
                cur_stat.servo1_angle = mech_char.servo1_max;
                mor_delta.servo1_dir = 2;
            } else {
                cur_stat.servo1_angle = tmp_new_angle;
            }
        }
        //servo1
    }

    //servo2
    if (tmp_counter_2 > mor_delta.servo1_pow) {
        tmp_counter_2 = 0;
        if ((mor_delta.servo2_dir - 2) == 0) {
            cur_stat.servo2_angle = 0;
            
        } else {
            tmp_new_angle = cur_stat.servo2_angle + ((mor_delta.servo2_dir - 2));
            if ( mech_char.servo2_min > tmp_new_angle) {
                cur_stat.servo2_angle = mech_char.servo2_min;
                mor_delta.servo2_dir = 2;
            } else if (tmp_new_angle > mech_char.servo2_max) {
                cur_stat.servo2_angle = mech_char.servo2_max;
                mor_delta.servo2_dir = 2;
            } else {
                cur_stat.servo2_angle = tmp_new_angle;
            }
        }
        //servo1
    }
    cur_stat.mor1_dir = mor_delta.mor1_dir;
    cur_stat.mor2_dir = mor_delta.mor2_dir;
    cur_stat.mor_pow = mor_delta.mor_pow;
    
};
void apply_status2mech(){
    PWM1DCH = cur_stat.servo1_angle >> 2;
    PWM1DCL = cur_stat.servo1_angle << 6;
    PWM4DCH = cur_stat.servo2_angle >> 2;
    PWM4DCL = cur_stat.servo2_angle << 6;
//    PWM2DCH = cur_stat.mor_pow >> 2;      //やっぱメインループのパルスで制御
//    PWM2DCL = cur_stat.mor_pow << 6;     //やっぱメインループのパルスで制御
    
    if (3 == cur_stat.mor1_dir) {
        LATCbits.LATC5 = 1;
        LATCbits.LATC6 = 0;
    } else if (1 == cur_stat.mor1_dir) {
        LATCbits.LATC5 = 0;
        LATCbits.LATC6 = 1;
    } else if (2 == cur_stat.mor1_dir) {
        LATCbits.LATC5 = 1;
        LATCbits.LATC6 = 1;
    }
        
    if (3 == cur_stat.mor2_dir) {
        LATBbits.LATB7 = 1;
        LATCbits.LATC7 = 0;
    } else if (1 == cur_stat.mor2_dir) {
        LATBbits.LATB7 = 0;
        LATCbits.LATC7 = 1;
    } else if (2 == cur_stat.mor2_dir) {
        LATBbits.LATB7 = 1;
        LATCbits.LATC7 = 1;
    }

};



