#include <linux.h>

int rt_sigaction(int signum, const struct sigaction *act,
	struct sigaction *oldact, size_t sigsetsize);

int sigaction(int signum, const struct sigaction *act, struct sigaction *oact)
{
	return rt_sigaction(signum, act, oact, sizeof(sigset_t));
}