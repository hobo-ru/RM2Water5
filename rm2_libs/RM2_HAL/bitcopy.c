#include "bitcopy.h"

#define min(x,y)	((x<y)?(x):(y))
#define BITMASKU8(x) ((1U << (x)) - 1)
/**
* Select n_bits bits from src starting from the lower bit and
* copy the selected bits to dst at offset msb_off, starting from the most
* significant bit.
* The caller must ensure that n_bits + msb_off <= 8
*
* @param dst: Destination byte
* @param src: Source byte
* @param n_bits: Number of low bits to select from src
* @param msb_off: Offset from msb in dst
*/
void offset_bitcpy(uint8_t* dst, uint8_t src, uint8_t n_bits, uint8_t msb_off)
{
	uint8_t sel_src = src & BITMASKU8(n_bits);
	uint8_t shift = 8 - (msb_off + n_bits);
	uint8_t shift_src = sel_src << shift;
	*dst |= shift_src;
}

/**
* Copy an arbitrary number of *bits* from an arbitrary position (expressed
* in bits) in the src buffer, to an abitrary position (expressed in *bits*) in
* the dst buffer.
* The caller must ensure that dst_offbits < 8 && src_offbits < 8. If you need
* to have a bit offset > 8, set dst += bit offset / 8 and
* dst_offbits = bit offset % 8 (respectively for src). n_bits may be >= 8.
*
* @param dst: Destination buffer
* @param src: Source buffer
* @param n_bits: Number of bits to copy from src to dst
* @params src_offbits: [0,7] Offset in the first byte of the source buffer
* @params dst_offbits: [0,7] Offset in the second byte of the destination
*      buffer
*/
void copy_lowbits_off(uint8_t* dst, uint8_t* src,  uint16_t n_bits, uint8_t dst_offbits, uint8_t src_offbits)
{
	if(dst_offbits > 7)
	{
		dst += dst_offbits / 8;
		dst_offbits = dst_offbits % 8;
	}

	while(n_bits != 0)
	{
		// Number of bits to select from the first byte of src, and write
		// to the first byte of dst
		uint8_t sel_byte  = min(n_bits, 8U - dst_offbits);
		uint8_t off_src = *src >> src_offbits;
		offset_bitcpy(dst, off_src, sel_byte, dst_offbits);
		// Update dst offsets
		uint8_t add_dst_off = sel_byte + dst_offbits;
		dst += add_dst_off / 8;
		dst_offbits = add_dst_off % 8;
		// Update src offsets
		src_offbits += sel_byte;
		src += src_offbits / 8;
		src_offbits = src_offbits % 8;
		n_bits -= sel_byte;
	}
}