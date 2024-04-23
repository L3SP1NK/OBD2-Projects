/***************************************************************************
                          tcan_m3.c  -  description
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
#include "tcan_m3.h"


#define IO_PIN_COUNT 46


static const struct TIoPinDefs TCanM3IoPinDefs[] = {
  { 0, PIN_IN_OUT,              1, 1, "I/O 0",  "In 0",  "Out 0",  "Key 22", "Switch 22", ""},
  { 1, PIN_IN_OUT | PIN_A_OUT,  1, 2, "I/O 1",  "In 1",  "Out 1",  "Key 21", "Switch 21", "Analog Out 0"},
  { 2, PIN_IN_OUT | PIN_A_OUT,  2, 1, "I/O 2",  "In 2",  "Out 2",  "Key 20", "Switch 20", "Analog Out 1"},
  { 3, PIN_IN_OUT,              2, 2, "I/O 3",  "In 3",  "Out 3",  "Key 19", "Switch 19", ""},
  { 4, PIN_IN_OUT,              3, 1, "I/O 4",  "In 4",  "Out 4",  "Key 18", "Switch 18", ""},
  { 5, PIN_IN_OUT,              3, 2, "I/O 5",  "In 5",  "Out 5",  "Key 17", "Switch 17", ""},
  { 6, PIN_IN_OUT,              4, 1, "I/O 6",  "In 6",  "Out 6",  "Key 16", "Switch 16", ""},
  { 7, PIN_IN_OUT,              4, 2, "I/O 7",  "In 7",  "Out 7",  "Key 15", "Switch 15", ""},
  { 8, PIN_IN_OUT,              5, 1, "I/O 8",  "In 8",  "Out 8",  "Key 14", "Switch 14", ""},
  { 9, PIN_IN_OUT,              5, 2, "I/O 9",  "In 9",  "Out 9",  "Key 13", "Switch 13", ""},
  {10, PIN_IN_OUT,              6, 1, "I/O 10", "In 10", "Out 10", "Key 12", "Switch 12", ""},
  {11, PIN_IN_OUT,              6, 2, "I/O 11", "In 11", "Out 11", "Key 11", "Switch 11", ""},
  {12, PIN_IN_OUT,              7, 1, "I/O 12", "In 12", "Out 12", "Key 10", "Switch 10", ""},
  {13, PIN_IN_OUT,              7, 2, "I/O 13", "In 13", "Out 13", "Key 9",  "Switch 9",  ""},
  {14, PIN_IN_OUT,              8, 1, "I/O 14", "In 14", "Out 14", "Key 8",  "Switch 8",  ""},
  {15, PIN_IN_OUT,              8, 2, "I/O 15", "In 15", "Out 15", "Key 7",  "Switch 7",  ""},
  {16, PIN_IN_OUT,              9, 1, "I/O 16", "In 16", "Out 16", "Key 6",  "Switch 6",  ""},
  {17, PIN_IN_OUT,              9, 2, "I/O 17", "In 17", "Out 17", "Key 5",  "Switch 5",  ""},
  {18, PIN_IN_OUT,             10, 1, "I/O 18", "In 18", "Out 18", "Key 4",  "Switch 4",  ""},
  {19, PIN_IN_OUT,             10, 2, "I/O 19", "In 19", "Out 19", "Key 3",  "Switch 3",  ""},
  {20, PIN_IN_OUT,             11, 1, "I/O 20", "In 20", "Out 20", "Key 2",  "Switch 2",  ""},
  {21, PIN_IN_OUT,             11, 2, "I/O 21", "In 21", "Out 21", "Key 1",  "Switch 1",  ""},
  {22, PIN_IN_OUT,             12, 1, "I/O 22", "In 22", "Out 22", "Key 0",  "Switch 0",  ""},
  {23, PIN_IN_OUT,             12, 2, "I/O 23", "In 23", "Out 23", "",       "",          ""},
  {24, PIN_IN_OUT,             13, 1, "I/O 24", "In 24", "Out 24", "",       "",          ""},
  {25, PIN_IN_OUT,             13, 2, "I/O 25", "In 25", "Out 25", "",       "",          ""},
  {26, PIN_IN_OUT,             14, 1, "I/O 26", "In 26", "Out 26", "",       "",          ""},
  {27, PIN_IN_OUT,             14, 2, "I/O 27", "In 27", "Out 27", "",       "",          ""},
  {28, PIN_IN_OUT,             15, 1, "I/O 28", "In 28", "Out 28", "",       "",          ""},
  {29, PIN_IN_OUT,             15, 2, "I/O 29", "In 29", "Out 29", "",       "",          ""},
  {30, PIN_IN_OUT | PIN_A_IN,   1, 4, "I/O 30", "In 30", "Out 30", "Key 23", "Switch 23", "Analog In 0"},
  {31, PIN_IN_OUT | PIN_A_IN,   1, 5, "I/O 31", "In 31", "Out 31", "Key 24", "Switch 24", "Analog In 1"},
  {32, PIN_IN_OUT | PIN_A_IN,   2, 4, "I/O 32", "In 32", "Out 32", "Key 25", "Switch 25", "Analog In 2"},
  {33, PIN_IN_OUT | PIN_A_IN,   2, 5, "I/O 33", "In 33", "Out 33", "Key 26", "Switch 26", "Analog In 3"},
  {34, PIN_IN_OUT | PIN_A_IN,   3, 4, "I/O 34", "In 34", "Out 34", "Key 27", "Switch 27", "Analog In 4"},
  {35, PIN_IN_OUT | PIN_A_IN,   3, 5, "I/O 35", "In 35", "Out 35", "Key 28", "Switch 28", "Analog In 5"},
  {36, PIN_IN_OUT | PIN_A_IN,   4, 4, "I/O 36", "In 36", "Out 36", "Key 29", "Switch 29", "Analog In 6"},
  {37, PIN_IN_OUT | PIN_A_IN,   4, 5, "I/O 37", "In 37", "Out 37", "Key 30", "Switch 30", "Analog In 7"},
  {38, PIN_IN_OUT | PIN_ENC_A,  5, 4, "I/O 38", "In 38", "Out 38", "Key 31", "Switch 31", "Encoder 0 - A"},
  {39, PIN_IN_OUT | PIN_ENC_B,  5, 5, "I/O 39", "In 39", "Out 39", "",       "",          "Encoder 0 - B"},
  {40, PIN_IN_OUT | PIN_ENC_A,  6, 4, "I/O 40", "In 40", "Out 40", "",       "",          "Encoder 1 - A"},
  {41, PIN_IN_OUT | PIN_ENC_B,  6, 5, "I/O 41", "In 41", "Out 41", "",       "",          "Encoder 1 - B"},
  {42, PIN_IN_OUT | PIN_ENC_A,  7, 4, "I/O 42", "In 42", "Out 42", "",       "",          "Encoder 2 - A"},
  {43, PIN_IN_OUT | PIN_ENC_B,  7, 5, "I/O 43", "In 43", "Out 43", "",       "",          "Encoder 2 - B"},
  {44, PIN_IN_OUT | PIN_ENC_A,  8, 4, "I/O 44", "In 44", "Out 44", "",       "",          "Encoder 3 - A"},
  {45, PIN_IN_OUT | PIN_ENC_B,  8, 5, "I/O 45", "In 45", "Out 45", "",       "",          "Encoder 3 - B"}};

static const struct TKeyPannel TCanM3KeyPannel = {9, 4, 7, 2};


struct TCanHw *TCanM3Create(void)
{
guint i;
struct TCanHw *hw;

if (!(hw = (struct TCanHw *)g_malloc0(sizeof(struct TCanHw))))
  return(NULL);
hw->HardwareType = HARDWARE_TYPE_TCAN_M3;
hw->BaseWdg = NULL;
hw->TableRows = 15;
hw->TableColumns = 5;
hw->DeviceIndex = INDEX_INVALID;
hw->IoPinsCount = IO_PIN_COUNT;
hw->IoPinDefs = TCanM3IoPinDefs;
hw->KeyPannel = &TCanM3KeyPannel;
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


void TCanM3Destroy(struct TCanHw *hw)
{
guint i;

if (hw)
  {
  for (i = 0; i < hw->IoPinsCount; i++)
    safe_free(hw->IoPin[i].Name);
  g_free(hw);
  }
}


