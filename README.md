mapping kernel mem (allocated with kmalloc) to user space (accesed via mmap)

build for x86:
-----------------
gcc user-simpel.c (user space)
make (kernel module, copy the Makefile bevor)

build for ppc:
-----------------
powerpc-linux-gnu-gcc-4.8 user-simpel.c (user space)
make (kernel module, copy the Makefile bevor)

./a.out /dev/simpler


build for user space for arm64:
-------------------------------
. /opt/fsl-qoriq/2.0/environment-setup-aarch64-fsl-linux
$CC user-simpel.c

build for kernel space for arm64:
-------------------------------
. /opt/fsl-qoriq/2.0/environment-setup-aarch64-fsl-linux
unset LDD
make
