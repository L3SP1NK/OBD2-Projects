#ifndef __CAN_TYPES_H__
#define __CAN_TYPES_H__

#include <global.h>

#ifdef __cplusplus
  extern "C" {
#endif

/*#ifdef __WIN32__
#include "time.h" */
/*struct timeval
  {
  long tv_sec;
  long tv_usec;
  }; */
/*#else
#include <sys/time.h>
#endif */

#define INDEX_INVALID          0xFFFFFFFF

#define INDEX_FIFO_PUFFER_MASK 0x0000FFFF
#define INDEX_SOFT_FLAG        0x02000000
#define INDEX_TXT_FLAG         0x01000000
#define INDEX_CAN_KANAL_MASK   0x000F0000
#define INDEX_CAN_DEVICE_MASK  0x00F00000

#define INDEX_FIFO_VIRTUAL     0x80000000
#define INDEX_USER_MASK        0xFC000000

#define INDEX_CAN_KANAL_A      0x00000000
#define INDEX_CAN_KANAL_B      0x00010000

/***************************************************************/
/*  Typen                                                      */
/***************************************************************/

/******************************************/
/*            CAN Message Type            */
/******************************************/
#define MsgFlags Flags.Long
#define MsgLen Flags.Flag.Len
#define MsgRTR Flags.Flag.RTR
#define MsgEFF Flags.Flag.EFF
#define MsgTxD Flags.Flag.TxD
#define MsgErr Flags.Flag.Error
#define MsgSource Flags.Flag.Source
#define MsgFilHit Flags.Flag.FilHit
#define MsgOV Flags.Flag.OV
#define MsgData Data.Bytes

/*
Byte

 0 -> LEC Last Error Code
        1 = Stuff Error More than 5 equal bits in a sequence have occurred in a part of a
            received message where this is not allowed.
        2 = Form Error A fixed format part of a received frame has the wrong format.
        3 = AckError The message this CAN Core transmitted was not acknowledged by another node.
        4 = Bit1Error During the transmission of a message (with the exception of the
            arbitration field), the device wanted to send a recessive level (bit of logical
            value '1'), but the monitored busvalue was dominant.
        5 = Bit0Error During the transmission of a message (or acknowledge bit or active error
            flag, or overload flag), the device wanted to send a dominant level (data or
            identifier bit logical value '0'), but the monitored Bus value was recessive.
            During busoff recovery this status is set each time a sequence of 11 recessive bits
            has been monitored. This enables the CPU to monitor the proceeding of the busoff
            recovery sequence (indicating the bus is not stuck at dominant level or continuously
            disturbed).
        6 = CRCError The CRC check sum was incorrect in the message received, the CRC received
            for an incoming message does not match with the calculated CRC for the received data.
  1 -> Bus Status (Bit 0 - 3)
        0 = Ok
        1 = Error Warning
        2 = Error Passiv
        3 = Bus Off
       Bus-Failure -> Bit4 
  2 -> Receive Error Counter
  3 -> Transmit Error Counter
  

OV -> FIFO Overflow Frame

Byte 0
  0 -> Source 
         1 = CAN Controller
         2 = Tiny-CAN FIFO
         3 = API
  1,2 -> Messages Lost
    
*/

struct TCanFlagsBits
  {
  unsigned Len:4;    // DLC -> Datenlänge 0 - 8 Byte
  unsigned TxD:1;    // TxD -> 1 = Tx CAN Nachricht, 0 = Rx CAN Nachricht
                     //        Eine Erfolgreich versendete Nachricht wird als Bestätigung
                     //        ins Empfangsfifo zurückgeschrieben
                     //        Nicht alle Module unterstützen diese Funktion u. das 
                     //        Feature muss aktiveirt sein
  unsigned Error:1;  // Error -> 1 = CAN Bus Fehler Nachricht
                     //        Nicht alle Module unterstützen diese Funktion u. das 
                     //        Feature muss aktiveirt sein
  unsigned RTR:1;    // Remote Transmition Request bit -> Kennzeichnet eine RTR Nachricht
  unsigned EFF:1;    // Extended Frame Format bit -> 1 = 29 Bit Id's, 0 = 11 Bit Id's
  unsigned Source:8; // Quelle der Nachricht (Device)
  // Neu
  unsigned FilHit:1; // FilHit -> 1 = Filter Hit
  unsigned OV:1;     // FIFO Overflow
  unsigned Res1:1;
  unsigned Res3:1;
  unsigned Res4:4;
  unsigned Res5:8;
  };

#pragma pack(push, 1)
union TCanFlags
  {
  struct TCanFlagsBits Flag;
  uint32_t Long;
  };
#pragma pack(pop)  

#pragma pack(push, 1)
union TCanData
  {
  char Chars[8];
  unsigned char Bytes[8];
  uint16_t Words[4];
  uint32_t Longs[2];
  };
#pragma pack(pop)  

#pragma pack(push, 1)
struct TTime
  {
  uint32_t Sec;
  uint32_t USec;
  };
#pragma pack(pop)  

#pragma pack(push, 1)
struct TCanMsg
  {
  uint32_t Id;
  union TCanFlags Flags;
  union TCanData Data;
  struct TTime Time;
  };
#pragma pack(pop)


/******************************************/
/*                   CAN FD               */
/******************************************/
#define MsgFD Flags.Flag.FD      // FD Format 
#define MsgBRS Flags.Flag.BRS    // Bit Rate Switch

struct TCanFdFlagsBits
  {
  unsigned Source:8; // Quelle der Nachricht (Device)
  unsigned Len:8;    // DLC -> Datenlänge 0 - 8 Byte
  
  unsigned TxD:1;    // TxD -> 1 = Tx CAN Nachricht, 0 = Rx CAN Nachricht
                     //        Eine Erfolgreich versendete Nachricht wird als Bestätigung
                     //        ins Empfangsfifo zurückgeschrieben
                     //        Nicht alle Module unterstützen diese Funktion u. das 
                     //        Feature muss aktiveirt sein
  unsigned Error:1;  // Error -> 1 = CAN Bus Fehler Nachricht
                     //        Nicht alle Module unterstützen diese Funktion u. das 
                     //        Feature muss aktiveirt sein
  unsigned RTR:1;    // Remote Transmition Request bit -> Kennzeichnet eine RTR Nachricht
  unsigned EFF:1;    // Extended Frame Format bit -> 1 = 29 Bit Id's, 0 = 11 Bit Id's
  unsigned FD:1;     // CAN-FD Frame
  unsigned BRS:1;    // Bit Rate Switch
  unsigned Res0:1;
  unsigned OV:1;     // FIFO Overflow
  
  unsigned Res2:1;
  unsigned Res3:1;
  unsigned Res4:1;
  unsigned Res5:1;
  unsigned Res6:1;
  unsigned Res7:1;
  unsigned Res8:1;
  unsigned FilHit:1; // FilHit -> 1 = Filter Hit
  };

#pragma pack(push, 1)
union TCanFdFlags
  {
  struct TCanFdFlagsBits Flag;
  uint32_t Long;
  };
#pragma pack(pop)  

#pragma pack(push, 1)
union TCanFdData
  {
  char Chars[64];
  unsigned char Bytes[64];
  uint16_t Words[32];
  uint32_t Longs[16];
  };
#pragma pack(pop)

#pragma pack(push, 1)
struct TCanFdMsg
  {
  uint32_t Id;
  union TCanFdFlags Flags;
  union TCanFdData Data;
  struct TTime Time;
  };
#pragma pack(pop)


/******************************************/
/*         CAN Message Filter Type        */
/******************************************/
#define FilFlags Flags.Long
#define FilRTR Flags.Flag.RTR
#define FilEFF Flags.Flag.EFF
#define FilMode Flags.Flag.Mode
#define FilIdMode Flags.Flag.IdMode
#define FilEnable Flags.Flag.Enable

// * = Reserviert, zur Zeit noch unbenutzt

struct TMsgFilterFlagsBits
  {
  // 1. Byte
  unsigned Len:4;       // * Dlc
  unsigned Res:2;       // Reserviert
  unsigned RTR:1;       // Remote Transmition Request
  unsigned EFF:1;       // Extended Frame Format
  // 2. Byte
  unsigned IdMode:2;    // 0 = Maske & Code
                        // 1 = Start & Stop
                        // 2 = Single Id
  unsigned DLCCheck:1;  // *
  unsigned DataCheck:1; // *
  unsigned Res1:4;
  // 3. Byte
  unsigned Res2:8;
  // 4. Byte
  unsigned Type:4;      // 0 = Single Puffer
  unsigned Res3:2;
  unsigned Mode:1;      // 0 = Message entfernen
                        // 1 = Message nicht entfernen
  unsigned Enable:1;    // 0 = Filter sperren
                        // 1 = Filter freigeben
  };

#pragma pack(push, 1)
union TMsgFilterFlags
  {
  struct TMsgFilterFlagsBits Flag;
  uint32_t Long;
  };
#pragma pack(pop)  

#pragma pack(push, 1)
struct TMsgFilter
  {                     // IdMode    -> Maske & Code | Start & Stop | Single Id
                        // --------------------------+--------------+-----------
  uint32_t Maske;       // Filter-Id ->    Maske     |     Stop     |  
  uint32_t Code;        // Filter-Id ->    Code      |     Start    |    Id
  union TMsgFilterFlags Flags;  
  union TCanData Data;  // *
  };
#pragma pack(pop)  

struct TCanIndexSource
  {
  // 1. u. 2 Byte
  unsigned SubIndex:16;
  // 3. Byte
  unsigned Source:8;
  // 4. Byte
  unsigned TxD:1;
  unsigned Soft:1;
  unsigned User:5;
  unsigned Virtual:1;
  };

struct TCanIndexBits
  {
  // 1. u. 2 Byte
  unsigned SubIndex:16;
  // 3. Byte
  unsigned Channel:4;
  unsigned Device:4;
  // 4. Byte
  unsigned TxD:1;
  unsigned Soft:1;
  unsigned User:5;
  unsigned Virtual:1;
  };

union TCanIndex
  {
  struct TCanIndexBits Item;
  struct TCanIndexSource SrcItem;
  uint32_t Long;
  };


// <*> neu, übertragen
#ifdef ENABLE_RS485_SUPPORT

#define RS485_TX_STATUS_FINISH  1
#define RS485_TX_STATUS_ERROR   2

#define RS485_RX_STATUS_FINISH   1
#define RS485_RX_STATUS_TIMEOUT  2
#define RS485_RX_STATUS_ERROR    3
#define RS485_RX_STATUS_OV       4

typedef struct _TRS485MsData TRS485MsData; 

#pragma pack(push, 1)
struct _TRS485MsData
  {
  uint16_t Flags;
  uint8_t TxStatus;
  uint8_t RxStatus;
  uint16_t TxSize;
  uint8_t TxData[4096];
  uint16_t RxSize;
  uint8_t RxData[4096];
  };
#pragma pack(pop)

#endif


M_FORCE_INLINE void CanFdToCan(struct TCanMsg *dst, const struct TCanFdMsg *src) 
{
uint8_t len;

dst->Id = src->Id;
len = src->MsgLen;
dst->MsgFlags = (src->MsgFlags >> 12) & 0x000000F0;
dst->MsgOV = src->MsgOV;
if (len > 8)
  len = 8;
dst->MsgLen = len;
dst->MsgSource = src->MsgSource;
dst->MsgFilHit = src->MsgFilHit;  
dst->Data.Longs[0] = src->Data.Longs[0]; 
dst->Data.Longs[1] = src->Data.Longs[1]; 
dst->Time.Sec = src->Time.Sec;
dst->Time.USec = src->Time.USec;
}


M_FORCE_INLINE void CanToCanFd(struct TCanFdMsg *dst, const struct TCanMsg *src) 
{
dst->Id = src->Id;
dst->MsgFlags = (src->MsgFlags << 12) & 0x000F0000;
dst->MsgOV = src->MsgOV;
dst->MsgLen = src->MsgLen;
dst->MsgSource = src->MsgSource;
dst->MsgFilHit = src->MsgFilHit;  
dst->Data.Longs[0] = src->Data.Longs[0]; 
dst->Data.Longs[1] = src->Data.Longs[1]; 
dst->Time.Sec = src->Time.Sec;
dst->Time.USec = src->Time.USec;
}


#ifdef __cplusplus
  }
#endif


#endif
