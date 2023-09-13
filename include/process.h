#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long int UpTime() const;
  bool operator<(Process const& a) const;

// Declare any necessary private members
 private:
  int pid_;
  long ram_;
  float cpu_utilization_;
  long uptime_;
  std::string user_;
  std::string command_;



};

#endif