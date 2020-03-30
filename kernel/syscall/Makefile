OBJS = 	syscall.o\
		arg.o\
		exit.o\
		fork.o\
		read.o\
		write.o\
		open.o\
		close.o\
		wait.o\
		creat.o\
		link.o\
		unlink.o\
		execv.o\
		kill.o\
		reboot.o\

CC = gcc
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
INCLUDE = ../../include
OBJDIR = ..

# gcc compile options
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -fno-asynchronous-unwind-tables -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif
CFLAGS += -I $(INCLUDE) -I /usr/include
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
# FreeBSD ld wants ``elf_i386_fbsd''
LDFLAGS = -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

all: $(OBJS)
	$(LD) $(LDFLAGS) -o $(OBJDIR)/syscall.o -r $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


