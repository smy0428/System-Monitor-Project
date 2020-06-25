#include <string>
#include "format.h"

using std::string;
using std::to_string;


// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
  int hh = 0, mm = 0, ss = 0;
  // get the remainder of a day
  seconds = seconds % 86400;
  hh = seconds / 3600;
  seconds = seconds % 3600;
  mm = seconds / 60;
  ss = seconds % 60;
  return Format::Helper(hh) + ":" +  Format::Helper(mm) + ":" + Format::Helper(ss);
}

string Format::Helper(int i) {
  if (i < 10) {
  	return "0" + to_string(i);
  }
  return to_string(i);
}