#include "mctx.h"

#include <stdio.h>

int
main(void)
{
	mctx_context_t *ctx = mctx_context__new();
	if (ctx == NULL) {
		fputs("c-malloc-context: mctx_context__new failed.\n", stderr);
		return 1;
	}

	for(size_t i = 0; i < 32; i++){
		char *message = mctx__malloc(ctx, 32);
		
		if (message == NULL) {
			fputs("c-malloc-context: mctx__malloc failed.\n", stderr);
			return 1;
		}

		char *resized = mctx__realloc(ctx, message, 64);

		if (resized == NULL) {
			fputs("c-malloc-context: mctx__realloc failed.\n", stderr);
			return 1;
		}
	}

	mctx_context__free(ctx);
	return 0;
}
