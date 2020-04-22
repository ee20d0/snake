/*
 * snake.c: Makes everything work together, core game logic.
 */
#include "snake.h"

struct pt {
	int x;
	int y;
};

static struct pt itoxy(const unsigned i)
{
	return (struct pt) { (i % FIELD_WIDTH), (i / FIELD_WIDTH) };
}

static int xytoi(int x, int y)
{
	return y * FIELD_WIDTH + x;
}

static void call_cb_xy(snake_pt_cb cb, void *data, const int i, const int set)
{
	const struct pt pt = itoxy(i);
	cb(pt.x, pt.y, set, data);
}

void snake_draw(struct snake_state *st, snake_pt_cb cb, void *data)
{
	for (int i = st->fat_p+1; i < FIELD_SIZE; ++i)
		call_cb_xy(cb, data, st->fat_arr[i], 1);

	call_cb_xy(cb, data, st->food_pos, 1);
	call_cb_xy(cb, data, st->snake_tail_pos, 1);
	call_cb_xy(cb, data, st->snake_head_pos, 1);
}

static unsigned different_axis(enum snake_direction a, enum snake_direction b)
{
	return DIR_VERTICAL(a) ^ DIR_VERTICAL(b);
}

int snake_input(struct snake_state *st, enum snake_direction dir)
{
	if (st->buffi < INPUT_BUFSIZE) {
		if (different_axis(liq_peek(&st->liquid_mem, st->buffi), dir))
			liq_poke(&st->liquid_mem, st->buffi++, dir);
		else
			return -1;
	}
	return 0;
}

static void place_food(struct snake_state *st, snake_pt_cb cb, void *data)
{
	st->food_pos = fat_rand(&st->fat_mem);
	if (cb)
		call_cb_xy(cb, data, st->food_pos, 1);
}

static unsigned calc_new_pos(unsigned cpos, enum snake_direction dir)
{
	struct pt pt = itoxy(cpos);

	switch (dir) {
	case DIR_UP:
		pt.y -= 1;
		break;
	case DIR_DOWN:
		pt.y += 1;
		break;
	case DIR_LEFT:
		pt.x -= 1;
		break;
	case DIR_RIGHT:
		pt.x += 1;
		break;
	}

	if (pt.x < 0)
		pt.x = FIELD_WIDTH-1;
	else
		pt.x %= FIELD_WIDTH;

	if (pt.y < 0)
		pt.y = FIELD_HEIGHT-1;
	else
		pt.y %= FIELD_HEIGHT;

	return xytoi(pt.x, pt.y);
}

static void update_tail(struct snake_state *st, snake_pt_cb cb, void *data)
{
	fat_set_free(&st->fat_mem, st->snake_tail_pos);

	if (cb)
		call_cb_xy(cb, data, st->snake_tail_pos, 0);

	st->snake_tail_pos = calc_new_pos(st->snake_tail_pos,
				liq_pop(&st->liquid_mem));
}

enum update_head_ret {
	HEAD_OK,
	HEAD_FOOD,
	HEAD_COLLISION
};

static int update_head(struct snake_state *st, snake_pt_cb cb, void *data)
{
	const unsigned next_dir = liq_peek(&st->liquid_mem, st->buffi ? 1 : 0);

	if (st->buffi)
		st->buffi--;

	liq_push(&st->liquid_mem, next_dir);
	st->snake_head_pos = calc_new_pos(st->snake_head_pos, next_dir);

	if (!fat_isfree(&st->fat_mem, st->snake_head_pos))
		return HEAD_COLLISION;

	fat_set_used(&st->fat_mem, st->snake_head_pos);
	if (cb)
		call_cb_xy(cb, data, st->snake_head_pos, 1);

	if (st->snake_head_pos == st->food_pos)
		return HEAD_FOOD;

	return HEAD_OK;
}

int snake_step(struct snake_state *st, snake_pt_cb cb, void *data)
{
	if (st->food_delay)
		st->food_delay--;
	else
		update_tail(st, cb, data);

	switch (update_head(st, cb, data)) {
	case HEAD_FOOD:
		st->snake_len++;
		st->food_delay++;
		place_food(st, cb, data);
		break;
	case HEAD_COLLISION:
		return SNAKE_GAMEOVER;
	}

	if (st->snake_len == 256)
		return SNAKE_WON;

	return SNAKE_CONTINUE;
}

void snake_init(struct snake_state *st)
{
	st->snake_tail_pos = xytoi(7, 8);
	st->snake_head_pos = xytoi(8, 8);

	fat_reset(&st->fat_mem);
	fat_set_used(&st->fat_mem, st->snake_tail_pos);
	fat_set_used(&st->fat_mem, st->snake_head_pos);

	liq_reset(&st->liquid_mem);
	liq_push(&st->liquid_mem, DIR_RIGHT);
	st->buffi = 0;

	place_food(st, NULL, NULL);
	st->food_delay = 0;

	st->snake_len = 2;
}
