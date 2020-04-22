#include <app.h>
#include <snake.h>

#define STACK_SIZE 4096

const struct timespec sleep_100ms = {
	.tv_sec		= 0,
	.tv_nsec	= 100000000
};

unsigned char fat_rng(void)
{
	unsigned char r;
	read(3, &r, 1);
	return r;
}

struct fb_ctx {
	off_t		s_off;
	int		fd;
	unsigned short	stride;
	unsigned char	scale;
};

#define MIN(a,b) ((a<b)?(a):(b))
static void fb_ctx_init(struct fb_ctx *fb, const char *fb_fn)
{
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;

	fb->fd = open(fb_fn, O_RDWR);

	ioctl(fb->fd, FBIOGET_FSCREENINFO, &fix);
	fb->stride = fix.line_length;

	ioctl(fb->fd, FBIOGET_VSCREENINFO, &var);
	fb->scale = MIN(var.xres, var.yres) / FIELD_HEIGHT;

	fb->s_off = (var.xres > var.yres)
		? ((var.xres-var.yres) / 2 * 4)
		: ((var.yres-var.xres) / 2 * 4);
}

static void clean_fb(struct fb_ctx *fb)
{
	char blk[64];
	memset(&blk, 16, sizeof(blk));

	while (write(fb->fd, &blk, sizeof(blk)) > 0)
		;
}

static void putpixel(struct fb_ctx *fb, const int x, const int y, const int v)
{
	#define BLK_LINE_SIZE (4*fb->scale) /* assume 4 byte RGBA :)*/
	char *blk_line = alloca(BLK_LINE_SIZE);
	memset(blk_line, v, BLK_LINE_SIZE);

	const int poff = (y*fb->scale*fb->stride) + (x*BLK_LINE_SIZE);
	for (int i = 0; i < fb->scale; ++i) {
		lseek(fb->fd, fb->s_off+poff+(fb->stride*i), SEEK_SET);
		write(fb->fd, blk_line, BLK_LINE_SIZE);
	}
}

static void clean_field(struct fb_ctx *fb)
{
	for (int y = 0; y < 16; ++y)
		for (int x = 0; x < 16; ++x)
			putpixel(fb, x, y, 0);
}

static void fb_pt_cb(int x, int y, int set, void *data)
{
	if (set)
		putpixel(data, x, y, 255);
	else
		putpixel(data, x, y, 0);
}

static int init_state;
enum {
	INIT_NONE,
	INIT_VT,
	INIT_TTY,
	INIT_FB
};

static int exit_req;
enum {
	EXIT_NONE,
	EXIT_GAMEOVER,
	EXIT_INTERRUPT,
	EXIT_QUIT,
	EXIT_WON
};

static void sighandler(int signal)
{
	exit_req = EXIT_INTERRUPT;
}

static void writeu8(unsigned char u)
{
	unsigned char c;
	if (u/10)
		writeu8(u/10);

	c = u % 10 + '0';
	write(0, &c, 1);
}

static struct snake_state snake;
int main(int argc, char **argv)
{
	struct termios oldterm;
	struct vt_mode oldvt;
	struct fb_ctx fctx;

	open("/dev/urandom", O_RDONLY); /* urandom fd == 3 */

	do { /* Apply stack size if necessary */
		struct rlimit64 rlim;

		prlimit64(0, RLIMIT_STACK, NULL, &rlim);
		if (rlim.rlim_cur == STACK_SIZE)
			break;

		rlim.rlim_cur = rlim.rlim_max = STACK_SIZE;
		prlimit64(0, RLIMIT_STACK, &rlim, NULL);

		/* ASLR with low stack sizes causes SIGSEGV */
		personality(ADDR_NO_RANDOMIZE);

		/* need to re-exec to apply changes */
		char *const noarg = NULL;
		execve("/proc/self/exe", &noarg, &noarg);
	} while (0);

	/* Setup VT */ {
		struct sigaction sact = {0};
		sact.sa_handler = sighandler;
		sact.sa_flags |= SA_RESTART;
		sigaction(SIGTERM, &sact, NULL);
		sigaction(SIGINT, &sact, NULL);

		struct vt_mode newvt;
		ioctl(0, VT_GETMODE, &oldvt);
		newvt = oldvt;
		newvt.mode = VT_PROCESS;
		newvt.relsig = SIGINT;
		init_state = INIT_VT;
		ioctl(0, VT_SETMODE, &newvt);
	}

	/* setup input */ {
		fcntl(0, F_SETFL, O_NONBLOCK);

		struct termios newterm;
		ioctl(0, TCGETS, &oldterm);
		newterm = oldterm;
		newterm.c_lflag &= ~(ICANON | ECHO);
		init_state = INIT_TTY;
		ioctl(0, TCSETS, &newterm);
	}

	fb_ctx_init(&fctx, "/dev/fb0");
	init_state = INIT_FB;
	ioctl(0, KDSETMODE, KD_GRAPHICS);
	clean_fb(&fctx);
	clean_field(&fctx);

	snake_init(&snake);
	snake_draw(&snake, fb_pt_cb, &fctx);
	while (!exit_req) {
		char r;
		while (read(0, &r, 1) > 0 && r == 0x1b) {
			if (!read(0, &r, 1) || r != 0x5b) {
				exit_req = EXIT_QUIT;
				goto exit;
			}

			read(0, &r, 1);
			switch (r) {
			case 'A': r = DIR_UP;	break;
			case 'B': r = DIR_DOWN;	break;
			case 'D': r = DIR_LEFT;	break;
			case 'C': r = DIR_RIGHT;break;
			}
			snake_input(&snake, r);
		}

		switch (snake_step(&snake, fb_pt_cb, &fctx)) {
		case SNAKE_GAMEOVER:
			exit_req = EXIT_GAMEOVER;
			break;
		case SNAKE_WON:
			exit_req = EXIT_WON;
			break;
		}
		nanosleep(&sleep_100ms, NULL);
	}

exit:
	switch (init_state) {
	case INIT_FB:
		ioctl(0, KDSETMODE, KD_TEXT);
	case INIT_TTY:
		ioctl(0, TCSETS, &oldterm);
	case INIT_VT:
		ioctl(0, VT_SETMODE, &oldvt);
	}

	#define writes(s) do { write(0, s, sizeof(s)-1); } while (0)
	switch (exit_req) {
	case EXIT_GAMEOVER:
		writes("Game Over\n");
		break;
	case EXIT_INTERRUPT:
		writes("Interrupted by signal\n");
		break;
	case EXIT_QUIT:
		writes("Game Quit\n");
		break;
	case EXIT_WON:
		writes("Congratulations!\n");
		break;
	}

	if (snake.snake_len <= 2) {
		writes("You didn't grow at all!\n");
	} else if (snake.snake_len != 256) {
		writes("You grew ");
		writeu8(snake.snake_len-2);
		if (snake.snake_len-2 == 1)
			writes(" pixel\n");
		else
			writes(" pixels\n");
	} else {
		writes("You're winner!\n");
	}

	return 0;
}
