/*
 * fat.c, "Free(grid cell) Array Thing"
 *
 * Keeps track of unused/used blocks by shuffling their indexes in an
 * array; items in m->fat_arr[] <= fat_p are considered unused.
 */
#include "snake.h"

/* Initializes a fat_mem struct to the default state */
void fat_reset(struct fat_mem *m)
{
	for (int i = 0; i < FIELD_SIZE; ++i)
		m->fat_map[i] = m->fat_arr[i] = i;
	m->fat_p = FIELD_SIZE-1;
}

/* If a cell index is contained in a fat_arr[] index <= fat_p, it's unused */
int fat_isfree(struct fat_mem *m, const int i)
{
	return m->fat_map[i] <= m->fat_p;
}

/* Helper static function that swaps a cell index with the last used/free */
#define SWAP(a,b) { typeof(a) tmp = a; a=b; b=tmp; } while (0)
static void fat_swap(struct fat_mem *m, const int i, const int p)
{
	const int j = m->fat_arr[p];
	const int q = m->fat_map[i];

	SWAP(m->fat_map[i], m->fat_map[j]);
	SWAP(m->fat_arr[p], m->fat_arr[q]);
}

void fat_set_used(struct fat_mem *m, const int i)
{
	if (fat_isfree(m, i))
		fat_swap(m, i, (m->fat_p)--);
}

void fat_set_free(struct fat_mem *m, const int i)
{
	if (!fat_isfree(m, i))
		fat_swap(m, i, ++(m->fat_p));
}

/* Used to mask off unnecesary top bits off the random number used below */
static unsigned __attribute__ ((const)) getuintmask(const unsigned u)
{
	return (unsigned)-1 >> __builtin_clz(u);
}

/*
 * Returns the index of a random free unused block or -1 if there's none
 * tries to be unbiased by using masks and rejection instead of "%"
 */
int fat_rand(struct fat_mem *m)
{
	if (m->fat_p > 0) {
		unsigned r;
		do {
			r = fat_rng() & getuintmask(m->fat_p);
		} while (r > m->fat_p);
		return m->fat_arr[r];
	} else if (m->fat_p == 0) {
		return m->fat_arr[0];
	}

	return -1;
}