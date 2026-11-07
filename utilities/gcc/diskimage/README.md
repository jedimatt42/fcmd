# DISKIMAGE copy DSKx to DIS/FIX 128 file

## Create Image from Disk

`DISKIMAGE /r <device> <filename>`

Creates a disk image in DISPLAY FIXED 128 format on the 4A filesystem. 

- device : the input drive to read from such as `DSK1.`
- filename : the file path for the sector dump file

Produces the equivalent of a 4A .DSK sector dump file as a DIS/FIX 128 file.
Each record in the file is half a sector.


## TIPI notes: 

To save the image file without a TIFILES FIAD header, so it is truly
the same format that pre-existing tools expect, use the TIPI path flag to force
headerless file saves: ?X 

For example, to copy the real floppy in DSK1. to the //TIPI/TIPI/SECTOR.IMG 

```
DISKIMAGE 1100.DSK1. TIPI.?X.SECTOR/IMG
```

If you save to a file ending in /DSK (.DSK on the host OS) then the tipimon.service
will detect the disk image, and unpack it to a directory. Usually not what you 
want to do, although it may be useful in certain circumstances.

## Write Image to Disk

`DISKIMAGE /w <filename> <device>`

Write a disk image in DISPLAY FIXED 128 format to a floppy device.

- filename : the file path for the sector dump file
- device : the output drive to write to such as `DSK1.`

The destination floppy should have been formatted at least once already.

