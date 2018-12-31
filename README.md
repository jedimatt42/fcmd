# gcc pattern for bank switched cartridge ROM

## Theory:

us a trampoline to store the caller bank and return address
onto a stack, then switch banks, and call destination.

This will require some sort of FAR_CALL macro. 

Getting this onto the gcc stack in the right order to not 
require knowing all the parameters will be difficult...

### Experiments

start with zero argument methods. Build from there as we 
infer the calling convention.

