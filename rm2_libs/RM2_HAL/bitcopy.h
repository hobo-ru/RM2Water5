#ifndef __RM2SETTINGS_H
#define __RM2SETTINGS_H

#include "em_device.h"
#include "em_chip.h"

void copy_lowbits_off(uint8_t* dst, uint8_t* src,  uint16_t n_bits, uint8_t dst_offbits, uint8_t src_offbits);

#endif