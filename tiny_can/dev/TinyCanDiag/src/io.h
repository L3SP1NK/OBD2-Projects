#ifndef __IO_H__
#define __IO_H__

#include <glib.h>
#include "can_types.h"
#include "hw_types.h"


#define INDEX_CHANNEL(idx, ch) ((idx) | ((ch) << 16))

int32_t TinyCanOpen(struct TCanHw *hw);
int32_t TinyCanSetIoConfig(struct TCanHw *hw);
void TinyCanClose(struct TCanHw *hw);

int GetPinIndex(struct TCanHw *hw, int pin, int pin_type);
int32_t SetOutPin(struct TCanHw *hw, int pin, int state);
int32_t SetAnalogOutPin(struct TCanHw *hw, int pin, uint16_t value);
int32_t GetAnalogInPin(struct TCanHw *hw, int pin, uint16_t *value);
int32_t GetEncoderInPin(struct TCanHw *hw, int pin, int32_t *value);
int32_t GetDigInPort(struct TCanHw *hw, int port, uint16_t *value);
int32_t GetKeycode(struct TCanHw *hw, uint32_t *value);

#endif
