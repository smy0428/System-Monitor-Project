#include "ncurses_display.h"
#include "system.h"


using std::to_string;

int main() {
  System system;
  NCursesDisplay::Display(system);
}