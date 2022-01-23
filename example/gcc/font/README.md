# FONT load and save tool

`FONT <filename>`

Load PROGRAM image font upto 2K into VDP character pattern table

`FONT [/w] <filename>`

Load PROGRAM image font from Harry Wilhelm's XB256 distribution.

`FONT [/s] <filename>`

Save all 256 character pattern definitions to PROGRAM image file.

## Default Format

8 byte per character pattern definitions starting at character code 0. Upto 256 characters can be defined. PROGRAM image can be shorter, such as `fbForth` style `FBFONT` file which is just definitions 0-127.

## Wilhelm Format

The `/w` option uses the Wilhelm format which simply starts at the space ` ` character.

