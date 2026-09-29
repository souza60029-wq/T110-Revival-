################################################################################
HOW TO BUILD KERNEL FOR SM-T110_NA_XAR

1. How to Build
	- get Toolchain
	download and install arm-eabi-4.6 toolchain for ARM EABI.
	Extract kernel source and move into the top directory.

	$ export ARCH=arm
	$ make pxa986_goyawifi_rev02_defconfig
	$ make
	
2. Output files
	- Kernel : Kernel/arch/arm/boot/zImage
	- module : Kernel/drivers/*/*.ko
	
3. How to Clean	
    $ make clean
	
4. How to make .tar binary for downloading into target.
	- change current directory to Kernel/arch/arm/boot
	- type following command
	$ tar cvf SM-T110_NA_XAR_Kernel.tar zImage
#################################################################################