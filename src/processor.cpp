#include "processor.h"
#include "linux_parser.h"
#include <vector>

using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  float currTotal = LinuxParser::Jiffies();
  float currNonIdle = LinuxParser::ActiveJiffies();
  float deltaTotal = currTotal - prevTotal;
  float deltaNonIdle = currNonIdle - prevNonIdle;
  
  prevTotal = currTotal;
  prevNonIdle = currNonIdle; 
  float res = deltaNonIdle / deltaTotal;
  // check the res with 0.0 otherwise the code crash.
  return (res>0.0)? res : 0.0;
}

Processor::Processor() {
  prevTotal = LinuxParser::Jiffies();
  prevNonIdle = LinuxParser::ActiveJiffies();
}