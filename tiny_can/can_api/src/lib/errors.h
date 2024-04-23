#ifndef __ERRORS_H__
#define __ERRORS_H__

#include "global.h"

#define MAX_ERROR_NUMBERS 38         // +1


// Allgemein
#define ERR_DRIVER_NOT_INIT           -1   // Treiber nicht Initialisiert
#define ERR_PARAM                     -2   // Es wurden ungueltige Parameter Werte uebergeben
#define ERR_INDEX_RANGE               -3   // Ungueltiger Index-Wert
#define ERR_INDEX_NO_DEVICE           -4   // Device nicht gefunden <*>
#define ERR_PUBLIC_ERROR              -5   // Allgemeiner Fehler
#define ERR_WRITE_SOFT_FIFO           -6   // In das Fifo kann nicht geschrieben werden
#define ERR_WRITE_PUFFER              -7   // Der Puffer kann nicht geschrieben werden
#define ERR_READ_SOFT_FIFO            -8   // Das Fifo kann icht gelesen werden
#define ERR_READ_PUFFER               -9   // Der Puffer kann nicht gelesen werden
// VAR.C
#define ERR_VAR_NOT_FOUND             -10  // Variable nicht gefunden
#define ERR_VAR_GET_PROTECT           -11  // Lesen der Variable nicht erlaubt
#define ERR_VAR_GET_PUFFER_SIZE       -12  // Lesepuffer für Variable zu klein
#define ERR_VAR_SET_PROTECT           -13  // Schreiben der Variable nicht erlaubt
#define ERR_VAR_SET_SIZE              -14  // Der zu schreibende String/Stream ist zu groß
#define ERR_VAR_SET_MIN               -15  // Min Wert unterschritten
#define ERR_VAR_SET_MAX               -16  // Max Wert überschritten
#define ERR_ACCESS_DENIED             -17  // Zugriff verweigert
// VAR_EVENT.C
#define ERR_VAR_SPEED_NOT_SUPPORT     -18  // Ungültige CAN-Speed
#define ERR_VAR_BAUD_RATE_NOT_SUPPORT -19  // Ungültige Baudrate
// CONFIG.C
#define ERR_FIELD_VALUE_IS_EMPTY      -20  // Value nicht gesetzt
// MHSTCAN.C
#define ERR_HARDWARE_OFFLINE          -21  // Keine Verbindung zur Hardware
// TAR_DRV.C
#define ERR_SM_DRV_COMMANDO           -22  // Kommunikationsfehler zur Hardware
#define ERR_SM_DRV_ACK_COUNT          -23  // Hardware sendet falsche Anzahl Parameter
// xxx_MHSTCAN.C
#define ERR_ALLOCATE_MEMORY           -24  // Zu wenig Arbeitsspeicher
#define ERR_ALLOCATE_RESOURCES        -25  // Das System kann die benötigten Resourcen nicht bereitstellen
#define ERR_SYSTEM_CALL               -26  // Ein System-CALL kehrt mit eine Fehler zurück
#define ERR_MAIN_THREAD_IS_BUSY       -27  // Der Main-Thread ist beschäftigt
#define ERR_MAIN_THREAD_NOT_RUN       -28  // Der Main-Thread läuft nicht
#define ERR_USER_ALLOC_MEM_NOT_FOUND  -29  // User Allocierter Speicher nicht gefunden
#define ERR_INIT_USB_COM              -30  // USB/COM Schnittstelle kann nicht Initialisiert werden
#define ERR_DEVICE_UNPLUGGED          -31  // Hardware nicht verbunden

#define ERR_START_MAIN_THREAD         -32  // Der Main-Thread kann nicht gestartet werden
#define ERR_VAR_WRONG_TYPE            -33  // Type der Variable falsch

// <*> neu
#define ERR_DEVICE_BUSY               -34  // Device ist beschäftigt, schließen nicht möglich
#define ERR_FATAL_INTERNEL_ERROR      -35  // 
#define ERR_CAN_DLC_OVERFLOW          -36
#define ERR_NOT_IN_FD_MODE_INIT       -37  // Treiber nicht im CAN-FD Mode Initialisiert

#define ERR_COMM_STREAM_SIZE          -38
#define ERR_COMM_STREAM_FORMAT        -39
#define ERR_UNSUPPORTED_FEATURE       -40  // Die Hardware unterstützt diese Funktion nicht

// Fehler für SocketCAN
/*#define ERR_CREATE_SOCKET             -500  // Fehler beim erzeugen des Sockets
#define ERR_BIND_SOCKET               -501  // Fehler bei Verbinden des Sockets
#define ERR_SOCKOPT_SET_LOOPBACK      -502  // Fehler Call "setsockopt", set "LOOPBACK"
#define ERR_SOCKOPT_SET_TIMESTAMP     -503  // Fehler Call "setsockopt", set "TIMESTAMP"
*/


const char *GetErrorString(int32_t error) ATTRIBUTE_INTERNAL;

#endif