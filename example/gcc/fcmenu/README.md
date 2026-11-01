# FCMenu

Force Command Menu utility.

## Configuration

```
Usage: FCMENU <datafile>
```

* datafile - A DISPLAY VARIABLE 80 text file containing menu entry definitions.

Empty lines are ignored.
Lines beginning with a `#` are ignored.

All other lines must have the format:

`<key>` '|' `<title>` '|' `<command line>`

* key - a single character keyboard shortcut for the item
* title - the label to display, should not be longer than 16 characters.
* command - the ForceCommand statement to run in response to item selection.

If key is the hyphone/minus character `-`, then title and command
are ignored, and the item causes a separator to be displayed in the
menu.

There can be 100 menu entries, including separators.

Another instance of FCMENU may be run with a different data file to create nested menus.

## Keys

* F9 - BACK, exits the current menu.
* `.` - PERIOD, or `>` move forward a page
* `,` - COMMA, or `<` move back a page

## Examples

```
# Load TI Extended Basic cartridge
X|Extended BASIC|FG99 TIXB_G
```

```
# create a horizontal bar
-|-|-
```

```
# Load an EA5 game
P|Parsec|LOAD TIPI.GAMES.EA5.PARSEC1
```

## Limitations

On an 80 column display, you should be able to display 40 items per page. A 40 column display will only show 20 items per page.

The config file should not have more than 100 items (this will be enforced in a future build)

ForceCommand requires SAMS memory. For each ForceCommand executable, such as FCMENU or ForceCommand's FTP, it provides a new set of pages for upper memory expansion. Upon exit, the previous calling executable is paged back in and resumes control.

Mouse is only supported on F18A 80x30 column mode.
