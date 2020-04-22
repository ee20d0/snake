/*
 * liquid.c: LIst/QUeue and Input/Direction
 *
 * Merges the input queue with the list of used cells as they're very close
 * in implementation.
 */
#include "snake.h"

#define MASK_2BITS 0x3

/* Helper functions to rw/address two bits from memory */
static unsigned read_2b(const unsigned char *const m, const unsigned i)
{
	const unsigned m_ind = i / LIQ_ELEMS_UCHAR;
	const unsigned shift = i % LIQ_ELEMS_UCHAR * LIQ_ELEM_BITS;

	return (m[m_ind] >> shift) & MASK_2BITS;
}

static void write_2b(unsigned char *const m, const unsigned i, const unsigned v)
{
	const unsigned m_ind = i / LIQ_ELEMS_UCHAR;
	const unsigned shift = i % LIQ_ELEMS_UCHAR * LIQ_ELEM_BITS;
	const unsigned bmask = ~(MASK_2BITS << shift);
	const unsigned shval = (v & MASK_2BITS) << shift;

	m[m_ind] = (m[m_ind] & bmask) | shval;
}

void liq_push(struct liquid_mem *m, const unsigned v)
{
	write_2b(m->liq_me, m->liq_head++, v);
	m->liq_head %= LIQ_ELEMS;
}

unsigned liq_pop(struct liquid_mem *m)
{
	const unsigned ret = read_2b(m->liq_me, m->liq_tail++);
	m->liq_tail %= LIQ_ELEMS;
	return ret;
}

/* Read a value from the liq without affecting it, has implicit -1 offset */
unsigned liq_peek(struct liquid_mem *m, const unsigned offset)
{
	const int eff_off = -1 + (int)offset; /* effective offset */
	const unsigned i = ((int)m->liq_head + eff_off < 0)
				? LIQ_ELEMS-1
				: ((int)m->liq_head + eff_off) % LIQ_ELEMS;

	return read_2b(m->liq_me, i);
}

void liq_poke(struct liquid_mem *m, const unsigned offset, const unsigned v)
{
	write_2b(m->liq_me, (m->liq_head+offset)%LIQ_ELEMS, v);
}

/* Tail points to next value to be read, head points to next value to write */
void liq_reset(struct liquid_mem *m)
{
	m->liq_me[0] = m->liq_me[LIQ_ELEMS-1] = m->liq_tail = m->liq_head = 0;
}
