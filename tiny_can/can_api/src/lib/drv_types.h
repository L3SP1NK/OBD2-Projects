#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__

//#include "global.h"<*>

#ifdef __cplusplus
  extern "C" {
#endif

/***************************************************************/
/* Define Makros                                               */
/***************************************************************/

// CAN Übertragungsgeschwindigkeit
#define CAN_10K_BIT   10    // 10 kBit/s
#define CAN_20K_BIT   20    // 20 kBit/s
#define CAN_50K_BIT   50    // 50 kBit/s
#define CAN_100K_BIT  100   // 100 kBit/s
#define CAN_125K_BIT  125   // 125 kBit/s
#define CAN_250K_BIT  250   // 250 kBit/s
#define CAN_500K_BIT  500   // 500 kBit/s
#define CAN_800K_BIT  800   // 800 kBit/s
#define CAN_1M_BIT    1000  // 1 MBit/s

// CAN Bus Mode
#define OP_CAN_NO_CHANGE         0  // Aktuellen Zustand nicht ändern
#define OP_CAN_START             1  // Startet den CAN-Bus
#define OP_CAN_STOP              2  // Stopt den CAN-Bus
#define OP_CAN_RESET             3  // Reset CAN Controller (BusOff löschen)
#define OP_CAN_START_LOM         4  // Startet den CAN-Bus im Silent Mode (Listen Only Mode)
#define OP_CAN_START_NO_RETRANS  5  // Startet den CAN-Bus im Automatic Retransmission disable Mode
#define OP_CAN_ECU_FLASH_MODE    6  // Start im ECU Flash Mode

#define CAN_CMD_NONE                 0x0000
#define CAN_CMD_RXD_OVERRUN_CLEAR    0x0001
#define CAN_CMD_RXD_FIFOS_CLEAR      0x0002
#define CAN_CMD_TXD_OVERRUN_CLEAR    0x0004
#define CAN_CMD_TXD_FIFOS_CLEAR      0x0008
#define CAN_CMD_HW_FILTER_CLEAR      0x0010
#define CAN_CMD_SW_FILTER_CLEAR      0x0020
#define CAN_CMD_TXD_PUFFERS_CLEAR    0x0040
// <*> neu
#define CAN_CMD_START_DATA_TRANSFER  0x1000
#define CAN_CMD_CANCEL_DATA_TRANSFER 0x2000
#define CAN_CMD_START_TEST           0xE000
#define CAN_CMD_STOP_TEST            0xF000

#define CAN_CMD_ALL_CLEAR            0x0FFF


// DrvStatus
#define DRV_NOT_LOAD              0  // Die Treiber DLL wurde noch nicht geladen
#define DRV_STATUS_NOT_INIT       1  // Treiber noch nicht Initialisiert (Funktion "CanInitDrv" noch nicht aufgerufen)
#define DRV_STATUS_INIT           2  // Treiber erfolgrich Initialisiert
#define DRV_STATUS_PORT_NOT_OPEN  3  // Die Schnittstelle wurde nicht geöffnet
#define DRV_STATUS_PORT_OPEN      4  // Die Schnittstelle wurde geöffnet
#define DRV_STATUS_DEVICE_FOUND   5  // Verbindung zur Hardware wurde Hergestellt
#define DRV_STATUS_CAN_OPEN       6  // Device wurde geöffnet und erfolgreich Initialisiert
#define DRV_STATUS_CAN_RUN_TX     7  // CAN Bus RUN nur Transmitter (wird nicht verwendet !)
#define DRV_STATUS_CAN_RUN        8  // CAN Bus RUN

// CanStatus
#define CAN_STATUS_OK          0     // CAN-Controller: Ok
#define CAN_STATUS_ERROR       1     // CAN-Controller: CAN Error
#define CAN_STATUS_WARNING     2     // CAN-Controller: Error warning
#define CAN_STATUS_PASSIV      3     // CAN-Controller: Error passiv
#define CAN_STATUS_BUS_OFF     4     // CAN-Controller: Bus Off
#define CAN_STATUS_UNBEKANNT   5     // CAN-Controller: Status Unbekannt

// Neu für Low-Speed CAN, TJA1055 Fehler
#define BUS_FAILURE            0x10

// Fifo Status
#define FIFO_OK                 0 // Fifo-Status: Ok
#define FIFO_HW_OVERRUN         1 // Fifo-Status: Hardware Fifo Überlauf
#define FIFO_SW_OVERRUN         2 // Fifo-Status: Software Fifo Überlauf
#define FIFO_HW_SW_OVERRUN      3 // Fifo-Status: Hardware & Software Fifo Überlauf
#define FIFO_STATUS_UNBEKANNT   4 // Fifo-Status: Unbekannt

// Makros für SetEvent
#define EVENT_ENABLE_PNP_CHANGE          0x0001
#define EVENT_ENABLE_STATUS_CHANGE       0x0002
#define EVENT_ENABLE_RX_FILTER_MESSAGES  0x0004
#define EVENT_ENABLE_RX_MESSAGES         0x0008
#define EVENT_ENABLE_ALL                 0x00FF

#define EVENT_DISABLE_PNP_CHANGE         0x0100
#define EVENT_DISABLE_STATUS_CHANGE      0x0200
#define EVENT_DISABLE_RX_FILTER_MESSAGES 0x0400
#define EVENT_DISABLE_RX_MESSAGES        0x0800
#define EVENT_DISABLE_ALL                0xFF00

// MHS (EV)ent (S)ource
#define MHS_EVS_STATUS  1
#define MHS_EVS_PNP     2
#define MHS_EVS_OBJECT  3

#define MHS_EVS_DIN     4     
#define MHS_EVS_ENC     5
#define MHS_EVS_KEY     6

#define LOG_MESSAGE          0x00000001
#define LOG_STATUS           0x00000002
#define LOG_RX_MSG           0x00000004
#define LOG_TX_MSG           0x00000008
#define LOG_API_CALL         0x00000010
#define LOG_API_CALL_RX      0x00000020
#define LOG_API_CALL_TX      0x00000040
#define LOG_API_CALL_STATUS  0x00000080
#define LOG_ERROR            0x00000100
#define LOG_WARN             0x00000200
#define LOG_ERR_MSG          0x00000400
#define LOG_OV_MSG           0x00000800
#define LOG_USB              0x00008000
#define LOG_DEBUG            0x08000000

#define LOG_WITH_TIME    0x40000000
#define LOG_DISABLE_SYNC 0x80000000

// <*> neu
#define TCAN_INFO_KEY_HW_SNR         0x00000000  // Hardware Snr
#define TCAN_INFO_KEY_HW_ID_STR      0x00000001  // Hardware ID String 
#define TCAN_INFO_KEY_HW_BIOS_STR    0x00000002  // Bios ID String
#define TCAN_INFO_KEY_HW_REVISION    0x00000003  // Hardware Revision    
#define TCAN_INFO_KEY_HW_DATE        0x00000004  // Fertigungsdatum
#define TCAN_INFO_KEY_HW_VARIANT_STR 0x00000005  // Hardware Variante
     
#define TCAN_INFO_KEY_HW_CAN_COUNT   0x00008000  // Anzahl CAN Interfaces
#define TCAN_INFO_KEY_HW_CAN_DRV     0x00008010  // Treiber              
#define TCAN_INFO_KEY_HW_CAN_OPTO    0x00008020  // Opto                 
#define TCAN_INFO_KEY_HW_CAN_TERM    0x00008030  // Term                 
#define TCAN_INFO_KEY_HW_CAN_HS      0x00008040  // HighSpeed
#define TCAN_INFO_KEY_HW_I2C_CNT     0x00008100  // Anzahl I2C Interfaces
#define TCAN_INFO_KEY_HW_SPI_CNT     0x00008200  // Anzahl SPI Interfaces

#define TCAN_INFO_KEY_FW_ID          0x00001000  // ID                   
#define TCAN_INFO_KEY_FW_ID_STR      0x00001001  // ID String            
#define TCAN_INFO_KEY_FW_VERSION     0x00001002  // Version              
#define TCAN_INFO_KEY_FW_VERSION_STR 0x00001003  // Version String       
#define TCAN_INFO_KEY_FW_AUTOR       0x00001004  // Autor                
#define TCAN_INFO_KEY_FW_OPTIOS      0x00001005  // Optionen             
#define TCAN_INFO_KEY_FW_SNR         0x00001006  // Snr                  
            
#define TCAN_INFO_KEY_FW_CAN_FLAGS   0x00008001  // CAN Features Flags
#define TCAN_INFO_KEY_FW_CAN_FLAGS2  0x00008002  // CAN Features Flags2 // <*>
#define TCAN_INFO_KEY_FW_CAN_CLOCK1  0x00008003  // CAN Clock 1
#define TCAN_INFO_KEY_FW_CAN_CLOCK2  0x00008004  // CAN Clock 2
#define TCAN_INFO_KEY_FW_CAN_CLOCK3  0x00008005  // CAN Clock 3
#define TCAN_INFO_KEY_FW_CAN_CLOCK4  0x00008006  // CAN Clock 4
#define TCAN_INFO_KEY_FW_CAN_CLOCK5  0x00008007  // CAN Clock 5
#define TCAN_INFO_KEY_FW_CAN_CLOCK6  0x00008008  // CAN Clock 6             
#define TCAN_INFO_KEY_FW_PUFFER_CNT  0x00008050  // Anzahl Interval Puffer
#define TCAN_INFO_KEY_FW_FILTER_CNT  0x00008060  // Anzahl Filter        

#define TCAN_INFO_KEY_OPEN_INDEX     0x01000001
#define TCAN_INFO_KEY_HARDWARE_ID    0x01000002
#define TCAN_INFO_KEY_HARDWARE       0x01000003
#define TCAN_INFO_KEY_VENDOR         0x01000004

#define TCAN_INFO_KEY_DEVICE_NAME    0x01000005
#define TCAN_INFO_KEY_SERIAL_NUMBER  0x01000006

#define TCAN_INFO_KEY_CAN_FEATURES   0x01000007
#define TCAN_INFO_KEY_CAN_CHANNELS   0x01000008
#define TCAN_INFO_KEY_RX_FILTER_CNT  0x01000009 
#define TCAN_INFO_KEY_TX_BUFFER_CNT  0x0100000A
#define TCAN_INFO_KEY_CAN_CLOCKS     0x0100000B
#define TCAN_INFO_KEY_CAN_CLOCK1     0x0100000C
#define TCAN_INFO_KEY_CAN_CLOCK2     0x0100000D
#define TCAN_INFO_KEY_CAN_CLOCK3     0x0100000E
#define TCAN_INFO_KEY_CAN_CLOCK4     0x0100000F
#define TCAN_INFO_KEY_CAN_CLOCK5     0x01000010
#define TCAN_INFO_KEY_CAN_CLOCK6     0x01000011

#define TCAN_INFO_KEY_API_VERSION    0x02000001
#define TCAN_INFO_KEY_DLL            0x02000002
#define TCAN_INFO_KEY_CFG_APP        0x02000003


#define CAN_FEATURE_LOM          0x0001  // Silent Mode (LOM = Listen only Mode)
#define CAN_FEATURE_ARD          0x0002  // Automatic Retransmission disable
#define CAN_FEATURE_TX_ACK       0x0004  // TX ACK (Gesendete Nachrichten bestätigen)
#define CAN_FEATURE_ERROR_MSGS   0x0008  // Error Messages Support
#define CAN_FEATURE_FD_HARDWARE  0x0010  // CAN-FD Hardware
#define CAN_FEATURE_FIFO_OV_MODE 0x0020  // FIFO OV Mode (Auto Clear, OV CAN Messages)
#define CAN_FEATURE_ECU_FLASH    0x0040  // Hardware beschleunigung für ISO-TP ECU-Flash programmierung
#define CAN_FEATURE_CAN_TEST     0x4000  // Tiny-CAN Tester Firmware
#define CAN_FEATURE_HW_TIMESTAMP 0x8000  // Hardware Time Stamp

#define DEVICE_NAME_MAX_SIZE   255
#define SERIAL_NUMBER_MAX_SIZE 16 
#define DESCRIPTION_MAX_SIZE   64

#define TIME_STAMP_OFF        0
#define TIME_STAMP_SOFT       1
#define TIME_STAMP_HW_UNIX    2
#define TIME_STAMP_HW         3
#define TIME_STAMP_HW_SW_UNIX 4

/***************************************************************/
/*  Typen                                                      */
/***************************************************************/
#pragma pack(push, 1)
struct TModulFeatures  // In tcan_drv.h kopiert
  {
  uint32_t CanClock;
  uint32_t Flags;
  uint32_t CanChannelsCount;
  uint32_t HwRxFilterCount;
  uint32_t HwTxPufferCount;
  };
#pragma pack(pop)  


#pragma pack(push, 1)
struct TCanDevicesList
  {
  uint32_t TCanIdx;
  uint32_t HwId;
  char DeviceName[DEVICE_NAME_MAX_SIZE];
  char SerialNumber[SERIAL_NUMBER_MAX_SIZE];
  char Description[DESCRIPTION_MAX_SIZE];
  struct TModulFeatures ModulFeatures;
  };
#pragma pack(pop)  


#pragma pack(push, 1)
struct TCanDeviceInfo
  {
  uint32_t HwId;
  uint32_t FirmwareVersion;
  uint32_t FirmwareInfo;
  char SerialNumber[SERIAL_NUMBER_MAX_SIZE];
  char Description[DESCRIPTION_MAX_SIZE];
  struct TModulFeatures ModulFeatures;
  };
#pragma pack(pop)

#pragma pack(push, 1)
struct TCanInfoVar
  {
  uint32_t Key;
  uint32_t Type; 
  uint32_t Size;
  char Data[255];
  };
#pragma pack(pop)

#pragma pack(push, 1)
struct TCanInfoVarList
  {
  uint32_t Size;
  struct TCanInfoVar *List;
  };
#pragma pack(pop)


/******************************************/
/*             Device Status              */
/******************************************/
#pragma pack(push, 1)
struct TDeviceStatus
  {
  int32_t DrvStatus;
  unsigned char CanStatus;
  unsigned char FifoStatus;
  };
#pragma pack(pop)

#ifdef __cplusplus
  }
#endif

#endif
