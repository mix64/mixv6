#pragma once

struct inode;
struct proc;

void *kalloc();         /* allocate physical memory by 4KB */
void kfree(char *addr); /* free physical memory */

uint32_t *setupkvm();
void inituvm(uint32_t *pgdir, void *start, uint32_t size);
int loaduvm(uint32_t *pgdir, char *addr, struct inode *ip, uint32_t offset, uint32_t size);
uint32_t *copyuvm(uint32_t *pgdir, uint32_t size);
int allocuvm(uint32_t *pgdir, uint32_t oldsz, uint32_t newsz);
int deallocuvm(uint32_t *pgdir, uint32_t oldsz, uint32_t newsz);
void copyout(uint32_t *pgdir, uint32_t va, char *src, uint32_t len);
int ustack(uint32_t *pgdir, uint32_t start);
void freevm(uint32_t *pgdir);
void switchuvm(struct proc *p);
