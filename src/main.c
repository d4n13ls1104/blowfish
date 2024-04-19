#include "gc.h"

int main(void)
{
	GameContext *ctx = gc_create();
	if (ctx == NULL) {
		fprintf(stderr, "gc_create\n");
		exit(EXIT_FAILURE);
	}

	while (!ctx->quit) {
		gc_poll_events(ctx);
		gc_render(ctx);
	}

	gc_destroy(ctx);
	return 0;
}
