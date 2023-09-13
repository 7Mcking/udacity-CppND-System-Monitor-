#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>
#include <iostream>
using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::stol;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key;
  float mem_total{1}, mem_free{1};
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "MemTotal:") {
        linestream >> mem_total;
      } else if (key == "MemFree:") {
        linestream >> mem_free;
        break;
      }
    }
  }
  return (mem_total - mem_free)/mem_total;
}


// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long system_uptime;
  string line, uptimestring;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream uptimeStream(line);
    uptimeStream >> uptimestring;
  }
  system_uptime = std::stol(uptimestring);
  return system_uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid ) {
  long total_time{0};
  string line, value;
  vector<string> values;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream basic_string_stream(line);
        while(basic_string_stream >> value){
          values.push_back(value);
        }
  }

  // Making sure that the values are not out of range
  long utime{0}, stime{0}, cutime{0}, cstime{0};
  if(std::all_of(values[13].begin(), values[13].end(), isdigit)){
    utime = std::stol(values[13]);
  }
  if(std::all_of(values[14].begin(), values[14].end(), isdigit)){
    stime = std::stol(values[14]);
  }
  if(std::all_of(values[15].begin(), values[15].end(), isdigit)){
    cutime = std::stol(values[15]);
  }
  if(std::all_of(values[16].begin(), values[16].end(), isdigit)){
    cstime = std::stol(values[16]);
  }

  total_time = utime + stime + cutime + cstime;
  return total_time/ sysconf(_SC_CLK_TCK);
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  return std::stol(jiffies[CPUStates::kUser_]) + std::stol(jiffies[CPUStates::kNice_]) +
         std::stol(jiffies[CPUStates::kSystem_]) + std::stol(jiffies[CPUStates::kIRQ_]) +
         std::stol(jiffies[CPUStates::kSoftIRQ_]) +
         std::stol(jiffies[CPUStates::kSteal_]);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();
  return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, cpu;
  int value;
  vector<string> values;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream basic_string_stream(line);
    basic_string_stream >> cpu;
    while(basic_string_stream >> value){
      if (basic_string_stream){
        values.push_back(to_string(value));
      }
    }
  }
  return values;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int total_processes{0};
  string  key, line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){
        while(std::getline(filestream, line)){
      std::istringstream basic_string_stream(line);
      basic_string_stream >> key;
      if(key == "processes"){
        basic_string_stream >> total_processes;
        break;
      }
    }
  }
  return total_processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int running_processes{0};
  string  key, line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::istringstream basic_string_stream(line);
        basic_string_stream >> key;
        if(key == "procs_running"){
          basic_string_stream >> running_processes;
          break;
      }
    }
  }
  return running_processes;
}

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid ) {
  string command;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(filestream.is_open()){
    std::getline(filestream, command);
  }
  return command;
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, ram;
  long memory{0};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::istringstream basic_string_stream(line);
      basic_string_stream >> key;
      if(key == "VmSize:"){
        basic_string_stream >> memory;
        memory = memory/1000;
        ram = to_string(memory);
        break;
      }
    }
  }
  return ram;
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::istringstream basic_string_stream(line);
      basic_string_stream >> key;
      if(key == "Uid:"){
        basic_string_stream >> uid;
        break;
      }
    }
  }
  return uid;
}

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string uid_, user, value, line;
  string name = "DEFAULT";
  std::ifstream filestream(kPasswordPath);
  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream basic_string_stream(line);
      basic_string_stream >> user >> value >> uid_;
      if(uid_ == uid){
        name = user;
        break;
      }
    }
  }
  return name;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  long start_time{0};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream basic_string_stream(line);
    while(basic_string_stream >> value){
      values.push_back(value);
    }
  }
  try {
    start_time = std::stol(values[21]);
      } catch (const std::invalid_argument& arg) {
    return 0;
  }
  return start_time;
}
