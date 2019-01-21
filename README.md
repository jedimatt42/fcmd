# TIPICMD usage

## cd

Change directory

syntax: 'cd' (path)

Example: Change to TIPI.GAMES.EA5 folder

```
[1000.TIPI.]
$ cd TIPI.GAMES.EA5

[1000.TIPI.GAMES.EA5.]
$
```

Example: Change to floppy controller disk 2

```
[1000.TIPI.]
$ cd 1100.DSK2.

[1100.DSK2.]
$
```

Example: Change to parent folder

```
[1000.TIPI.GAMES.]
$ cd ..

[1000.TIPI.]
$
```

Example: Change into folder under current folder

```
[1000.TIPI.]
$ cd GAMES

[1000.TIPI.GAMES.]
$
```

## checksum

## copy

## delete

## dir

List files in current directory or given directory

syntax: 'dir' [path]

Example: List files in current folder

```
[1000.TIPI.]
$ dir

Vol: TIPI
TIPICFG    PRG
TIPICFH    PRG
TIPICFI    PRG
```

Example: List files in root folder of TIPI. device

```
[1100.DSK1.]
$ dir TIPI.

Vol: TIPI
TIPICFG    PRG
TIPICFH    PRG
TIPICFI    PRG
```

Example: List files in floppy drive DSK1 (if both TIPI and Floppy Controller are present)

```
[1000.TIPI.]
$ dir 1100.DSK1.

Vol: INFOCOM
LOAD       PRG
DEADLINE   PRG
DEAD       D/F 128
```

## drives 

List drives along with disambiguation

syntax: 'drives' 

Example:

```
$ drives

1000 - TIPI DSK0 DSK1 DSK2 DSK3
1100 - DSK1 DSK2 DSK3 DSK4
```

## exit

Exit the program

syntax: 'exit'

Example:

```
[1000.TIPI.]
$ exit
```

## lvl2

## mkdir

## protect

## rename

## rmdir

## unprotect

## ver

Print information about TIPIFM

syntax: 'ver'

Example:

```
[1000.TIPI.]
$ ver

TIPICMD v1
Command Shell for TIPI
www.jedimatt42.com
```

## width

Change display width. TIPICMD auto-detects F18A and starts in best mode.

syntax: 'width' (40|80)

Example: Set 80 column mode

```
[1000.TIPI.]
$ width 80
```

Example: Set 40 column mode

```
[1000.TIPI.]
$ width 40
```

