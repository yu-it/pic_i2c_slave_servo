//#include <stdio.h>
//#include <stdlib.h>
//#include <xc.h>
//#include <pic16F1508.h>
//#define	I2C_ADDR	0x12	// I2C 7bit address
//void I2CWrite(unsigned char data){ 
//    while(SSP1STATbits.BF);      //wait while buffer is full 
//    do{ 
//        SSP1CON1bits.WCOL = 0;           //clear write collision flag 
//        SSP1BUF = data; 
//    }while (SSP1CON1bits.WCOL);           //do until write collision flag is clear 
//    if (SSP1CON2bits.SEN) SSP1CON1bits.CKP = 1;           //release the SCL line 
//} 
//
//
//void setUpI2CSlave()
//{
//
//// start I2C(1) setup   ：I2C(1) is slave
//    SSP1CON2 = 0b00000001;  //SEN is set to enable clock stretching 
//	
//	SSP1CON3 = 0x00; 
//    SSP1ADD = I2C_ADDR<<1;      //7-bit address is stored in the 7 MSB's of the SSP1ADD register**********
//    SSP1STAT = 0x00; 
//    SSP1CON1 = 0b00110110;      
//    /* Enable interrupts */ 
//    SSP1IF  = 0;   //Clear MSSP interrupt flag 
//    SSP1IE  = 1;   //I2C interrupt enable 
//    
//}
////I2C
//#define RXBUFFER_SIZE 4 
//
//volatile unsigned char RXBuffer[RXBUFFER_SIZE]; 
//volatile unsigned char RXBufferIndex = 0;
//
//#define STATE1      0b00001001 // 0x09 master write last was address
//#define STATE2      0b00101001 // 0x29 master write last was data
//#define STATE3      0b00001101 // 0x0d master read last was address
//#define STATE4      0b00101100 // 0x2c master write last was data
//#define STATE5      0b00101000 // 0x28
//
//void checkStatAndMngI2c()
//{
//    static  char    DAStatus=0;
//    unsigned char i2cStatus, value; 
//    i2cStatus = SSP1STAT;
//        
//    i2cStatus = (i2cStatus & 0b00101101);    //Mask out unimportant bits 
//                 // _, _, D/A, _, S, R/W, _, BF 
//    switch (i2cStatus){
//        case STATE1:                              
//            value = SSP1BUF;         //read buffer, clear BF 
//
//            RXBufferIndex = 0;          //clear index 
//            DAStatus=1;                 // next call is address inside memory
//
//            if(SSP1CON1bits.SSPOV)SSP1CON1bits.SSPOV = 0;         //clear receive overflow indicator
//            if (SSP1CON2bits.SEN) SSP1CON1bits.CKP = 1;           //release the SCL line 
//            break; 
//
//        case STATE2:                      
//            value = SSP1BUF;                    //read buffer, clear BF
//            
//            if (DAStatus==1){
//                RXBufferIndex=value;
//                DAStatus=2;
//            }
//            else{
//               RXBuffer[RXBufferIndex] = value;
//               RXBufferIndex++;                    //increment index 
//               if (RXBufferIndex>=RXBUFFER_SIZE) 
//                    RXBufferIndex = 0; 
//            }
//
//            if (SSP1CON2bits.SEN) SSP1CON1bits.CKP = 1;           //release the SCL line 
//            break; 
//
//        case STATE3:         
//            value = SSP1BUF;         //dummy read
//
//            if (RXBufferIndex>=RXBUFFER_SIZE) 
//                RXBufferIndex = 0; 
//
//            I2CWrite(RXBuffer[RXBufferIndex]);    //write back the index of status requested 
//            RXBufferIndex++; 
//            break; 
//
//        case STATE4:            
//            if(RXBufferIndex>=RXBUFFER_SIZE) 
//                RXBufferIndex = 0; 
//
//            I2CWrite(RXBuffer[RXBufferIndex]);    //write back the index of status requested 
//            RXBufferIndex++; 
//            break; 
//
//        case STATE5:           
//            break; 
//        default: 
//            if (SSP1CON2bits.SEN) SSP1CON1bits.CKP = 1;           //release the SCL line 
//            break; 
//    }//end switch (i2cStatus) 
//}
//unsigned int testflg;
//int I2Cinterrupt()
//{
//    int ssp1if;
//    ssp1if = SSP1IF;   // PERIPHERAL INTERRUPT REQUEST REGISTER 1
//    if (ssp1if){
//        if (testflg) {
//            testflg = 0;
//            PORTBbits.RB5 = 0;
//        } else {
//            testflg = 1;
//            PORTBbits.RB5 = 1;
//            
//        }
//        SSP1IF = 0;      //Clear interrupt flag 
//        checkStatAndMngI2c();
//    }
//    return  ssp1if;
//}
//
