# snake

By default builds an SDL2 and a standalone executable that uses `/dev/fb0`

To build a smaller version use `make LHDR=<path to x86 linux kernel headers> main_fb86_s`

The kernel headers can be obtained from the kernel sources with `make ARCH=x86 INSTALL_HDR_PATH=<path> headers_install`
