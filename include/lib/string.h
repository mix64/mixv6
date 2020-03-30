#pragma once

#include <types.h>

void memset(void *addr, int data, int count);
void memmove(void *src, void *dst, uint32_t count);
int strcmp(const char *p, const char *q, uint32_t limit);
int strlen(const char *s);