#include "process.h"
#include "linux_parser.h"
#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <string>
#include <fstream>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {
  pid_ = pid;
  command_ = LinuxParser::Command(pid_);
  string ram_str = LinuxParser::Ram(pid_);
  ram_ = std::stol(ram_str);
  uptime_ = LinuxParser::UpTime(pid_);
  user_ = LinuxParser::User(pid_);

  long seconds = LinuxParser::UpTime() - uptime_;
  long total_time = LinuxParser::ActiveJiffies(pid_);
  try {
    cpu_utilization_ = float(total_time) / float(seconds);
  } catch (...) {
    cpu_utilization_ = 0.0;
  }
}
// TODO: Return this process's ID
int Process::Pid() const {return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_utilization_; }

// TODO: Return the command that generated this process
string Process::Command() const { return command_; }

// TODO: Return this process's memory utilization
string Process::Ram() const { return std::to_string(ram_); }

// TODO: Return the user (name) that generated this process
string Process::User() const { return user_; }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() const { return uptime_; }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a [[maybe_unused]]) const {
  return CpuUtilization() < a.CpuUtilization();
}