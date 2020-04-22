#ifndef SNAKE_H
#define SNAKE_H

#define NULL ((void *)0)

/* A 16x16 field size is assumed throughout the code */
#define FIELD_WIDTH	16
#define FIELD_HEIGHT	16
#define FIELD_SIZE	(FIELD_WIDTH * FIELD_HEIGHT)

/*
 * This is used both for direction of the snake's head and tail and for
 * input buffering (which is merged with the code for the former).
 */
enum snake_direction {
	DIR_RIGHT,
	DIR_LEFT,
	DIR_DOWN,
	DIR_UP
};
#define DIR_VERTICAL(d) (d&2) /* 2nd bit set == vertical dir */

/*
 * Declarations relating to fat.c
 */
typedef struct fat_mem {
	unsigned char fat_map[FIELD_SIZE];
	unsigned char fat_arr[FIELD_SIZE];
	short fat_p;
} fat_mem;
void fat_reset(struct fat_mem *);
int fat_isfree(struct fat_mem *, const int);
void fat_set_used(struct fat_mem *, const int);
void fat_set_free(struct fat_mem *, const int);
int fat_rand(struct fat_mem *); /* get random free cell in FAT */
unsigned char fat_rng(void); /* This is defined elsewhere but used in fat.c */

/*
 * Declarations relating to liquid.c
 */
#define INPUT_BUFSIZE	2	/* Allows u-turning and doesn't feel laggy */
#define LIQ_ELEM_BITS	2
#define LIQ_ELEMS_UCHAR	4	/* 2 bits per list/queue element */
#define LIQ_ELEMS	(FIELD_SIZE-1 + INPUT_BUFSIZE)
#define LIQ_BYTES	((LIQ_ELEMS/LIQ_ELEMS_UCHAR)	\
			+!!(LIQ_ELEMS%LIQ_ELEMS_UCHAR))

typedef struct liquid_mem {
	unsigned short liq_head;
	unsigned short liq_tail;
	unsigned char liq_me[LIQ_BYTES];
} liquid_mem;
void liq_push(struct liquid_mem *, const unsigned v);
unsigned liq_pop(struct liquid_mem *);
unsigned liq_peek(struct liquid_mem *, const unsigned off);
void liq_poke(struct liquid_mem *, const unsigned off, const unsigned v);
void liq_reset(struct liquid_mem *m);

/* Declarations relating to snake.c */
typedef struct snake_mem {
	unsigned char snake_head_pos;
	unsigned char snake_tail_pos;
	unsigned char food_pos;
	unsigned char food_delay; /* delay tail after eating food, n frames */
	unsigned char buffi;	/* Buffered input */
	unsigned short snake_len;
} snake_mem;

struct snake_state {	/* By your powers combined */
	fat_mem;
	liquid_mem;
	snake_mem;
};

enum snake_step_ret {
	SNAKE_CONTINUE,
	SNAKE_GAMEOVER,
	SNAKE_WON
};
typedef void (*snake_pt_cb)(int x, int y, int set, void *data);
int snake_input(struct snake_state *, enum snake_direction);
int snake_step(struct snake_state *, snake_pt_cb, void *);
void snake_draw(struct snake_state *, snake_pt_cb, void *);
void snake_init(struct snake_state *);

#endif
