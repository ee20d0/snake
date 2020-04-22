CFLAGS = -fplan9-extensions -flto -march=native -Os -fomit-frame-pointer -pipe -ffunction-sections -fdata-sections
SNAKE_COMMON_OBJ = snake.o liquid.o fat.o

vpath %.c 1/x86 1/ 2/
vpath %.h 1/x86/include/ 1/ 2/ $(LHDR)
vpath %.S 1/x86

all: main_sdl2 main_fb

$(SNAKE_COMMON_OBJ): snake.h

main_sdl2: CFLAGS += -Og -ggdb
main_sdl2: LDFLAGS += -lSDL2
main_fb86: CFLAGS += -I1 -I1/x86/include -I$(LHDR) -I2/ -ffreestanding -nostdinc -nostdlib -nostartfiles -m32 -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -static -march=i386
main_fb86: ASFLAGS = $(CFLAGS)
main_fb86: LDFLAGS = $(CFLAGS) -Wl,-T1/x86/link.lds,-e_start,--build-id=none,-melf_i386,-gc-sections

main_sdl2: main_sdl2.o $(SNAKE_COMMON_OBJ)
main_fb: main_fb.o $(SNAKE_COMMON_OBJ)
main_fb86: head.o syscall.o signal.o $(patsubst %.o,%.c,$(SNAKE_COMMON_OBJ))
main_fb86_s: main_fb86
	objcopy -O binary -j .text main_fb86 main_fb86_s

clean:
	rm -rf *.o main_fb main_sdl2 main_fb86 main_fb86_s

.PHONY: all clean
