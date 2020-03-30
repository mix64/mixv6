# mixv6
Simple Operating System for x86

## How to use
1. Download Bochs from http://bochs.sourceforge.net  
(I debugging this project in Bochs 2.6.9)

2. $ mkdir build && make 

3. register .img file on Bochs Configure.  

4. Boot from Bochs.

### TODO
・Support TCP/IP.  
・Implementation heap memory for kernel. (like Slab allocation or kalloc())  
・Increase system call, user library.  
・Support Dynamic Link Library. (like .dll or .so)  
・Support ARM architecture. (especially Raspberry Pi Zero)
