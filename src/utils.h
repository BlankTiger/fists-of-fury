#pragma once

#include <stdio.h>
#include <stdlib.h>

#define unreachable(msg) do { \
    fprintf(stderr, "%s:%d: unreachable condition reached in function %s: %s\n", __FILE__, __LINE__, __func__, msg); \
    abort(); \
} while(0)
