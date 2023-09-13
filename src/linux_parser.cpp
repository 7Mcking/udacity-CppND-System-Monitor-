#include "linux_parser.h"
#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::stol;
using std::ifstream;
using std::istringstream;
using std::all_of;
using std::getline;

// read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value; //Return OS Version
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream line_stream(line);
    line_stream >> os >> version >> kernel;
  }
  return kernel; // Return Kernel Version
}

// BONUS: Update this to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  for (const auto& entry : std::filesystem::directory_iterator(kProcDirectory)) {
    if (entry.is_directory()) {
      std::string filename = entry.path().filename().string();
      if (all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = std::stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float mem_total = 1.0f, mem_free = 1.0f;
  ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (!filestream.is_open()) {
    return 0.0f; // Return 0 if the file couldn't be opened.
  }

  std::string line;
  while (getline(filestream, line)) {
    istringstream line_stream(line);
    std::string key;
    line_stream >> key;

    if (key == "MemTotal:") {
      line_stream >> mem_total;
    } else if (key == "MemFree:") {
      line_stream >> mem_free;
      break;
    }
  }

  if (mem_total == 0.0f) {
    return 0.0f; // Avoid division by zero.
  }

  return (mem_total - mem_free) / mem_total;
}

//  Read and return the system uptime
long LinuxParser::UpTime() {
  ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    long system_uptime;
    filestream >> system_uptime;
    return system_uptime;
  } else {
    return 0; // Return 0 to indicate an error if the file couldn't be opened.
  }
}

//  Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

//  Read and return the number of active jiffies for a PID

long LinuxParser::ActiveJiffies(int pid ) {
  long total_time{0};
  string line, value;
  vector<string> values;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    getline(filestream, line);
    istringstream basic_string_stream(line);
        while(basic_string_stream >> value){
          values.push_back(value);
        }
  }
  long up_time{0}, system_time{0}, cutime{0}, cstime{0};

  up_time = std::stol(values[13]);

  system_time = std::stol(values[14]);

  cutime = std::stol(values[15]);

  cstime = std::stol(values[16]);

  total_time = up_time + system_time + cutime + cstime;

  return total_time/ sysconf(_SC_CLK_TCK);
}

//  Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  return std::stol(jiffies[CPUStates::kUser_]) + std::stol(jiffies[CPUStates::kNice_]) +
         std::stol(jiffies[CPUStates::kSystem_]) + std::stol(jiffies[CPUStates::kIRQ_]) +
         std::stol(jiffies[CPUStates::kSoftIRQ_]) +
         std::stol(jiffies[CPUStates::kSteal_]);
}

//  Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();
  return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

//  Read and return CPU utilization
vector<std::string> LinuxParser::CpuUtilization() {
  ifstream filestream(kProcDirectory + kStatFilename);
  if (!filestream.is_open()) {
        return {};  // Return an empty vector to indicate an error if
                    // the file couldn't be opened.
  }

  vector<std::string> values;
  string line;
  getline(filestream, line);
  istringstream line_stream(line);

  // Read the CPU label and values into the vector directly.
  line_stream >> line; // Read and discard the "cpu" label.
  std::copy(std::istream_iterator<std::string>(line_stream),
      std::istream_iterator<std::string>(),
          std::back_inserter(values));

  return values;
}

//  Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int total_processes{0};
  ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){
      string  key, line;
      while(getline(filestream, line)){
      istringstream string_stream(line);
      string_stream >> key;
      if(key == "processes"){
        string_stream >> total_processes;
        break;
      }
    }
  }
  return total_processes; //Return total number of running Processes
}

//  Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int running_processes{0};
  ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){
    string  key, line;
    while(getline(filestream, line)){
      istringstream basic_string_stream(line);
      basic_string_stream >> key;
      if(key == "procs_running"){
        basic_string_stream >> running_processes;
        break;
      }
    }
  }
  return running_processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid ) {
  string command;
  ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(filestream.is_open()){
    getline(filestream, command);
  }
  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string ram;
  long memory{0};
  ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(filestream.is_open()){
    string line, key;
    while(getline(filestream, line)){
      istringstream basic_string_stream(line);
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

//  Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(filestream.is_open()){
    while(getline(filestream, line)){
      istringstream basic_string_stream(line);
      basic_string_stream >> key;
      if(key == "Uid:"){
        basic_string_stream >> uid;
        break;
      }
    }
  }
  return uid;
}

//  Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string uid_, user, value, line;
  string name = "";
  ifstream filestream(kPasswordPath);
  if(filestream.is_open()){
    while(getline(filestream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      istringstream basic_string_stream(line);
      basic_string_stream >> user >> value >> uid_;
      if(uid_ == uid){
        name = user;
        break;
      }
    }
  }
  return name;
}

//  Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  long start_time{0};
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    getline(filestream, line);
    istringstream basic_string_stream(line);
    while(basic_string_stream >> value){
      values.push_back(value);
    }
  }
  try {
    start_time = std::stol(values[21]);
      } catch (...) {
    return 0;
  }
  return start_time;
}
