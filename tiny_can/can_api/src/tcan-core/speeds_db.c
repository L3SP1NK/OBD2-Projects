/***************************************************************************
                        speeds_db.c  -  description
                             -------------------
    begin             : 10.09.2022
    last modify       : 10.09.2022
    copyright         : (C) 2022 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include "util.h"
#include "speeds_db.h"

  
/*********************************************************************************/
/*                                   CAN                                         */
/*********************************************************************************/
const struct TSpeedDesc DefSpeedTab[] = {
  /********************** Clock: 48 MHz ****************************/
  {  10,  48, 0x8A3E80EF, 0x3A13D01D}, //    10 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  {  20,  48, 0x8A3E80EF, 0x3A09D01D}, //    20 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  {  50,  48, 0x8A3E80EF, 0x3A03D01D}, //    50 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  { 100,  48, 0x8B3E80EF, 0x3A01D01D}, //   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  { 125,  48, 0x8B3E80BF, 0x2E01A617}, //   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
  { 250,  48, 0x8B3E80BF, 0x2E00A617}, //   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
  { 500,  48, 0x893E605F, 0x24004B12}, //   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
  { 800,  48, 0x8B3E803B, 0x1C002B0E}, //   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
  {1000,  48, 0x8B3E802F, 0x1600220B}, //     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
  /********************** Clock: 80 MHz ****************************/
  {  10,  80, 0x0A3E80C7, 0x3027AD18}, //    10 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 // <*> Ranking 0x0 Bitrate Error ? prüfen
  {  20,  80, 0x8A3E80C7, 0x3013AD18}, //    20 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  50,  80, 0x8A3E80C7, 0x3007AD18}, //    50 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  { 100,  80, 0x8B3E80C7, 0x3003AD18}, //   100 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  { 125,  80, 0x8A3E809F, 0x26038A13}, //   125 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 250,  80, 0x8A3E809F, 0x26018A13}, //   250 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 500,  80, 0x8B3E809F, 0x3E007E1F}, //   500 kBit/s Clock:80MHz  PH1=127  PH2=32   SP=80.0%  SJW=32 
  { 800,  80, 0x8B3E8063, 0x30004918}, //   800 kBit/s Clock:80MHz  PH1=74   PH2=25   SP=75.0%  SJW=25 
  {1000,  80, 0x8B3E804F, 0x26003A13}, //     1 MBit/s Clock:80MHz  PH1=59   PH2=20   SP=75.0%  SJW=20 
  /********************** Clock: 96 MHz ****************************/
  {  10,  96, 0x0A3E80EF, 0x3A27D01D}, //    10 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  {  20,  96, 0x8A3E80EF, 0x3A13D01D}, //    20 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  {  50,  96, 0x8A3E80EF, 0x3A07D01D}, //    50 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  { 100,  96, 0x8A3E80EF, 0x3A03D01D}, //   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  { 125,  96, 0x8B3E80FF, 0x3E02DE1F}, //   125 kBit/s Clock:96MHz  PH1=223  PH2=32   SP=87.5%  SJW=32 
  { 250,  96, 0x8B3E80BF, 0x2E01A617}, //   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
  { 500,  96, 0x893E60BF, 0x4A009825}, //   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
  { 800,  96, 0x8B3E8077, 0x3A00581D}, //   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
  {1000,  96, 0x8B3E805F, 0x2E004617}, //     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
  /********************** Clock: 40 MHz ****************************/
  {  10,  40, 0x8A3E80C7, 0x3013AD18}, //    10 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  20,  40, 0x8A3E80C7, 0x3009AD18}, //    20 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  50,  40, 0x8B3E80C7, 0x3003AD18}, //    50 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  { 100,  40, 0x8B3E80C7, 0x3001AD18}, //   100 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  { 125,  40, 0x8A3E809F, 0x26018A13}, //   125 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 250,  40, 0x8B3E809F, 0x26008A13}, //   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 500,  40, 0x8B3E804F, 0x1E003E0F}, //   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
  { 800,  40, 0x893DE031, 0x1600240B}, //   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
  {1000,  40, 0x8B3E8027, 0x12001C09}, //     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  /********************** Clock: 16 MHz ****************************/
  {  10,  16, 0x8A3E80C7, 0x3007AD18}, //    10 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  20,  16, 0x8B3E80C7, 0x3003AD18}, //    20 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  50,  16, 0x8A3E809F, 0x26018A13}, //    50 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 100,  16, 0x8B3E809F, 0x26008A13}, //   100 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
  { 125,  16, 0x8B3E807F, 0x1E006E0F}, //   125 kBit/s Clock:16MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
  { 250,  16, 0x8B3E803F, 0x0E003607}, //   250 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
  { 500,  16, 0x893DC01F, 0x0A001805}, //   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  { 800,  16, 0x8B3E8013, 0x08000D04}, //   800 kBit/s Clock:16MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  {1000,  16, 0x8B3E800F, 0x06000A03}, //     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  /********************** Clock: 12 MHz ****************************/
  {  10,  12, 0x8A3E80EF, 0x3A04D01D}, //    10 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  {  20,  12, 0x8A3E80C7, 0x3002AD18}, //    20 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
  {  50,  12, 0x8B3E80EF, 0x3A00D01D}, //    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
  { 100,  12, 0x8B3E8077, 0x1C00670E}, //   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
  { 125,  12, 0x8B3E805F, 0x1600520B}, //   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
  { 250,  12, 0x8B3E802F, 0x0A002805}, //   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
  { 500,  12, 0x893DF017, 0x08001104}, //   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
  { 800,  12, 0x893D700E, 0x06000903}, //   800 kBit/s Clock:12MHz  PH1=10   PH2=4    SP=73.3%  SJW=4  
  {1000,  12, 0x8B3E800B, 0x04000702}, //     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {0, 0, 0, 0}};

/*********************************************************************************/
/*                                   CAN FD                                      */
/*********************************************************************************/
const struct TFdSpeedDesc DefFdSpeedTab[] = {
  /********************** Clock: 48 MHz ****************************/
  {  10,   125,  48, 0xBEBE80C7, 0x3017AD18, 0x00170C11}, // Nom.:    10 kBit/s Clock:48MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:48MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  10,   250,  48, 0xBEBE80C7, 0x3017AD18, 0x00170500}, // Nom.:    10 kBit/s Clock:48MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:48MHz  PH1=6    PH2=1    SP=87.5%  SJW=1  
  {  10,   500,  48, 0xBD3D512B, 0x540FFF2A, 0x000F0300}, // Nom.:    10 kBit/s Clock:48MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {  20,   125,  48, 0xBFBE80C7, 0x300BAD18, 0x000B1A33}, // Nom.:    20 kBit/s Clock:48MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  48, 0xBEBE80C7, 0x300BAD18, 0x000B0C11}, // Nom.:    20 kBit/s Clock:48MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:48MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  48, 0xBD3D512B, 0x5407FF2A, 0x00070811}, // Nom.:    20 kBit/s Clock:48MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=9    PH2=2    SP=83.3%  SJW=2  
  {  20,  1000,  48, 0xBEBE80C7, 0x300BAD18, 0x000B0100}, // Nom.:    20 kBit/s Clock:48MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  20,  1500,  48, 0xBDBD512B, 0x5407FF2A, 0x00070100}, // Nom.:    20 kBit/s Clock:48MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  48, 0xBFBE804F, 0x120B4409, 0x000B1A33}, // Nom.:    50 kBit/s Clock:48MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  48, 0xBFBE809F, 0x26058A13, 0x00051A33}, // Nom.:    50 kBit/s Clock:48MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   250 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   500,  48, 0xBD3A013F, 0x7C02FF3E, 0x00021855}, // Nom.:    50 kBit/s Clock:48MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  {  50,  1000,  48, 0xBEBE80EF, 0x3A03D01D, 0x00030722}, // Nom.:    50 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {  50,  1500,  48, 0xBFBE80EF, 0x3A03D01D, 0x00030411}, // Nom.:    50 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  48, 0xBEBE809F, 0x26058A13, 0x00050100}, // Nom.:    50 kBit/s Clock:48MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  3000,  48, 0xBFBE80EF, 0x3A03D01D, 0x00030100}, // Nom.:    50 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  4000,  48, 0xBDBA013F, 0x7C02FF3E, 0x00020100}, // Nom.:    50 kBit/s Clock:48MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  48, 0xBBBE8027, 0x080B2104, 0x000B1A33}, // Nom.:   100 kBit/s Clock:48MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  48, 0xBFBE804F, 0x12054409, 0x00051A33}, // Nom.:   100 kBit/s Clock:48MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   500,  48, 0xBD3E80EF, 0x3A01D01D, 0x00011FEE}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 100,  1000,  48, 0xBFBE80EF, 0x3A01D01D, 0x00011055}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 100,  1500,  48, 0xBFBE80EF, 0x3A01D01D, 0x00010A33}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 100,  2000,  48, 0xBFBE80EF, 0x3A01D01D, 0x00010722}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 100,  3000,  48, 0xBFBE80EF, 0x3A01D01D, 0x00010411}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 100,  4000,  48, 0xBEBE809F, 0x26028A13, 0x00020100}, // Nom.:   100 kBit/s Clock:48MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,  6000,  48, 0xBFBE80EF, 0x3A01D01D, 0x00010100}, // Nom.:   100 kBit/s Clock:48MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  48, 0xBBBE803F, 0x0E053607, 0x00051A33}, // Nom.:   125 kBit/s Clock:48MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:   250 kBit/s Clock:48MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 125,   500,  48, 0xBD3E80BF, 0x2E01A617, 0x00011FEE}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 125,  1000,  48, 0xBFBE80BF, 0x2E01A617, 0x00011055}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 125,  1500,  48, 0xBFBE80BF, 0x2E01A617, 0x00010A33}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 125,  2000,  48, 0xBFBE80BF, 0x2E01A617, 0x00010722}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 125,  3000,  48, 0xBFBE80BF, 0x2E01A617, 0x00010411}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  4000,  48, 0xBEBE807F, 0x1E026E0F, 0x00020100}, // Nom.:   125 kBit/s Clock:48MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,  6000,  48, 0xBFBE80BF, 0x2E01A617, 0x00010100}, // Nom.:   125 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 250,   500,  48, 0xBD3E805F, 0x1601520B, 0x00011FEE}, // Nom.:   250 kBit/s Clock:48MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:   500 kBit/s Clock:48MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 250,  1000,  48, 0xBEBE805F, 0x1601520B, 0x00011055}, // Nom.:   250 kBit/s Clock:48MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 250,  1500,  48, 0xBFBE80BF, 0x2E00A617, 0x00001677}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 250,  2000,  48, 0xBFBE80BF, 0x2E00A617, 0x00001055}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 250,  3000,  48, 0xBFBE80BF, 0x2E00A617, 0x00000A33}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 250,  4000,  48, 0xBFBE80BF, 0x2E00A617, 0x00000722}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 250,  6000,  48, 0xBFBE80BF, 0x2E00A617, 0x00000411}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  8000,  48, 0xBD3E80BF, 0x2E00A617, 0x00000300}, // Nom.:   250 kBit/s Clock:48MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     8 MBit/s Clock:48MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 500,  1000,  48, 0xBD3E605F, 0x24004B12, 0x00001FEE}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 500,  1500,  48, 0xBDBE605F, 0x24004B12, 0x00001677}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 500,  2000,  48, 0xBDBE605F, 0x24004B12, 0x00001055}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 500,  3000,  48, 0xBDBE605F, 0x24004B12, 0x00000A33}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 500,  4000,  48, 0xBDBE605F, 0x24004B12, 0x00000722}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 500,  6000,  48, 0xBDBE605F, 0x24004B12, 0x00000411}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 500,  8000,  48, 0xBD3E605F, 0x24004B12, 0x00000300}, // Nom.:   500 kBit/s Clock:48MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     8 MBit/s Clock:48MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 800,  1000,  48, 0xB93E803B, 0x1C002B0E, 0x00001FEE}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     1 MBit/s Clock:48MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 800,  1500,  48, 0xBBBE803B, 0x1C002B0E, 0x00001677}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 800,  2000,  48, 0xBBBE803B, 0x1C002B0E, 0x00001055}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 800,  3000,  48, 0xBBBE803B, 0x1C002B0E, 0x00000A33}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 800,  4000,  48, 0xBBBE803B, 0x1C002B0E, 0x00000722}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 800,  6000,  48, 0xBBBE803B, 0x1C002B0E, 0x00000411}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 800,  8000,  48, 0xB93E803B, 0x1C002B0E, 0x00000300}, // Nom.:   800 kBit/s Clock:48MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:     8 MBit/s Clock:48MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {1000,  1500,  48, 0xBBBE802F, 0x1600220B, 0x00001677}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:   1.5 MBit/s Clock:48MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  {1000,  2000,  48, 0xBBBE802F, 0x1600220B, 0x00001055}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:     2 MBit/s Clock:48MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  {1000,  3000,  48, 0xBBBE802F, 0x1600220B, 0x00000A33}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:     3 MBit/s Clock:48MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  {1000,  4000,  48, 0xBBBE802F, 0x1600220B, 0x00000722}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:     4 MBit/s Clock:48MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {1000,  6000,  48, 0xBBBE802F, 0x1600220B, 0x00000411}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:     6 MBit/s Clock:48MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {1000,  8000,  48, 0xB93E802F, 0x1600220B, 0x00000300}, // Nom.:     1 MBit/s Clock:48MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:     8 MBit/s Clock:48MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  /********************** Clock: 80 MHz ****************************/
  {  10,   125,  80, 0x3D3E70F9, 0x3C1FD91E, 0x001F1011}, // Nom.:    10 kBit/s Clock:80MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   125 kBit/s Clock:80MHz  PH1=17   PH2=2    SP=90.0%  SJW=2  
  {  10,   250,  80, 0x3D3E70F9, 0x3C1FD91E, 0x001F0700}, // Nom.:    10 kBit/s Clock:80MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   250 kBit/s Clock:80MHz  PH1=8    PH2=1    SP=90.0%  SJW=1  
  {  10,   500,  80, 0x3DBE70F9, 0x3C1FD91E, 0x001F0200}, // Nom.:    10 kBit/s Clock:80MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  {  20,   125,  80, 0xBEBE80C7, 0x3013AD18, 0x00131A33}, // Nom.:    20 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:80MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  80, 0xBEBE80C7, 0x3013AD18, 0x00130C11}, // Nom.:    20 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:80MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  80, 0xBDBE70F9, 0x3C0FD91E, 0x000F0611}, // Nom.:    20 kBit/s Clock:80MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  {  20,  1000,  80, 0xBEBE80C7, 0x3013AD18, 0x00130100}, // Nom.:    20 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  80, 0xBEBE804F, 0x12134409, 0x00131A33}, // Nom.:    50 kBit/s Clock:80MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:80MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  80, 0xBEBE809F, 0x26098A13, 0x00091A33}, // Nom.:    50 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   250 kBit/s Clock:80MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   500,  80, 0xBEBE80C7, 0x3007AD18, 0x00070E33}, // Nom.:    50 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=15   PH2=4    SP=80.0%  SJW=4  
  {  50,  1000,  80, 0xBEBE809F, 0x26098A13, 0x00090411}, // Nom.:    50 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  80, 0xBEBE809F, 0x26098A13, 0x00090100}, // Nom.:    50 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  4000,  80, 0xBDBA013F, 0x7C04FF3E, 0x00040100}, // Nom.:    50 kBit/s Clock:80MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  80, 0xBABE8027, 0x08132104, 0x00131A33}, // Nom.:   100 kBit/s Clock:80MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:80MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  80, 0xBEBE804F, 0x12094409, 0x00091A33}, // Nom.:   100 kBit/s Clock:80MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:80MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   500,  80, 0xBFBE80C7, 0x3003AD18, 0x00031E77}, // Nom.:   100 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 100,  1000,  80, 0xBFBE80C7, 0x3003AD18, 0x00030D44}, // Nom.:   100 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 100,  2000,  80, 0xBEBE809F, 0x26048A13, 0x00040411}, // Nom.:   100 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 100,  4000,  80, 0xBEBE809F, 0x26048A13, 0x00040100}, // Nom.:   100 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,  5000,  80, 0xBFBE80C7, 0x3003AD18, 0x00030100}, // Nom.:   100 kBit/s Clock:80MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  80, 0xBD3E804F, 0x12074409, 0x00071F66}, // Nom.:   125 kBit/s Clock:80MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:80MHz  PH1=32   PH2=7    SP=82.5%  SJW=7  
  { 125,   500,  80, 0xBFBE809F, 0x26038A13, 0x00031E77}, // Nom.:   125 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 125,  1000,  80, 0xBEBE809F, 0x26038A13, 0x00030D44}, // Nom.:   125 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 125,  2000,  80, 0xBEBE807F, 0x1E046E0F, 0x00040411}, // Nom.:   125 kBit/s Clock:80MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  4000,  80, 0xBEBE807F, 0x1E046E0F, 0x00040100}, // Nom.:   125 kBit/s Clock:80MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,  5000,  80, 0xBEBE809F, 0x26038A13, 0x00030100}, // Nom.:   125 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,  8000,  80, 0xBD3A013F, 0x7C01FF3E, 0x00010200}, // Nom.:   125 kBit/s Clock:80MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     8 MBit/s Clock:80MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  { 250,   500,  80, 0xBFBE804F, 0x12034409, 0x00031E77}, // Nom.:   250 kBit/s Clock:80MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   500 kBit/s Clock:80MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 250,  1000,  80, 0xBFBE809F, 0x26018A13, 0x00011C99}, // Nom.:   250 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 250,  2000,  80, 0xBEBE809F, 0x26018A13, 0x00010D44}, // Nom.:   250 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 250,  4000,  80, 0xBDBA013F, 0x7C00FF3E, 0x00000D44}, // Nom.:   250 kBit/s Clock:80MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 250,  5000,  80, 0xBEBE809F, 0x26018A13, 0x00010411}, // Nom.:   250 kBit/s Clock:80MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  8000,  80, 0xBD3A013F, 0x7C00FF3E, 0x00000611}, // Nom.:   250 kBit/s Clock:80MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     8 MBit/s Clock:80MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 500,  1000,  80, 0xBFBE804F, 0x1E013E0F, 0x00011C99}, // Nom.:   500 kBit/s Clock:80MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 500,  2000,  80, 0xBFBE809F, 0x3E007E1F, 0x00001C99}, // Nom.:   500 kBit/s Clock:80MHz  PH1=127  PH2=32   SP=80.0%  SJW=32 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 500,  4000,  80, 0xBFBE809F, 0x3E007E1F, 0x00000D44}, // Nom.:   500 kBit/s Clock:80MHz  PH1=127  PH2=32   SP=80.0%  SJW=32 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 500,  5000,  80, 0xBFBE809F, 0x3E007E1F, 0x00000A33}, // Nom.:   500 kBit/s Clock:80MHz  PH1=127  PH2=32   SP=80.0%  SJW=32 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 500,  8000,  80, 0xBD3E809F, 0x3E007E1F, 0x00000611}, // Nom.:   500 kBit/s Clock:80MHz  PH1=127  PH2=32   SP=80.0%  SJW=32 
                                                          //   FD:     8 MBit/s Clock:80MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 800,  1000,  80, 0xB9BDE031, 0x1601240B, 0x00011C99}, // Nom.:   800 kBit/s Clock:80MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     1 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 800,  2000,  80, 0xBFBE8063, 0x30004918, 0x00001C99}, // Nom.:   800 kBit/s Clock:80MHz  PH1=74   PH2=25   SP=75.0%  SJW=25 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 800,  4000,  80, 0xBFBE8063, 0x30004918, 0x00000D44}, // Nom.:   800 kBit/s Clock:80MHz  PH1=74   PH2=25   SP=75.0%  SJW=25 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 800,  5000,  80, 0xBFBE8063, 0x30004918, 0x00000A33}, // Nom.:   800 kBit/s Clock:80MHz  PH1=74   PH2=25   SP=75.0%  SJW=25 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 800,  8000,  80, 0xBD3E8063, 0x30004918, 0x00000611}, // Nom.:   800 kBit/s Clock:80MHz  PH1=74   PH2=25   SP=75.0%  SJW=25 
                                                          //   FD:     8 MBit/s Clock:80MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  {1000,  2000,  80, 0xBFBE804F, 0x26003A13, 0x00001C99}, // Nom.:     1 MBit/s Clock:80MHz  PH1=59   PH2=20   SP=75.0%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:80MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  {1000,  4000,  80, 0xBFBE804F, 0x26003A13, 0x00000D44}, // Nom.:     1 MBit/s Clock:80MHz  PH1=59   PH2=20   SP=75.0%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:80MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  {1000,  5000,  80, 0xBFBE804F, 0x26003A13, 0x00000A33}, // Nom.:     1 MBit/s Clock:80MHz  PH1=59   PH2=20   SP=75.0%  SJW=20 
                                                          //   FD:     5 MBit/s Clock:80MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  {1000,  8000,  80, 0xBD3E804F, 0x26003A13, 0x00000611}, // Nom.:     1 MBit/s Clock:80MHz  PH1=59   PH2=20   SP=75.0%  SJW=20 
                                                          //   FD:     8 MBit/s Clock:80MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  /********************** Clock: 96 MHz ****************************/
  {  10,   125,  96, 0x3DBD512B, 0x541FFF2A, 0x001F1322}, // Nom.:    10 kBit/s Clock:96MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   125 kBit/s Clock:96MHz  PH1=20   PH2=3    SP=87.5%  SJW=3  
  {  10,   250,  96, 0x3D3D512B, 0x541FFF2A, 0x001F0900}, // Nom.:    10 kBit/s Clock:96MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   250 kBit/s Clock:96MHz  PH1=10   PH2=1    SP=91.6%  SJW=1  
  {  10,   500,  96, 0x3D3D512B, 0x541FFF2A, 0x001F0300}, // Nom.:    10 kBit/s Clock:96MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {  20,   125,  96, 0xBFBE80C7, 0x3017AD18, 0x00171A33}, // Nom.:    20 kBit/s Clock:96MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  96, 0xBEBE80C7, 0x3017AD18, 0x00170C11}, // Nom.:    20 kBit/s Clock:96MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:96MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  96, 0xBD3D512B, 0x540FFF2A, 0x000F0811}, // Nom.:    20 kBit/s Clock:96MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=9    PH2=2    SP=83.3%  SJW=2  
  {  20,  1000,  96, 0xBEBE80C7, 0x3017AD18, 0x00170100}, // Nom.:    20 kBit/s Clock:96MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  20,  1500,  96, 0xBDBD512B, 0x540FFF2A, 0x000F0100}, // Nom.:    20 kBit/s Clock:96MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  96, 0xBFBE804F, 0x12174409, 0x00171A33}, // Nom.:    50 kBit/s Clock:96MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  96, 0xBFBE809F, 0x260B8A13, 0x000B1A33}, // Nom.:    50 kBit/s Clock:96MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   250 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   500,  96, 0xBD3A013F, 0x7C05FF3E, 0x00051855}, // Nom.:    50 kBit/s Clock:96MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  {  50,  1000,  96, 0xBEBE80EF, 0x3A07D01D, 0x00070722}, // Nom.:    50 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {  50,  1500,  96, 0xBFBE80EF, 0x3A07D01D, 0x00070411}, // Nom.:    50 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  96, 0xBEBE809F, 0x260B8A13, 0x000B0100}, // Nom.:    50 kBit/s Clock:96MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  3000,  96, 0xBFBE80EF, 0x3A07D01D, 0x00070100}, // Nom.:    50 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  4000,  96, 0xBDBA013F, 0x7C05FF3E, 0x00050100}, // Nom.:    50 kBit/s Clock:96MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  96, 0xBBBE8027, 0x08172104, 0x00171A33}, // Nom.:   100 kBit/s Clock:96MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  96, 0xBFBE804F, 0x120B4409, 0x000B1A33}, // Nom.:   100 kBit/s Clock:96MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   500,  96, 0xBD3E80EF, 0x3A03D01D, 0x00031FEE}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 100,  1000,  96, 0xBEBE80EF, 0x3A03D01D, 0x00031055}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 100,  1500,  96, 0xBFBE80EF, 0x3A03D01D, 0x00030A33}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 100,  2000,  96, 0xBEBE80EF, 0x3A03D01D, 0x00030722}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 100,  3000,  96, 0xBFBE80EF, 0x3A03D01D, 0x00030411}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 100,  4000,  96, 0xBEBE809F, 0x26058A13, 0x00050100}, // Nom.:   100 kBit/s Clock:96MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,  6000,  96, 0xBFBE80EF, 0x3A03D01D, 0x00030100}, // Nom.:   100 kBit/s Clock:96MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,  8000,  96, 0xBDBA013F, 0x7C02FF3E, 0x00020100}, // Nom.:   100 kBit/s Clock:96MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  96, 0xBBBE803F, 0x0E0B3607, 0x000B1A33}, // Nom.:   125 kBit/s Clock:96MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:   250 kBit/s Clock:96MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 125,   500,  96, 0xBD3E80BF, 0x2E03A617, 0x00031FEE}, // Nom.:   125 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 125,  1000,  96, 0xBFBE80FF, 0x3E02DE1F, 0x00021677}, // Nom.:   125 kBit/s Clock:96MHz  PH1=223  PH2=32   SP=87.5%  SJW=32 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 125,  1500,  96, 0xBFBE80BF, 0x2E03A617, 0x00030A33}, // Nom.:   125 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 125,  2000,  96, 0xBFBE80FF, 0x3E02DE1F, 0x00020A33}, // Nom.:   125 kBit/s Clock:96MHz  PH1=223  PH2=32   SP=87.5%  SJW=32 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 125,  3000,  96, 0xBFBE80BF, 0x2E03A617, 0x00030411}, // Nom.:   125 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  4000,  96, 0xBFBE80FF, 0x3E02DE1F, 0x00020411}, // Nom.:   125 kBit/s Clock:96MHz  PH1=223  PH2=32   SP=87.5%  SJW=32 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  6000,  96, 0xBFBE80BF, 0x2E03A617, 0x00030100}, // Nom.:   125 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,  8000,  96, 0xBFBE80FF, 0x3E02DE1F, 0x00020100}, // Nom.:   125 kBit/s Clock:96MHz  PH1=223  PH2=32   SP=87.5%  SJW=32 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 250,   500,  96, 0xBD3E805F, 0x1603520B, 0x00031FEE}, // Nom.:   250 kBit/s Clock:96MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:   500 kBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 250,  1000,  96, 0xBEBE807F, 0x1E026E0F, 0x00021677}, // Nom.:   250 kBit/s Clock:96MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 250,  1500,  96, 0xBFBE80BF, 0x2E01A617, 0x00011677}, // Nom.:   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 250,  2000,  96, 0xBFBE80BF, 0x2E01A617, 0x00011055}, // Nom.:   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 250,  3000,  96, 0xBFBE80BF, 0x2E01A617, 0x00010A33}, // Nom.:   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 250,  4000,  96, 0xBFBE80BF, 0x2E01A617, 0x00010722}, // Nom.:   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 250,  6000,  96, 0xBFBE80BF, 0x2E01A617, 0x00010411}, // Nom.:   250 kBit/s Clock:96MHz  PH1=167  PH2=24   SP=87.5%  SJW=24 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  8000,  96, 0xBEBE807F, 0x1E026E0F, 0x00020100}, // Nom.:   250 kBit/s Clock:96MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 500,  1000,  96, 0xBD3E605F, 0x24014B12, 0x00011FEE}, // Nom.:   500 kBit/s Clock:96MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 500,  1500,  96, 0xBDBE605F, 0x24014B12, 0x00011677}, // Nom.:   500 kBit/s Clock:96MHz  PH1=76   PH2=19   SP=80.2%  SJW=19 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 500,  2000,  96, 0xBD3E60BF, 0x4A009825, 0x00001FEE}, // Nom.:   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 500,  3000,  96, 0xBDBE60BF, 0x4A009825, 0x00001677}, // Nom.:   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 500,  4000,  96, 0xBDBE60BF, 0x4A009825, 0x00001055}, // Nom.:   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 500,  6000,  96, 0xBDBE60BF, 0x4A009825, 0x00000A33}, // Nom.:   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 500,  8000,  96, 0xBDBE60BF, 0x4A009825, 0x00000722}, // Nom.:   500 kBit/s Clock:96MHz  PH1=153  PH2=38   SP=80.2%  SJW=38 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 800,  1000,  96, 0xBABE8027, 0x12021C09, 0x00021677}, // Nom.:   800 kBit/s Clock:96MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
                                                          //   FD:     1 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 800,  1500,  96, 0xBBBE803B, 0x1C012B0E, 0x00011677}, // Nom.:   800 kBit/s Clock:96MHz  PH1=44   PH2=15   SP=75.0%  SJW=15 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 800,  2000,  96, 0xBD3E8077, 0x3A00581D, 0x00001FEE}, // Nom.:   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  { 800,  3000,  96, 0xBFBE8077, 0x3A00581D, 0x00001677}, // Nom.:   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  { 800,  4000,  96, 0xBFBE8077, 0x3A00581D, 0x00001055}, // Nom.:   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  { 800,  6000,  96, 0xBFBE8077, 0x3A00581D, 0x00000A33}, // Nom.:   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 800,  8000,  96, 0xBFBE8077, 0x3A00581D, 0x00000722}, // Nom.:   800 kBit/s Clock:96MHz  PH1=89   PH2=30   SP=75.0%  SJW=30 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {1000,  1500,  96, 0xBBBE802F, 0x1601220B, 0x00011677}, // Nom.:     1 MBit/s Clock:96MHz  PH1=35   PH2=12   SP=75.0%  SJW=12 
                                                          //   FD:   1.5 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  {1000,  2000,  96, 0xBD3E805F, 0x2E004617, 0x00001FEE}, // Nom.:     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
                                                          //   FD:     2 MBit/s Clock:96MHz  PH1=32   PH2=15   SP=68.7%  SJW=15 
  {1000,  3000,  96, 0xBFBE805F, 0x2E004617, 0x00001677}, // Nom.:     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
                                                          //   FD:     3 MBit/s Clock:96MHz  PH1=23   PH2=8    SP=75.0%  SJW=8  
  {1000,  4000,  96, 0xBFBE805F, 0x2E004617, 0x00001055}, // Nom.:     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
                                                          //   FD:     4 MBit/s Clock:96MHz  PH1=17   PH2=6    SP=75.0%  SJW=6  
  {1000,  6000,  96, 0xBFBE805F, 0x2E004617, 0x00000A33}, // Nom.:     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
                                                          //   FD:     6 MBit/s Clock:96MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  {1000,  8000,  96, 0xBFBE805F, 0x2E004617, 0x00000722}, // Nom.:     1 MBit/s Clock:96MHz  PH1=71   PH2=24   SP=75.0%  SJW=24 
                                                          //   FD:     8 MBit/s Clock:96MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  /********************** Clock: 40 MHz ****************************/
  {  10,   125,  40, 0xBEBE80C7, 0x3013AD18, 0x00130C11}, // Nom.:    10 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:40MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  10,   250,  40, 0xBEBE80C7, 0x3013AD18, 0x00130500}, // Nom.:    10 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:40MHz  PH1=6    PH2=1    SP=87.5%  SJW=1  
  {  10,   500,  40, 0xBDBE70F9, 0x3C0FD91E, 0x000F0200}, // Nom.:    10 kBit/s Clock:40MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  {  20,   125,  40, 0xBEBE80C7, 0x3009AD18, 0x00091A33}, // Nom.:    20 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:40MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  40, 0xBEBE80C7, 0x3009AD18, 0x00090C11}, // Nom.:    20 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:40MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  40, 0xBDBE70F9, 0x3C07D91E, 0x00070611}, // Nom.:    20 kBit/s Clock:40MHz  PH1=218  PH2=31   SP=87.6%  SJW=31 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  {  20,  1000,  40, 0xBEBE80C7, 0x3009AD18, 0x00090100}, // Nom.:    20 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  40, 0xBEBE804F, 0x12094409, 0x00091A33}, // Nom.:    50 kBit/s Clock:40MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:40MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  40, 0xBEBE809F, 0x26048A13, 0x00041A33}, // Nom.:    50 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   250 kBit/s Clock:40MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   500,  40, 0xBFBE80C7, 0x3003AD18, 0x00030E33}, // Nom.:    50 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=15   PH2=4    SP=80.0%  SJW=4  
  {  50,  1000,  40, 0xBEBE809F, 0x26048A13, 0x00040411}, // Nom.:    50 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  40, 0xBEBE809F, 0x26048A13, 0x00040100}, // Nom.:    50 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  40, 0xBABE8027, 0x08092104, 0x00091A33}, // Nom.:   100 kBit/s Clock:40MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:40MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  40, 0xBEBE804F, 0x12044409, 0x00041A33}, // Nom.:   100 kBit/s Clock:40MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:40MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   500,  40, 0xBFBE80C7, 0x3001AD18, 0x00011E77}, // Nom.:   100 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 100,  1000,  40, 0xBFBE80C7, 0x3001AD18, 0x00010D44}, // Nom.:   100 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 100,  2000,  40, 0xBEBE804F, 0x12044409, 0x00040100}, // Nom.:   100 kBit/s Clock:40MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,  4000,  40, 0xBD3E80C7, 0x3001AD18, 0x00010200}, // Nom.:   100 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  { 100,  5000,  40, 0xBFBE80C7, 0x3001AD18, 0x00010100}, // Nom.:   100 kBit/s Clock:40MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  40, 0xBD3E804F, 0x12034409, 0x00031F66}, // Nom.:   125 kBit/s Clock:40MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:40MHz  PH1=32   PH2=7    SP=82.5%  SJW=7  
  { 125,   500,  40, 0xBFBE809F, 0x26018A13, 0x00011E77}, // Nom.:   125 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 125,  1000,  40, 0xBEBE809F, 0x26018A13, 0x00010D44}, // Nom.:   125 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 125,  2000,  40, 0xBDBA013F, 0x7C00FF3E, 0x00000D44}, // Nom.:   125 kBit/s Clock:40MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 125,  4000,  40, 0xBD3A013F, 0x7C00FF3E, 0x00000611}, // Nom.:   125 kBit/s Clock:40MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 125,  5000,  40, 0xBEBE809F, 0x26018A13, 0x00010100}, // Nom.:   125 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,  8000,  40, 0xBD3A013F, 0x7C00FF3E, 0x00000200}, // Nom.:   125 kBit/s Clock:40MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     8 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  { 250,   500,  40, 0xBFBE804F, 0x12014409, 0x00011E77}, // Nom.:   250 kBit/s Clock:40MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   500 kBit/s Clock:40MHz  PH1=31   PH2=8    SP=80.0%  SJW=8  
  { 250,  1000,  40, 0xBFBE809F, 0x26008A13, 0x00001C99}, // Nom.:   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 250,  2000,  40, 0xBFBE809F, 0x26008A13, 0x00000D44}, // Nom.:   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 250,  4000,  40, 0xBD3E809F, 0x26008A13, 0x00000611}, // Nom.:   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 250,  5000,  40, 0xBFBE809F, 0x26008A13, 0x00000411}, // Nom.:   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  8000,  40, 0xBD3E809F, 0x26008A13, 0x00000200}, // Nom.:   250 kBit/s Clock:40MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     8 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  { 500,  1000,  40, 0xBFBE804F, 0x1E003E0F, 0x00001C99}, // Nom.:   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 500,  2000,  40, 0xBFBE804F, 0x1E003E0F, 0x00000D44}, // Nom.:   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 500,  4000,  40, 0xBD3E804F, 0x1E003E0F, 0x00000611}, // Nom.:   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 500,  5000,  40, 0xBFBE804F, 0x1E003E0F, 0x00000411}, // Nom.:   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 500,  8000,  40, 0xBD3E804F, 0x1E003E0F, 0x00000200}, // Nom.:   500 kBit/s Clock:40MHz  PH1=63   PH2=16   SP=80.0%  SJW=16 
                                                          //   FD:     8 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  { 800,  1000,  40, 0xB9BDE031, 0x1600240B, 0x00001C99}, // Nom.:   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
  { 800,  2000,  40, 0xB9BDE031, 0x1600240B, 0x00000D44}, // Nom.:   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  { 800,  4000,  40, 0xB93DE031, 0x1600240B, 0x00000611}, // Nom.:   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  { 800,  5000,  40, 0xB9BDE031, 0x1600240B, 0x00000411}, // Nom.:   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 800,  8000,  40, 0xB93DE031, 0x1600240B, 0x00000200}, // Nom.:   800 kBit/s Clock:40MHz  PH1=37   PH2=12   SP=76.0%  SJW=12 
                                                          //   FD:     8 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  {1000,  2000,  40, 0xBBBE8027, 0x12001C09, 0x00000D44}, // Nom.:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
                                                          //   FD:     2 MBit/s Clock:40MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
  {1000,  4000,  40, 0xB93E8027, 0x12001C09, 0x00000611}, // Nom.:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
                                                          //   FD:     4 MBit/s Clock:40MHz  PH1=7    PH2=2    SP=80.0%  SJW=2  
  {1000,  5000,  40, 0xBBBE8027, 0x12001C09, 0x00000411}, // Nom.:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
                                                          //   FD:     5 MBit/s Clock:40MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {1000,  8000,  40, 0xB93E8027, 0x12001C09, 0x00000200}, // Nom.:     1 MBit/s Clock:40MHz  PH1=29   PH2=10   SP=75.0%  SJW=10 
                                                          //   FD:     8 MBit/s Clock:40MHz  PH1=3    PH2=1    SP=80.0%  SJW=1  
  /********************** Clock: 16 MHz ****************************/
  {  10,   125,  16, 0xBFBE80C7, 0x3007AD18, 0x00070C11}, // Nom.:    10 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:16MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  10,   250,  16, 0xBFBE80C7, 0x3007AD18, 0x00070500}, // Nom.:    10 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:16MHz  PH1=6    PH2=1    SP=87.5%  SJW=1  
  {  10,   500,  16, 0xBD3E80C7, 0x3007AD18, 0x00070100}, // Nom.:    10 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  20,   125,  16, 0xBFBE80C7, 0x3003AD18, 0x00031A33}, // Nom.:    20 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  16, 0xBFBE80C7, 0x3003AD18, 0x00030C11}, // Nom.:    20 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:16MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  16, 0xBD3E80C7, 0x3003AD18, 0x00030411}, // Nom.:    20 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  20,  1000,  16, 0xBFBE80C7, 0x3003AD18, 0x00030100}, // Nom.:    20 kBit/s Clock:16MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  16, 0xBFBE804F, 0x12034409, 0x00031A33}, // Nom.:    50 kBit/s Clock:16MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  16, 0xBFBE809F, 0x26018A13, 0x00011A33}, // Nom.:    50 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   250 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   500,  16, 0xBD3A013F, 0x7C00FF3E, 0x00001855}, // Nom.:    50 kBit/s Clock:16MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  {  50,  1000,  16, 0xBEBE809F, 0x26018A13, 0x00010411}, // Nom.:    50 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  16, 0xBEBE809F, 0x26018A13, 0x00010100}, // Nom.:    50 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,  4000,  16, 0xBDBA013F, 0x7C00FF3E, 0x00000100}, // Nom.:    50 kBit/s Clock:16MHz  PH1=256  PH2=63   SP=80.3%  SJW=63 
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  16, 0xBBBE8027, 0x08032104, 0x00031A33}, // Nom.:   100 kBit/s Clock:16MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  16, 0xBFBE804F, 0x12014409, 0x00011A33}, // Nom.:   100 kBit/s Clock:16MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   250 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   500,  16, 0xBD3E809F, 0x26008A13, 0x00001855}, // Nom.:   100 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  { 100,  1000,  16, 0xBFBE809F, 0x26008A13, 0x00000A33}, // Nom.:   100 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 100,  2000,  16, 0xBFBE809F, 0x26008A13, 0x00000411}, // Nom.:   100 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 100,  4000,  16, 0xBFBE809F, 0x26008A13, 0x00000100}, // Nom.:   100 kBit/s Clock:16MHz  PH1=139  PH2=20   SP=87.5%  SJW=20 
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  16, 0xBBBE803F, 0x0E013607, 0x00011A33}, // Nom.:   125 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:   250 kBit/s Clock:16MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 125,   500,  16, 0xBD3E807F, 0x1E006E0F, 0x00001855}, // Nom.:   125 kBit/s Clock:16MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  { 125,  1000,  16, 0xBFBE807F, 0x1E006E0F, 0x00000A33}, // Nom.:   125 kBit/s Clock:16MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 125,  2000,  16, 0xBFBE807F, 0x1E006E0F, 0x00000411}, // Nom.:   125 kBit/s Clock:16MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  4000,  16, 0xBFBE807F, 0x1E006E0F, 0x00000100}, // Nom.:   125 kBit/s Clock:16MHz  PH1=111  PH2=16   SP=87.5%  SJW=16 
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 250,   500,  16, 0xB93E803F, 0x0E003607, 0x00001855}, // Nom.:   250 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
  { 250,  1000,  16, 0xBBBE803F, 0x0E003607, 0x00000A33}, // Nom.:   250 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 250,  2000,  16, 0xBBBE803F, 0x0E003607, 0x00000411}, // Nom.:   250 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  4000,  16, 0xBBBE803F, 0x0E003607, 0x00000100}, // Nom.:   250 kBit/s Clock:16MHz  PH1=55   PH2=8    SP=87.5%  SJW=8  
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 500,  1000,  16, 0xB1BDC01F, 0x0A001805, 0x00000A33}, // Nom.:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 500,  2000,  16, 0xB1BDC01F, 0x0A001805, 0x00000411}, // Nom.:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 500,  4000,  16, 0xB1BDC01F, 0x0A001805, 0x00000100}, // Nom.:   500 kBit/s Clock:16MHz  PH1=25   PH2=6    SP=81.2%  SJW=6  
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 800,  1000,  16, 0xB3BE8013, 0x08000D04, 0x00000A33}, // Nom.:   800 kBit/s Clock:16MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
                                                          //   FD:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
  { 800,  2000,  16, 0xB3BE8013, 0x08000D04, 0x00000411}, // Nom.:   800 kBit/s Clock:16MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 800,  4000,  16, 0xB3BE8013, 0x08000D04, 0x00000100}, // Nom.:   800 kBit/s Clock:16MHz  PH1=14   PH2=5    SP=75.0%  SJW=5  
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {1000,  2000,  16, 0xB3BE800F, 0x06000A03, 0x00000411}, // Nom.:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
                                                          //   FD:     2 MBit/s Clock:16MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {1000,  4000,  16, 0xB3BE800F, 0x06000A03, 0x00000100}, // Nom.:     1 MBit/s Clock:16MHz  PH1=11   PH2=4    SP=75.0%  SJW=4  
                                                          //   FD:     4 MBit/s Clock:16MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  /********************** Clock: 12 MHz ****************************/
  {  10,   125,  12, 0xBEBE80C7, 0x3005AD18, 0x00050C11}, // Nom.:    10 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:12MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  10,   250,  12, 0xBEBE80C7, 0x3005AD18, 0x00050500}, // Nom.:    10 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:12MHz  PH1=6    PH2=1    SP=87.5%  SJW=1  
  {  10,   500,  12, 0xBD3D512B, 0x5403FF2A, 0x00030300}, // Nom.:    10 kBit/s Clock:12MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {  20,   125,  12, 0xBFBE80C7, 0x3002AD18, 0x00021A33}, // Nom.:    20 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   125 kBit/s Clock:12MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  20,   250,  12, 0xBEBE80C7, 0x3002AD18, 0x00020C11}, // Nom.:    20 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:   250 kBit/s Clock:12MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  {  20,   500,  12, 0xBD3D512B, 0x5401FF2A, 0x00010811}, // Nom.:    20 kBit/s Clock:12MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=9    PH2=2    SP=83.3%  SJW=2  
  {  20,  1000,  12, 0xBEBE80C7, 0x3002AD18, 0x00020100}, // Nom.:    20 kBit/s Clock:12MHz  PH1=174  PH2=25   SP=87.5%  SJW=25 
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  20,  1500,  12, 0xBDBD512B, 0x5401FF2A, 0x00010100}, // Nom.:    20 kBit/s Clock:12MHz  PH1=256  PH2=43   SP=85.6%  SJW=43 
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {  50,   125,  12, 0xBFBE804F, 0x12024409, 0x00021A33}, // Nom.:    50 kBit/s Clock:12MHz  PH1=69   PH2=10   SP=87.5%  SJW=10 
                                                          //   FD:   125 kBit/s Clock:12MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  {  50,   250,  12, 0xBFBE8077, 0x1C01670E, 0x00011322}, // Nom.:    50 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:   250 kBit/s Clock:12MHz  PH1=20   PH2=3    SP=87.5%  SJW=3  
  {  50,   500,  12, 0xBD3E80EF, 0x3A00D01D, 0x00001144}, // Nom.:    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
  {  50,  1000,  12, 0xBFBE80EF, 0x3A00D01D, 0x00000722}, // Nom.:    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  {  50,  1500,  12, 0xBFBE80EF, 0x3A00D01D, 0x00000411}, // Nom.:    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {  50,  2000,  12, 0xBD3E80EF, 0x3A00D01D, 0x00000300}, // Nom.:    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {  50,  3000,  12, 0xBFBE80EF, 0x3A00D01D, 0x00000100}, // Nom.:    50 kBit/s Clock:12MHz  PH1=209  PH2=30   SP=87.5%  SJW=30 
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 100,   125,  12, 0xBBBE8027, 0x08022104, 0x00021A33}, // Nom.:   100 kBit/s Clock:12MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   125 kBit/s Clock:12MHz  PH1=27   PH2=4    SP=87.5%  SJW=4  
  { 100,   250,  12, 0xBABE8027, 0x08022104, 0x00020C11}, // Nom.:   100 kBit/s Clock:12MHz  PH1=34   PH2=5    SP=87.5%  SJW=5  
                                                          //   FD:   250 kBit/s Clock:12MHz  PH1=13   PH2=2    SP=87.5%  SJW=2  
  { 100,   500,  12, 0xBD3E8077, 0x1C00670E, 0x00001144}, // Nom.:   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
  { 100,  1000,  12, 0xBFBE8077, 0x1C00670E, 0x00000722}, // Nom.:   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 100,  1500,  12, 0xBFBE8077, 0x1C00670E, 0x00000411}, // Nom.:   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 100,  2000,  12, 0xBD3E8077, 0x1C00670E, 0x00000300}, // Nom.:   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 100,  3000,  12, 0xBFBE8077, 0x1C00670E, 0x00000100}, // Nom.:   100 kBit/s Clock:12MHz  PH1=104  PH2=15   SP=87.5%  SJW=15 
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 125,   250,  12, 0xBBBE802F, 0x0A012805, 0x00011322}, // Nom.:   125 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:   250 kBit/s Clock:12MHz  PH1=20   PH2=3    SP=87.5%  SJW=3  
  { 125,   500,  12, 0xBD3E805F, 0x1600520B, 0x00001144}, // Nom.:   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
  { 125,  1000,  12, 0xBFBE805F, 0x1600520B, 0x00000722}, // Nom.:   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 125,  1500,  12, 0xBFBE805F, 0x1600520B, 0x00000411}, // Nom.:   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 125,  2000,  12, 0xBD3E805F, 0x1600520B, 0x00000300}, // Nom.:   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 125,  3000,  12, 0xBFBE805F, 0x1600520B, 0x00000100}, // Nom.:   125 kBit/s Clock:12MHz  PH1=83   PH2=12   SP=87.5%  SJW=12 
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 250,   500,  12, 0xB93E802F, 0x0A002805, 0x00001144}, // Nom.:   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
  { 250,  1000,  12, 0xBBBE802F, 0x0A002805, 0x00000722}, // Nom.:   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 250,  1500,  12, 0xBBBE802F, 0x0A002805, 0x00000411}, // Nom.:   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 250,  2000,  12, 0xB93E802F, 0x0A002805, 0x00000300}, // Nom.:   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 250,  3000,  12, 0xBBBE802F, 0x0A002805, 0x00000100}, // Nom.:   250 kBit/s Clock:12MHz  PH1=41   PH2=6    SP=87.5%  SJW=6  
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 500,  1000,  12, 0xB1BDF017, 0x08001104, 0x00000722}, // Nom.:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 500,  1500,  12, 0xB1BDF017, 0x08001104, 0x00000411}, // Nom.:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 500,  2000,  12, 0xB13DF017, 0x08001104, 0x00000300}, // Nom.:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 500,  3000,  12, 0xB1BDF017, 0x08001104, 0x00000100}, // Nom.:   500 kBit/s Clock:12MHz  PH1=18   PH2=5    SP=79.1%  SJW=5  
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  { 800,  1000,  12, 0xB1BD700E, 0x06000903, 0x00000722}, // Nom.:   800 kBit/s Clock:12MHz  PH1=10   PH2=4    SP=73.3%  SJW=4  
                                                          //   FD:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
  { 800,  1500,  12, 0xB1BD700E, 0x06000903, 0x00000411}, // Nom.:   800 kBit/s Clock:12MHz  PH1=10   PH2=4    SP=73.3%  SJW=4  
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  { 800,  2000,  12, 0xB13D700E, 0x06000903, 0x00000300}, // Nom.:   800 kBit/s Clock:12MHz  PH1=10   PH2=4    SP=73.3%  SJW=4  
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  { 800,  3000,  12, 0xB1BD700E, 0x06000903, 0x00000100}, // Nom.:   800 kBit/s Clock:12MHz  PH1=10   PH2=4    SP=73.3%  SJW=4  
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {1000,  1500,  12, 0xB3BE800B, 0x04000702, 0x00000411}, // Nom.:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
                                                          //   FD:   1.5 MBit/s Clock:12MHz  PH1=5    PH2=2    SP=75.0%  SJW=2  
  {1000,  2000,  12, 0xB13E800B, 0x04000702, 0x00000300}, // Nom.:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
                                                          //   FD:     2 MBit/s Clock:12MHz  PH1=4    PH2=1    SP=83.3%  SJW=1  
  {1000,  3000,  12, 0xB3BE800B, 0x04000702, 0x00000100}, // Nom.:     1 MBit/s Clock:12MHz  PH1=8    PH2=3    SP=75.0%  SJW=3  
                                                          //   FD:     3 MBit/s Clock:12MHz  PH1=2    PH2=1    SP=75.0%  SJW=1  
  {0, 0, 0, 0, 0, 0}};