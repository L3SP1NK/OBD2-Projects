/***************************************************************************
                          drv_info.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modified     : 20.04.2023
    copyright         : (C) 2008 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "config.h"
#include "drv_info.h"

/*
API Version 1.31
  - Initialisierungsvariable CanDrvMode eingefügt (0 = RS232, 1 = FTDI-USB)

API Version 1.32
  - Initialisierungsvariable MainThreadPriority THREAD_PRIORITY_REALTIME ergänzt
  - Initialisierungsvariable MainThreadPriority eingefügt

API Version 2.00
  - Unterstützung für mehrere CAN-Module

API Version 2.02
  - Filter unterstützen RTR Frames

API Version 2.03
  - Der Timestamp unterstütz den Modus "TIME_STAMP_HW_SW_UNIX", ist ein Hardware
    Timestamp verfügbar wird dieser benutzt ansonsten wird der Software Timestamp
    verwendet

API Version 2.10
  - Empfangene CAN Nachrichten können auch "CAN Bus Fehler Telegramme" enthalten, siehe
    can_types.h für eine genauere Beschreibung.
    Die Funktion wird nur vom Tiny-CAN IV-XL Modul unterstützt.
  - (Nur Windows) CanDeviceOpen den Parameter "LocId" gibt es nicht mehr.
    Hat unter Win7 nicht richtig funktioniert und wurde deshalb gelöscht, ein Bug im
    FTDI Treiber der nicht behoben werden konnte.

API Version 2.11
  - Mit der neuen Variable "AutoStopCan" kann der CAN-Bus automatisch bei schließen
    des Devices gestoppt werden

API Version 3.00
  - Folgende Funktionsaufrufe wurden ergänzt:
       CanExGetDeviceListPerform
       CanExGetDeviceListGet
       CanExGetDeviceInfoPerform
       CanExGetDeviceInfoGet
       CanExGetAsStringCopy

API Version 3.10
  - Anpassungen für Tiny-CAN LS: "CanStatus" um die Eigenschaft "BUS_FAILURE" ergänzt

API Version 3.11
  - "TCanFlagsBits" wurde um das Flag "FilHit" ergänzt

API Version 3.12
  - Neuer API Call: CanExSetEventAll
  
API Version 4.00  
  - CAN-FD Unterstützung
  
API Version 4.01
  - Neu: FilIdMode -> 3 = Maske & Code Pass Mode
  
API Version 4.02
  - Neu: TxAckBypass  (#define HAVE_TX_ACK_BYPASS)
  
API Version 4.10  
  - Ethernet Hardware Unterstützung  
  
API Version 4.20
  - Neu: Hardware beschleunigung für ISO-TP ECU-Flash programmierung
  
API Version 4.30
  - Neue API Funtionen: CanExGetInfoList, CanExGetInfoListPerform, CanExGetInfoListGet            
*/

const char DrvInfoStr[] =
   {"Description=Tiny-CAN API Treiber;"
    "Hardware=Tiny-CAN I-XL, Tiny-CAN II-XL, Tiny-CAN IV-XL, Tiny-CAN V-XL, Tiny-CAN M1, Tiny-CAN M2, Tiny-CAN M3, Tiny-CAN M232, Tiny-CAN LS;"
    "Hardware IDs=0x43414E00, 0x43414E42, 0x43414E43, 0x43414E10, 0x43414E04, 0x43414E05, 0x43414E06, 0x43414E07, 0x43414E08, 0x43414E09;"
    "Version=8.1.1;"
    "Interface Type=USB;"
    "API Version=4.30;"
    "Autor=Klaus Demlehner;"
    "Homepage=www.mhs-elektronik.de"};
