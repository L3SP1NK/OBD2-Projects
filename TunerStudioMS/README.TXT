
To run TunerStudio:
- Extract the TunerStudio tar.gz file to your place of choice on the hard drive
- Grant your user write permissions to serial ports and lock files. 
   (In Debian this means adding your user to uucp group, 
    some other flavors of linux it is the lock group, some both)
- Run TunerStudio.sh to start TunerStudio.

TunerStudio.sh



Mac Users:
You can use this tar.gz or the OS X installer.

10.6 and older:
You must first run the sh script by opening a terminal, cd to the dir you have TunerStudio in
run:
./OSX_fixperm-217-leo.sh

Then to start TunerStudio run the command:
./TunerStudio.sh

or to force 32 bit mode if your usb serial cable drivers don't support 64 bit:
./TunerStudio32.sh

