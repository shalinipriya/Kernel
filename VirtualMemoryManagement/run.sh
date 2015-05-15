 make -f makefile.linux
export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libXpm.so.4 
sudo mount -o loop dev_kernel_grub.img /mnt/floppy/
sudo cp kernel.bin /mnt/floppy
sudo umount /mnt/floppy/
bochs -f bochsrc.bxrc 

