OBJDIR = build
SUBDIRS := ./boot ./dev ./fs ./kernel ./lib ./usr

LD = ld
LDFLAGS = -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

mixv6.img: $(SUBDIRS)
	$(LD) $(LDFLAGS) -T kernel.ls -o $(OBJDIR)/kernel $(shell find $(OBJDIR)/*.o)
	./mkfs

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	make -C $@ $(MAKECMDGOALS)

clean:
	find -type f -name *.[od] | xargs $(RM)
	$(RM) mixv6.img $(OBJDIR)/*

form: 
	find -type f -name "*.[ch]" | xargs clang-format -i

