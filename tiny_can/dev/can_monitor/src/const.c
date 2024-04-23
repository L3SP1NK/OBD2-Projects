/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2005 Klaus Demlehner (klaus@mhs-elektronik.de)           */
/*   Tiny-CAN Project Homepage: http://www.mhs-elektronik.de              */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */

/**************************************************************************/
/*                             Konstanten                                 */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Konstanten Variablen (Strings, ...) des Programms   */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : const.c                                             */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include <string.h>
#include "global.h"
#include "const.h"
#include "about_dlg.h"
#include "splash.h"
#include "main.h"
#include "support.h"


const struct TSplashData SplashData = {
  /* .Grafik = */ "splash.jpg",
  /* .ShowTime = */ 3};

const struct TAboutTxt AboutTxt = {
  /* .Programm =   */ "Tiny-CAN Monitor",
  /* .LogoFile =   */ "tiny-can.png",
  /* .Version =    */ "Version: 2.22",
  /* .Copyright =  */ "Copyright (C)2008-2010 MHS-Elektronik GmbH + Co. KG, Germany",
  /* .Autor =      */ "Klaus Demlehner",
  /* .Email =      */ "klaus@mhs-elektronik.de",
  /* .Homepage =   */ "www.MHS-Elektronik.de",
  /* .ShortLizenz = */ "This Programm comes with ABSOLUTELY NO WARRANTY; for details type \"GNU Licence\".\n" \
     "This is free software, and you are welcomme to redistribute it under certain\n" \
     "conditions; type \"GNU Licence\" for details",
  /* .LizenzFile = */ "COPYING"};

GtkWidget **SensitiveListe[SENSITIVE_LISTE_SIZE] =
  {&MainWin.MDatei,
   &MainWin.MCan,
   &MainWin.MMakro,
   &MainWin.MFilter,
   &MainWin.MAnsicht,
   &MainWin.MOptionen,
   &MainWin.MConnect,
   &MainWin.MDeviceInfo,
   &MainWin.MHilfe,
   &MainWin.MainToolbar,
   &MainWin.CanTxWindow,
   &MainWin.MFilMessages,
   &MainWin.CanStartStopButton,
   &MainWin.CanResetButton,
   &MainWin.MakroSendenButton,
   &MainWin.MakroPasteButton,
   NULL};

unsigned char SelectAllOffListe[] =
  {0,   // Menue Datei
   0,   // Menue MCan
   0,   // Menue Makro
   0,   // Menue Filter
   0,   // Menue Ansicht
   0,   // Menue Optionen
   0,   // Menue Connect
   0,   // Menue Device Info
   0,   // Menue Hilfe
   0,   // Main Toolbar
   0,   // Senden Toolbar
   0,   // Menue Filter Messages
   0,   // CAN Start/Stop Button
   0,   // CAN Reset Button
   0,   // Makro Senden Button
   0};  // Makro Paste Button

unsigned char SelectAllOnListe[] =
  {1,   // Menue Datei
   1,   // Menue MCan
   1,   // Menue Makro
   1,   // Menue Filter
   1,   // Menue Ansicht
   1,   // Menue Optionen
   1,   // Menue Connect
   1,   // Menue Device Info
   1,   // Menue Hilfe
   1,   // Main Toolbar
   1,   // Senden Toolbar
   1,   // Menue Filter Messages
   1,   // CAN Start/Stop Button
   1,   // CAN Reset Button
   1,   // Makro Senden Button
   1};  // Makro Paste Button

unsigned char SelectAllOnMakroOffListe[] =
  {1,   // Menue Datei
   1,   // Menue MCan
   1,   // Menue Makro
   1,   // Menue Filter
   1,   // Menue Ansicht
   1,   // Menue Optionen
   1,   // Menue Connect
   1,   // Menue Device Info
   1,   // Menue Hilfe
   1,   // Main Toolbar
   1,   // Senden Toolbar
   1,   // Menue Filter Messages
   1,   // CAN Start/Stop Button
   1,   // CAN Reset Button
   0,   // Makro Senden Button
   0};  // Makro Paste Button

unsigned char SelectCanOffListe[] =
  {1,   // Menue Datei
   0,   // Menue MCan
   1,   // Menue Makro
   1,   // Menue Filter
   1,   // Menue Ansicht
   1,   // Menue Optionen
   1,   // Menue Connect
   0,   // Menue Device Info
   1,   // Menue Hilfe
   1,   // Main Toolbar
   0,   // Senden Toolbar
   0,   // Menue Filter Messages
   0,   // CAN Start/Stop Button
   0,   // CAN Reset Button
   0,   // Makro Senden Button
   0};  // Makro Paste Button


unsigned char SelectAllCanOffListe[] =
  {1,   // Menue Datei
   0,   // Menue MCan
   1,   // Menue Makro
   1,   // Menue Filter
   1,   // Menue Ansicht
   1,   // Menue Optionen
   0,   // Menue Connect
   0,   // Menue Device Info
   1,   // Menue Hilfe
   1,   // Main Toolbar
   0,   // Senden Toolbar
   0,   // Menue Filter Messages
   0,   // CAN Start/Stop Button
   0,   // CAN Reset Button
   0,   // Senden Window Button
   0,   // Makro Senden Button
   0};  // Makro Paste Button
