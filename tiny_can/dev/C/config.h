#ifndef __CONFIG_H__
#define __CONFIG_H__


#define COM_MODE USB             // USB = USB Schnittstelle verwenden
                                 // SERIELL = RS232 Schnittstelle verwenden

/****************************/
/* NUR BEI COM_MODE USB     */
/****************************/
//#define HW_SNR "01000000"

/****************************/
/* NUR BEI COM_MODE SERIELL */
/****************************/

#define COM_PORT COM1            // COM Port 1 - 4

#define BAUD_RATE 57600          // Baudrate
                                 //   4800 Baud
                                 //   9600 Baud
                                 //   14400 Baud
                                 //   19200 Baud
                                 //   28800 Baud
                                 //   38400 Baud
                                 //   57600 Baud
                                 //   115200 Baud
                                 //   10400 Baud
                                 //   125000 Baud
                                 //   153600 Baud
                                 //   230400 Baud
                                 //   250000 Baud
                                 //   460800 Baud
                                 //   500000 Baud
                                 //   921600 Baud
                                 //   1 M Baud

#define CAN_SPEED CAN_125K_BIT   // CAN_10K_BIT  => 10 kBit/s
                                 // CAN_20K_BIT  => 20 kBit/s
                                 // CAN_50K_BIT  => 50 kBit/s
                                 // CAN_100K_BIT => 100 kBit/s
                                 // CAN_125K_BIT => 125 kBit/s
                                 // CAN_250K_BIT => 250 kBit/s
                                 // CAN_500K_BIT => 500 kBit/s
                                 // CAN_800K_BIT => 800 kBit/s
                                 // CAN_1M_BIT   => 1 MBit/s
                                 // CAN_1M5_BIT  => 1,5 MBit/s
                                 // CAN_2M_BIT   => 2 MBit/s
                                 // CAN_3M_BIT   => 3 MBit/s
                                 // CAN_4M_BIT   => 4 MBit/s
//#define CAN_SPEED_BTR_VALUE
#define CAN_DATA_SPEED        CAN_1M_BIT
//#define CAN_SPEED_DBTR_VALUE


//#define PUBLIC_INIT_STR ";LogFile=log.txt;LogFlags=0xC80001EF"


#endif
