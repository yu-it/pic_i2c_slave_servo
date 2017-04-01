/* 
 * File:   main.c
 * Author: yuusuke.ito
 *
 * Created on 2017/04/01, 11:25
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <pic16F1508.h>


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
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
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
unsigned int HWidth, VWidth, DFlag, Mode, i, testflg;
unsigned int Interval, AutoFlag, AutoInt, Level, NewLvl;
const unsigned int MAX = 1000;
const unsigned int MIN = 250;
//#define  _XTAL_FREQ     8000000      // クロック周波数500kHz
unsigned int current_1, current_4;


void CLCInit();
void Init();
/*
 * 
 */
void Init() {
    OSCCON = 0x73;                      // 内蔵8MHz
    /** 入出力ポートの設定 ***/
    ANSELA = 0x0;                      // AN5,AN6,AN7のみアナログ
    ANSELB = 0x00;                      // デジタル    
    TRISA = 0;                       // RA0,1,3,4,5のみ入力設定
    TRISAbits.TRISA0 = 1;
    TRISB = 0;                       // すべて出力
    //TRISC = 0x2E;                       // RC1,2,3,5のみ入力
    TRISC = 0;                       // RC1,2,3,5のみ入力
    /** プルアップイネーブル **/
    WPUA = 0b00000000;                        // RA4,5 pullup   
    WPUAbits.WPUA = 1;                        // RA4,5 pullup   
    /* タイマ0の設定　20.04msec周期 */
    OPTION_REG = 0x07;                  // Int. 1/256, プルアップ有効化
    TMR0 = 99;                          // 20msec         
    /* PWM1,4の初期設定 2.048msec周期*/
    /* Duty値設定範囲  450 to 1023 (0.9ms to 2.05ms width) */
    HWidth = 750;                       // 初期値1.5msec
    VWidth = 750;
    PWM1CON = 0x80;                     // PWM1オン Output無効
    PWM1DCH = HWidth >> 2;
    PWM1DCL = HWidth << 6;
    PWM4CON = 0x80;                     // PWM4オン Output無効
    PWM4DCH = VWidth >> 2;
    PWM4DCL = VWidth << 6;
    /* タイマ２の設定 */
    T2CON = 0x06;                       // 1/16  2MHz/16/256 = 2.048msec周期
    PR2 = 0xFF;                         // 10bit分解能
    /** CLCの初期設定  **/
    CLCInit();
     /* 変数リセット */
    Interval = 5;                       // 0.1sec
    AutoInt = 300;                      // 自動周期 30sec
    DFlag = 1;                          // 表示フラグセット
    Mode = 0;                           // 動作モード初期化 Manual
    AutoFlag = 0;                       // 自動モードフラグ
    printf("** Start App! **");         // 開始メッセージ表示
    /* 割り込み許可 */
    TMR0IE = 1;                         // タイマ0割り込み許可
    //TMR2IE = 1;                         // タイマ0割り込み許可
    PEIE = 1;                           // 周辺許可
    GIE = 1;                            // グローバル許可

}

int main(int argc, char** argv) {

    Init();
    int count = 0;
    int speed = 500;
    current_1 = 0;
    current_4 = 0;
    PORTBbits.RB4 = 1;
    PORTBbits.RB5 = 1;
    current_1 = (MAX + MIN) / 2;
    PWM1DCH = current_1 >> 2;                  // PWM1
    PWM1DCL = current_1 << 6;
    PWM4DCH = current_1 >> 2;                  // PWM4
    PWM4DCL = current_1 << 6;
    while (1) {
        if (RA0 == 1) {
            if (current_1 > MIN) {
                if (count > speed) {
                    current_1 = current_1 - 1;
                    PWM1DCH = current_1 >> 2;                  // PWM1
                    PWM1DCL = current_1 << 6;
                    PWM4DCH = current_1 >> 2;                  // PWM4
                    PWM4DCL = current_1 << 6;
                    PORTBbits.RB5 = 1;
                    count = 0;
                } else {
                    count ++;
                }
            } else {
                PORTBbits.RB5 = 0;
            }
        } else {
            if (current_1 < MAX) {
                if (count > speed) {
                    current_1 = current_1 + 1;
                    PWM1DCH = current_1 >> 2;                  // PWM1
                    PWM1DCL = current_1 << 6;
                    PWM4DCH = current_1 >> 2;                  // PWM4
                    PWM4DCL = current_1 << 6;
                    PORTBbits.RB4 = 1;
                    count = 0;
                } else {
                    count ++;
                }
            } else {
                PORTBbits.RB4 = 0;
                
            }

        }
    }
}

void Init_test(){
    ANSELA = 0;
    ANSELC = 0;
    
    WPUA = 0;
    TRISA = 0;
    TRISC = 0;
    
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    //TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA4 = 1;
    TRISAbits.TRISA5 = 1;
    
    OPTION_REG = 0b00000000 ; // デジタルI/Oに内部プルアップ抵抗を使用する
    ANSELB = 0;
    TRISB      = 0b01010000 ; // ピン(RB)はRB4(SCL1)/RB1(SDA1)のみ入力
    WPUB       = 0b01010000 ; // RB1/4は内部プルアップ抵抗を指定する
    PORTB      = 0b00000000 ; // RB出力ピンの初期化(全てLOWにする)

    /* 入出力ポートの設定 */
    ANSELA = 0b0;                  // すべてデジタルピン
    TRISA = 0b0;                   // RD2のみ入力
    TRISAbits.TRISA0 = 1;                   // RA0のみ入力
    /**** メインループ ********/
    while(1) {
        if(RA0 == 1){               // S1がオフの場合
            PORTAbits.RA1=1;           // D2,3,4点灯
            PORTAbits.RA1=0;           // D2,3,4点灯
        }
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
        if (testflg) {
            testflg = 0;
            PORTBbits.RB6 = 0;
        } else {
            testflg = 1;
            PORTBbits.RB6 = 1;
            
        }
        TMR0IF = 0;                         // フラグクリア
        TMR0 = 96;                          // 時間再設定
        Interval--;                         // インターバル更新
        if(Interval == 0){                  // 設定時間の場合
            Interval = 5;                   // 0.1secに再セット
            DFlag = 1;                      // 表示フラグオン          
            AutoInt--;                      // 10secタイマ更新
            if(AutoInt == 0){               // 10secか？
                AutoInt = 300;              // タイマ再設定30sec
                AutoFlag = 1;               // 自動モード開始フラグオン
            }
        }
    }
}
