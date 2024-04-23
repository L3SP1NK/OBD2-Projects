/***************************************************************************
                           errors.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modify       : 20.04.2022
    copyright         : (C) 2008 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdlib.h>
#include "errors.h"

/**************************************************************************/
/*                        C O N S T A N T E N                             */
/**************************************************************************/
static const char *ErrorStrings[MAX_ERROR_NUMBERS] = {
   "Ok",
   "Treiber nicht Initialisiert",                                   // ERR_DRIVER_NOT_INIT           -1
   "Es wurden ungueltige Parameter Werte uebergeben",               // ERR_PARAM                     -2
   "Ungueltiger Index-Wert",                                        // ERR_INDEX_RANGE               -3
   "Device nicht gefunden",                                         // ERR_INDEX_NO_DEVICE           -4
   "Allgemeiner Fehler",                                            // ERR_PUBLIC_ERROR              -5
   "In das Fifo kann nicht geschrieben werden",                     // ERR_WRITE_SOFT_FIFO           -6
   "Der Puffer kann nicht geschrieben werden",                      // ERR_WRITE_PUFFER              -7
   "Das Fifo kann nicht gelesen werden",                            // ERR_READ_SOFT_FIFO            -8
   "Der Puffer kann nicht gelesen werden",                          // ERR_READ_PUFFER               -9
   "Variable nicht gefunden",                                       // ERR_VAR_NOT_FOUND             -10
   "Lesen der Variable nicht erlaubt",                              // ERR_VAR_GET_PROTECT           -11
   "Lesepuffer für Variable zu klein",                              // ERR_VAR_GET_PUFFER_SIZE       -12
   "Schreiben der Variable nicht erlaubt",                          // ERR_VAR_SET_PROTECT           -13
   "Der zu schreibende String/Stream ist zu groß",                  // ERR_VAR_SET_SIZE              -14
   "Min Wert unterschritten",                                       // ERR_VAR_SET_MIN               -15
   "Max Wert überschritten",                                        // ERR_VAR_SET_MAX               -16
   "Zugriff verweigert",                                            // ERR_ACCESS_DENIED             -17
   "Ungültiger Wert für CAN-Speed",                                 // ERR_VAR_SPEED_NOT_SUPPORT     -18
   "Ungültiger Wert für Baudrate",                                  // ERR_VAR_BAUD_RATE_NOT_SUPPORT -19
   "Value nicht gesetzt",                                           // ERR_FIELD_VALUE_IS_EMPTY      -20
   "Keine Verbindung zur Hardware",                                 // ERR_HARDWARE_OFFLINE          -21
   "Kommunikationsfehler zur Hardware",                             // ERR_SM_DRV_COMMANDO           -22
   "Hardware sendet falsche Anzahl Parameter",                      // ERR_SM_DRV_ACK_COUNT          -23
   "Zu wenig Arbeitsspeicher",                                      // ERR_ALLOCATE_MEMORY           -24
   "Das System kann die benötigten Resourcen nicht bereitstellen",  // ERR_ALLOCATE_RESOURCES        -25
   "Ein System-CALL kehrt mit einen Fehler zurück",                 // ERR_SYSTEM_CALL               -26
   "Der Main-Thread ist beschäftigt",                               // ERR_MAIN_THREAD_IS_BUSY       -27
   "Der Main-Thread laeuft nicht",                                  // ERR_MAIN_THREAD_NOT_RUN       -28
   "User Allocierter Speicher nicht gefunden",                      // ERR_USER_ALLOC_MEM_NOT_FOUND  -29
   "USB/COM Schnittstelle kann nicht Initialisiert werden",         // ERR_INIT_USB_COM              -30 
   "Hardware nicht verbunden",                                      // ERR_DEVICE_UNPLUGGED          -31
   "Der Main-Thread kann nicht gestartet werden",                   // ERR_START_MAIN_THREAD         -32    
   "Type der Variable falsch",                                      // ERR_VAR_WRONG_TYPE            -33
   "Device ist beschäftigt, schließen nicht möglich",               // ERR_DEVICE_BUSY               -34 
   "Unbekanter Interner Fehler",                                    // ERR_FATAL_INTERNEL_ERROR      -35
   "Wert für DLC zu groß (FD Flags gesetzt?)",                      // ERR_CAN_DLC_OVERFLOW          -36
   "Treiber nicht im CAN-FD Mode Initialisiert"};                   // ERR_NOT_IN_FD_MODE_INIT       -37  
    
/*   "Fehler beim erzeugen des Sockets",                              // ERR_CREATE_SOCKET             -34
   "Fehler bei Verbinden des Sockets",                              // ERR_BIND_SOCKET               -35
   "Fehler Call \"setsockopt\", set \"LOOPBACK\"",                  // ERR_SOCKOPT_SET_LOOPBACK      -36
   "Fehler Call \"setsockopt\", set \"TIMESTAMP\""};                // ERR_SOCKOPT_SET_TIMESTAMP     -37
*/

static const char UnknownErrorString[] = {"Unbekannter Fehler"};


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/

/*
******************** GetErrorString ********************
*/
const char *GetErrorString(int32_t error)
{
if (error > 0)
  error = 0;
error = abs(error);
if (error >= MAX_ERROR_NUMBERS)
  return(UnknownErrorString);
else
  {
  /* if (error == abs(ERR_SM_DRV_COMMANDO))
    return(SMDrvGetErrorStr(SMDrvGetLastError()));
  else */
  return(ErrorStrings[error]);
  }
}
