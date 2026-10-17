# mctx.h

> A tiny memory buddy for C: allocate now, clean up later.

`mctx.h` is a small allocation context for programs that create a bunch of
temporary values together. Instead of keeping track of every pointer and
calling `free` on each one, put the allocations in an `mctx_context_t` and
release the whole group when you are done.

It is intentionally simple:

- one dependency: the C standard library;
- one linked list of tracked allocation blocks;
- one cleanup call for the whole group;
- no individual `free` operation to forget.

## The idea

An `mctx_context_t` is a little bucket for allocations:

```text
create context
    |
    +-- mctx__malloc(...)  -> tracked block
    +-- mctx__malloc(...)  -> tracked block
    +-- mctx__realloc(...) -> tracked block
    |
cleanup context ----------> all blocks are released
```

Every pointer returned by `mctx__malloc` or `mctx__realloc` belongs to its
context. The context must stay alive until those pointers are no longer used.

## API at a glance

| Function | What it does |
| --- | --- |
| `mctx_context__init(&ctx)` | Initializes a context supplied by the caller. |
| `mctx_context__new()` | Allocates and initializes a context on the heap. |
| `mctx__malloc(ctx, size)` | Allocates `size` bytes and tracks the block. |
| `mctx__realloc(ctx, ptr, size)` | Resizes a tracked block. Passing `NULL` behaves like `mctx__malloc`; an unknown pointer returns `NULL`. |
| `mctx_context__deinit(&ctx)` | Releases tracked blocks, but not the context object itself. |
| `mctx_context__free(ctx)` | Releases tracked blocks and a heap-allocated context. |

## Example: a stack-owned context

This is the simplest shape for a short-lived scope:

```c
#include "mctx.h"

#include <stdio.h>
#include <string.h>

int
main(void)
{
	mctx_context_t ctx;
	mctx_context__init(&ctx);

	char *message = mctx__malloc(&ctx, 32);
	int *numbers = mctx__malloc(&ctx, 3 * sizeof *numbers);

	if (message == NULL || numbers == NULL) {
		mctx_context__deinit(&ctx);
		return 1;
	}

	strcpy(message, "hello from mctx.h");
	numbers[0] = 7;
	numbers[1] = 21;
	numbers[2] = 42;

	char *larger_message = mctx__realloc(&ctx, message, 64);
	if (larger_message == NULL) {
		mctx_context__deinit(&ctx);
		return 1;
	}
	message = larger_message;

	printf("%s (%d, %d, %d)\n",
	       message, numbers[0], numbers[1], numbers[2]);

	/*
	 * This releases message and numbers together.
	 * Do not call free(message) or free(numbers) yourself.
	 */
	mctx_context__deinit(&ctx);
	return 0;
}
```

The `mctx_context__deinit` call is the cleanup button. It releases every
allocation made through that context, including blocks that were moved by
`mctx__realloc`.

## Example: a heap-owned context

When the context itself needs to outlive the current stack frame, use
`mctx_context__new` and pair it with `mctx_context__free`:

```c
void
use_heap_context(void)
{
	mctx_context_t *ctx = mctx_context__new();
	if (ctx == NULL) {
		return;
	}

	char *greeting = mctx__malloc(ctx, 32);
	if (greeting == NULL) {
		mctx_context__free(ctx);
		return;
	}

	strcpy(greeting, "a tiny memory hug");
	puts(greeting);

	mctx_context__free(ctx);
}
```

The important bit is that the context is the owner of the allocation group,
and `mctx_context__free` releases both the greeting and the context itself.

## Building the example project

The repository includes a tiny sample program and a `Makefile`:

```sh
make
make run
make clean
```

To use the header in your own small program:

```sh
cc -std=c99 -Wall -Wextra -Wpedantic -Iinclude your_program.c \
	-o your_program
```

`mctx.h` contains the implementation as well as the declarations. Include it
in one translation unit, then link that translation unit into your program.

## Debugging in VS Code

The repository includes a ready-to-go `.vscode/launch.json` and matching
`.vscode/tasks.json`. Install a C/C++ extension with GDB support, open the
project folder, and press `F5`. VS Code will:

1. rebuild the project with debug symbols;
2. launch `build/c-malloc-context` under GDB; and
3. let you set breakpoints in `src/main.c` or `include/mctx.h`.

## A few friendly rules

1. **Use one context consistently.** Only pass a pointer back to
   `mctx__realloc` if it was allocated by the same context.
2. **Let the context do the freeing.** Do not call `free` on a tracked pointer;
   call `mctx_context__deinit` or `mctx_context__free` once the group is done.
3. **Check allocation results.** `mctx__malloc` and `mctx__realloc` return
   `NULL` when allocation fails.
4. **Keep ownership obvious.** A context is a convenient lifetime boundary,
   not a general-purpose garbage collector or a thread-safe allocator.

Small API, tidy cleanup, fewer pointer-shaped chores. That is the whole magic.