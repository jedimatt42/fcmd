#include <fc_api.h>
#include "page.h"
#include "screen.h"
#include "version.h"
#include "history.h"
#include "gemini.h"

void about() {
  history_add_link("about:");
  page_clear_lines();
  page_add_line("# About " VERSION "\n");
  page_add_line("\n");
  page_add_line("=> //virgil.jedimatt42.com/ti994a/virgil/ Virgil99 homepage\n");
  page_add_line("\n");
  page_add_line("## Mouse usage:\n");
  page_add_line("\n");
  page_add_line("Scroll by clicking the mouse in the right hand 10 character column of the\n");
  page_add_line("screen. The mouse cursor will change when you hover over the region to\n");
  page_add_line("indicate the paging or scrolling action that will occur. Note, the action\n");
  page_add_line("occurs on mouse-up. You cannot hold the button down at this time. Near the\n");
  page_add_line("top right, is page up. Near the bottom right is page down. Closer to the\n");
  page_add_line("middle right is line up and line down.\n");
  page_add_line("\n");
  page_add_line("The top row the of the screen contains the title, and memory usage on the left. On the right are 'buttons' you can click to operate or interrupt the browser.\n");
  page_add_line("\n");
  page_add_line("* ADDRESS - opens a text input dialog with the current URL to edit\n");
  page_add_line("* BACK - reload the previous page\n");
  page_add_line("* STOP - stop loading the current page. This will take the place of the BACK button while a page is loading\n");
  page_add_line("* QUIT - exit the browser\n");
  page_add_line("\n");
  page_add_line("## Keyboard usage:\n");
  page_add_line("* ESC(F9) - exit the browser\n");
  page_add_line("* FCTN X - scroll down\n");
  page_add_line("* FCTN E - scroll up\n");
  page_add_line("* CTRL X - page down\n");
  page_add_line("* CTRL E - page up\n");
  page_add_line("\n");
  page_add_line("NOTE: stop, back, and address are not yet available from keyboard.\n");
  page_add_line("## Supported URL formats:\n");
  page_add_line("\n");
  page_add_line("No external protocols are supported at this time.\n");
  page_add_line("gemini://<hostname>/path\n");
  page_add_line("//<hostname>/path\n");
  page_add_line("about:\n");
  page_add_line("\n");
  page_add_line("## Link Colors:\n");
  page_add_line("\n");
  page_add_line("=> gemini://deadend.net Gemini URLs are Green\n");
  page_add_line("=> gopher://deadend.net Gopher URLS are Yellow\n");
  page_add_line("=> http://deadend.net HTTP URLS are Pale Yellow\n");
  screen_redraw();
}

