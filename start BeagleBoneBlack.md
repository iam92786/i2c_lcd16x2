# In Package:
	1. Quick start Mannual
	2. Cable - USB mini
	3. BeagleBone Balck Board


# User Guide: 
	1. Carry your own Bootbale SD card.
	2. Boot BBB with Bootable SD card only.
	3. Changes in prebuilt image(emmc card image) of BBB is restricted. 
	(Hence always carry your own Bootable SD card) 


# Access BBB in our Local Machine
	1. Connect the BBB with local machine using USB mini cable.

	2. Wait for 2 minute after power on.
	
	3. ping 192.168.7.2
		* C:\Users\asiddiqui>ping 192.168.7.1
	
	4. ifconfig
	
	5. For ssh of BBB
		```ssh debian@192.168.7.2```
		> Are you sure you want to continue connecting (yes/no/[fingerprint])? "yes"
		> debian@192.168.7.2's password:temppwd
		> debian@beaglebone:~$
		(ssh is Successful, YOu can Start using BBB)


# Quick Observation / Specification:
* debian@beaglebone:~$ pwd
	/home/debian

* debian@beaglebone:~$ uname -r
	4.19.94-ti-r42

* debian@beaglebone:~$ uname -a
	Linux beaglebone 4.19.94-ti-r42 #1buster SMP PREEMPT Tue Mar 31 19:38:29 UTC 2020 armv7l GNU/Linux


# Where to get source code:
	* The U-Boot source code is maintained in the Git repository at [link](https://source.denx.de/u-boot/u-boot.git)

	* you can browse it online at [link](https://source.denx.de/u-boot/u-boot)


# BeagleBoard.org Latest Firmware Images
	* https://beagleboard.org/latest-images

# BeagleBoard.org Latest Firmware Images
	* https://github.com/beagleboard

# Helpful Links
	* https://elinux.org/Beagleboard:Updating_The_Software