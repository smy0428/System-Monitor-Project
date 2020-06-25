#ifndef PROCESSOR_H
#define PROCESSOR_H



class Processor {
 public:
  float Utilization();
  Processor();

 private:
  float prevTotal;
  float prevNonIdle;
};

#endif