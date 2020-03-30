#include <x86/regs.h>
#include <x86/segment.h>

#Start the first CPU : switch to 32 - bit protected mode, jump into C.
#The BIOS loads this code from the first sector of the hard disk into
#memory at physical address 0x7c00 and starts executing in real mode
#with % cs = 0 % ip = 7c00.

.code16                       # Assemble for 16-bit mode
.globl start
start:
  cli
#Zero data segment registers DS, ES, and SS.
  xorw    %ax, %ax             # Set %ax to zero
  movw    %ax, %ds             # -> Data Segment
  movw    %ax, %es             # -> Extra Segment
  movw    %ax, %ss             # -> Stack Segment

#Physical address line A20 is tied to zero so that the first PCs
#with 2 MB would run software that assumed 1 MB.Undo that.
seta20.1:
  inb     $0x64, %al               # Wait for not busy
  testb   $0x2, %al
  jnz     seta20.1

  movb    $0xd1, %al               # 0xd1 -> port 0x64
  outb    %al, $0x64

seta20.2:
  inb     $0x64, %al               # Wait for not busy
  testb   $0x2, %al
  jnz     seta20.2

  movb    $0xdf, %al               # 0xdf -> port 0x60
  outb    %al, $0x60

#Switch from real to protected mode.Use a bootstrap GDT that makes
#virtual addresses map directly to physical addresses so that the
#effective memory map doesn't change during the transition.
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $(CR0_PE), %eax
  movl    %eax, %cr0

// PAGEBREAK!
#Complete the transition to 32 - bit protected mode by using a long jmp
#to reload % cs and % eip.The segment descriptors are set up with no
#translation, so that the mapping is still the identity mapping.
  ljmp    $(SEG_KCODE), $start32

.code32  # Tell assembler to generate 32-bit code now.
start32:
#Set up the protected - mode data segment registers
  movw    $(SEG_KDATA), %ax       # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  xorw    %ax, %ax                # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS
  movl    $start, %esp            # Set up the stack pointer

  call    bootmain

#If returns(it shouldn't), trigger a Bochs
#breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
  outw    %ax, %dx

spin:
  jmp spin


.globl memclr
memclr: # (char *addr, uint32_t count)
  push    %ebp
  movl    %esp, %ebp
  push    %edi

  movl    8(%ebp), %edi           # base address
  movl    12(%ebp), %ecx          # write count
  movb    $0x00, %al              # write data (0)
  rep stosb

  pop     %edi
  leave
  ret


.globl readseg
readseg: # (char *addr, uint8_t count, uint32_t offset)

  push    %ebp
  movl    %esp, %ebp
  push    %edi

  call    waitdisk

  movb    12(%ebp), %al           # read sector num.
  movw    $0x1F2, %dx             # sector count, how many sectors to read/write
  outb    %al, %dx                

  movw    $0x1F3, %dx             # Port to send bit 0 - 7 of LBA
  movb    16(%ebp), %al           # Get bit 1 - 7 in AL
  outb    %al, %dx

  movw    $0x1F4, %dx             # Port to send bit 8 - 15 of LBA
  movb    17(%ebp), %al           # Get bit 8 - 15 in AL
  outb    %al, %dx

  movw    $0x1F5, %dx             # Port to send bit 16 - 23 of LBA
  movb    18(%ebp), %al           # Get bit 16 - 23 in AL
  outb    %al, %dx

  movw    $0x1F6, %dx             # Port to send drive and bit 24 - 27 of LBA
  movb    19(%ebp), %al           # Get bit 24 - 27 in AL
  orb     $0xE0, %al              # Set bit 6 in al for LBA(Logical Block Address) mode
  outb    %al, %dx

  movw    $0x1F7, %dx             # Command port
  movb    $0x20, %al              # 0x20: Read with retry.
  outb    %al, %dx

  call    waitdisk                # wait disk read finish

  movb    $0x80, %dl
  movl    8(%ebp), %edi           # kernel base address
  xorl    %eax, %eax
  movb    12(%ebp), %al           # read sector num. only bit 0 - 7 valid
  mulb    %dl                     # sector num(%al) * 0x80(=512/4)(%dl) -> %ax
  movl    %eax, %ecx
  movw    $0x1F0, %dx             # data register, the bytes are written/read here
  rep insl

  pop     %edi
  leave
  ret


waitdisk:
  movw    $0x1F7, %dx
waitdisk.1:
  inb     %dx, %al                # Status register
  andb    $0xC0, %al              # bit7: controller is executing a command
  cmpb    $0x40, %al              # bit6: drive is ready
  jnz     waitdisk.1              # waiting disk
  ret

#Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt:
  SEG_ASM(0, 0, 0, 0)                           # null seg
  SEG_ASM(STA_X|STA_R, 0, 0xffffffff, 0)        # kernel code seg
  SEG_ASM(STA_W, 0, 0xffffffff, 0)              # kernel data seg
  SEG_ASM(STA_X|STA_R, 0, 0xffffffff, DPL_USER) # user code seg
  SEG_ASM(STA_W, 0, 0xffffffff, DPL_USER)       # user data seg

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt                             # address gdt
