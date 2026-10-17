#include <stdlib.h>

typedef struct _mctx_context_entry_t{
	struct _mctx_context_entry_t *next;
	struct _mctx_context_entry_t *prev;
	void *ptr;
}mctx_context_entry_t;

typedef struct _mctx_context_t{
	struct _mctx_context_entry_t *first;
}mctx_context_t;

void
mctx_context__init(mctx_context_t *ctx)
{
	ctx->first = NULL;
}

mctx_context_t *
mctx_context__new(void)
{
	mctx_context_t *ctx = malloc(sizeof *ctx);

	if (NULL != ctx) {
		mctx_context__init(ctx);
	}

	return ctx;
}

void
mctx_context__deinit(mctx_context_t *ctx)
{
	mctx_context_entry_t *entry = ctx->first;
	while(NULL != entry){
		mctx_context_entry_t *next = entry->next;
		free(entry);
		entry = next;
	}
	ctx->first = NULL;
}

void
mctx_context__free(mctx_context_t *ctx)
{
	mctx_context__deinit(ctx);
	free(ctx);
}

void *
mctx__malloc(mctx_context_t *ctx, size_t size)
{
	if (size > (size_t)-1 - sizeof(mctx_context_entry_t)) {
		return NULL;
	}

	mctx_context_entry_t *ret = malloc(size + sizeof(mctx_context_entry_t));
	if (ret != NULL) {
		ret->next = ctx->first;
		ret->prev = NULL;
		if (ctx->first != NULL) {
			ctx->first->prev = ret;
		}
		ret->ptr = (void *)(ret + 1);
		ctx->first = ret;
		ret = ret->ptr;
	}
	return ret;
}

void *
mctx__realloc(mctx_context_t *ctx, void *ptr, size_t size)
{
	if (ptr == NULL) {
		return mctx__malloc(ctx,size);
	}

	mctx_context_entry_t *entry = ((mctx_context_entry_t *)ptr) - 1;
	mctx_context_entry_t *prev = entry->prev;
	mctx_context_entry_t *next = entry->next;
	mctx_context_entry_t *resized = realloc(
		entry
		,size + sizeof(mctx_context_entry_t)
	);

	if (resized == NULL) {
		return NULL;
	}

	resized->prev = prev;
	resized->next = next;
	resized->ptr = (void *)(resized + 1);

	if (prev != NULL) {
		prev->next = resized;
	} else {
		ctx->first = resized;
	}

	if (next != NULL) {
		next->prev = resized;
	}

	return resized->ptr;
}
