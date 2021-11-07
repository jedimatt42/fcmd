# VIRGIL99

TI-99/4A Gemini Protocol Browser

* Gemini Protocol Project => https://gemini.circumlunar.space/

## About

This is a text mode browser for the text friendly `gemini` protocol over TLS. 

## Requirements

* TI-99/4A
* SAMS
* F18A VDP
* TIPI
* usb-mouse connected to TIPI
* Force Command

## Usage

```
VIRGIL99 gemini://gemini.circumlunar.space/
```

This will open the gemini url given, and display the page.

## TODO

* first line of each new SAMS page is missing (every 48th line)
* long urls wrap and become disfunctional
* history is wonkey again
* eof... if last line doesn't have a 0x0A, then it isn't shown.

I want to implement:

* file:///urls... 
* SAMS paging for code... 
* file download
* view history
* bookmarks

