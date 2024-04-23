/***************************************************************************
                          tcan_m2.c  -  description
                             -------------------
    begin             : 13.07.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include "gtk_util.h"
#include "hw_types.h"
#include "gui_help.h"
#include "gui.h"
#include "tcan_m2.h"

#define IO_PIN_COUNT 32


static const struct TIoPinDefs TCanM2IoPinDefs[] = {
  { 0, PIN_IN_OUT | PIN_KEY | PIN_A_OUT,  1, 1, "I/O 0",  "In 0",  "Out 0",  "Key 0",  "Switch 0",  "Analog Out 0"},
  { 1, PIN_IN_OUT | PIN_KEY,              1, 2, "I/O 1",  "In 1",  "Out 1",  "Key 1",  "Switch 1",  ""},
  { 2, PIN_IN_OUT | PIN_KEY,              2, 1, "I/O 2",  "In 2",  "Out 2",  "Key 2",  "Switch 2",  ""},
  { 3, PIN_IN_OUT | PIN_KEY,              2, 2, "I/O 3",  "In 3",  "Out 3",  "Key 3",  "Switch 3",  ""},
  { 4, PIN_IN_OUT | PIN_KEY | PIN_A_OUT,  3, 1, "I/O 4",  "In 4",  "Out 4",  "Key 4",  "Switch 4",  "Analog Out 1"},
  { 5, PIN_IN_OUT | PIN_KEY,              3, 2, "I/O 5",  "In 5",  "Out 5",  "Key 5",  "Switch 5",  ""},
  { 6, PIN_IN_OUT | PIN_KEY,              4, 1, "I/O 6",  "In 6",  "Out 6",  "Key 6",  "Switch 6",  ""},
  { 7, PIN_IN_OUT | PIN_KEY,              4, 2, "I/O 7",  "In 7",  "Out 7",  "Key 7",  "Switch 7",  ""},
  { 8, PIN_IN_OUT | PIN_KEY,              5, 1, "I/O 8",  "In 8",  "Out 8",  "Key 8",  "Switch 8",  ""},
  { 9, PIN_IN_OUT | PIN_KEY,              5, 2, "I/O 9",  "In 9",  "Out 9",  "Key 9",  "Switch 9",  ""},
  {10, PIN_IN_OUT | PIN_KEY,              6, 1, "I/O 10", "In 10", "Out 10", "Key 10", "Switch 10", ""},
  {11, PIN_IN_OUT | PIN_KEY,              6, 2, "I/O 11", "In 11", "Out 11", "Key 11", "Switch 11", ""},
  {12, PIN_IN_OUT | PIN_KEY,              7, 1, "I/O 12", "In 12", "Out 12", "Key 12", "Switch 12", ""},
  {13, PIN_IN_OUT | PIN_KEY,              7, 2, "I/O 13", "In 13", "Out 13", "Key 13", "Switch 13", ""},
  {14, PIN_IN_OUT | PIN_KEY,              8, 1, "I/O 14", "In 14", "Out 14", "Key 14", "Switch 14", ""},
  {15, PIN_IN_OUT | PIN_KEY,              8, 2, "I/O 15", "In 15", "Out 15", "Key 15", "Switch 15", ""},
  {16, PIN_IN_OUT | PIN_ENC_A,            9, 1, "I/O 16", "In 16", "Out 16", "",       "",          "Encoder 0 - A"},
  {17, PIN_IN_OUT | PIN_ENC_B,            9, 2, "I/O 17", "In 17", "Out 17", "",       "",          "Encoder 0 - B"},
  {18, PIN_IN_OUT | PIN_ENC_A,           10, 1, "I/O 18", "In 18", "Out 18", "",       "",          "Encoder 1 - A"},
  {19, PIN_IN_OUT | PIN_ENC_B,            1, 4, "I/O 19", "In 19", "Out 19", "",       "",          "Encoder 1 - B"},
  {20, PIN_IN_OUT | PIN_ENC_A,            1, 5, "I/O 20", "In 20", "Out 20", "",       "",          "Encoder 2 - A"},
  {21, PIN_IN_OUT | PIN_ENC_B,            2, 4, "I/O 21", "In 21", "Out 21", "",       "",          "Encoder 2 - B"},
  {22, PIN_IN_OUT | PIN_A_IN,             2, 5, "I/O 22", "In 22", "Out 22", "",       "",          "Analog In 0"},
  {23, PIN_IN_OUT | PIN_A_IN,             3, 4, "I/O 23", "In 23", "Out 23", "",       "",          "Analog In 1"},
  {24, PIN_IN_OUT | PIN_A_IN,             3, 5, "I/O 24", "In 24", "Out 24", "",       "",          "Analog In 2"},
  {25, PIN_IN_OUT | PIN_A_IN,             4, 4, "I/O 25", "In 25", "Out 25", "",       "",          "Analog In 3"},
  {26, PIN_IN_OUT | PIN_A_IN,             4, 5, "I/O 26", "In 26", "Out 26", "",       "",          "Analog In 4"},
  {27, PIN_IN_OUT | PIN_A_IN,             5, 4, "I/O 27", "In 27", "Out 27", "",       "",          "Analog In 5"},
  {28, PIN_IN_OUT | PIN_ENC_A,            5, 5, "I/O 28", "In 28", "Out 28", "",       "",          "Encoder 3 - A"},
  {29, PIN_IN_OUT | PIN_ENC_B,            6, 4, "I/O 29", "In 29", "Out 29", "",       "",          "Encoder 3 - B"},
  {30, PIN_IN_OUT,                        6, 5, "I/O 30", "In 30", "Out 30", "",       "",          ""},
  {31, PIN_IN_OUT,                        7, 4, "I/O 31", "In 31", "Out 31", "",       "",          ""}};


static const struct TKeyPannel TCanM2KeyPannel = {8, 4, 3, 2};


struct TCanHw *TCanM2Create(void)
{
guint i;
struct TCanHw *hw;

if (!(hw = (struct TCanHw *)g_malloc0(sizeof(struct TCanHw))))
  return(NULL);
hw->HardwareType = HARDWARE_TYPE_TCAN_M2;
hw->BaseWdg = NULL;
hw->TableRows = 10;
hw->TableColumns = 5;
hw->DeviceIndex = INDEX_INVALID;
hw->IoPinsCount = IO_PIN_COUNT;
hw->IoPinDefs = TCanM2IoPinDefs;
hw->KeyPannel = &TCanM2KeyPannel;
for (i = 0; i < hw->IoPinsCount; i++)
  {
  hw->IoPin[i].Mode = PIN_CFG_DIG_IN;
  hw->IoPin[i].Name = NULL;
  hw->IoPin[i].DefaultName = 1;
  hw->IoPin[i].In = 0;
  hw->IoPin[i].AIn = 0;
  hw->IoPin[i].OutWidget = NULL;
  hw->IoPin[i].InBoxWidget = NULL;
  hw->IoPin[i].InWidget = NULL;
  hw->IoPin[i].AInBoxWidget = NULL;
  hw->IoPin[i].AInWidget = NULL;
  hw->IoPin[i].AOutBoxWidget = NULL;
  hw->IoPin[i].AOutWidget = NULL;
  hw->IoPin[i].ConfigBoxWidget = NULL;
  hw->IoPin[i].ConfigWidget = NULL;
  hw->IoPin[i].InNameWidget = NULL;
  hw->IoPin[i].AInNameWidget = NULL;
  hw->IoPin[i].AOutNameWidget = NULL;
  IoPinSetDefaultName(hw, i);
  }
return(hw);
}


void TCanM2Destroy(struct TCanHw *hw)
{
guint i;

if (hw)
  {
  for (i = 0; i < hw->IoPinsCount; i++)
    safe_free(hw->IoPin[i].Name);
  g_free(hw);
  }
}


