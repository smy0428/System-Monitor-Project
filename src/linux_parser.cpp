#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>

#include "linux_parser.h"

using std::stoi;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;
using std::map;

// An example of how to read data from the filesystem
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

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
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

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  map<string, float> memoeryMap {
    {"MemTotal:", 0}, {"MemFree:", 0}, {"Buffers:", 0}, {"Cached:", 0}, {"Shmem:", 0}, {"SReclaimable:", 0}
  };
  string line;
  string key;
  float value;
  bool flag = true;
  
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (flag && std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      auto search = memoeryMap.find(key);
      if (flag && search != memoeryMap.end()) {
      	memoeryMap[key] = value;
        if (key == "SReclaimable:") {
          flag = false;
        }
      }
    }
  }
  
  float totalUsedM = memoeryMap["MemTotal:"] - memoeryMap["MemFree:"];
  float cachedM = memoeryMap["Cached:"] - memoeryMap["Shmem:"] + memoeryMap["SReclaimable:"];
  float nonCacheBufferM = totalUsedM - (memoeryMap["Buffers:"] + cachedM);
  return nonCacheBufferM / memoeryMap["MemTotal:"];
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  long uptime = 0, idletime = 0;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  return uptime; 
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  // indices from 13 to 16
  string line;
  string path = kProcDirectory + to_string(pid) + kStatFilename;
  vector<string> infos{};
  string info;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> info) {
      infos.push_back(info);
    }
  }
  
  long res = 0;
  for (int i = 13; i <=16; i++) {
    res += stol(infos[i]);
  }
  return res;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<long> CPUs = LinuxParser::CpuUtilization();
  return CPUs[CPUStates::kUser_]    + CPUs[CPUStates::kNice_]
       + CPUs[CPUStates::kSystem_]  + CPUs[CPUStates::kIRQ_]
       + CPUs[CPUStates::kSoftIRQ_] + CPUs[CPUStates::kSteal_];
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<long> CPUs = LinuxParser::CpuUtilization();
  // idleJiffies = idle + iowait
  return CPUs[CPUStates::kIdle_] + CPUs[CPUStates::kIOwait_];
}

// Read and return CPU utilization
vector<long> LinuxParser::CpuUtilization() {
  string line;
  string head;
  long num;
  vector<long> CPUs{};
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    // cut the "cpu" head from line
    linestream >> head;
    while(linestream >> num) {
      CPUs.push_back(num);
    }
  }
  return CPUs; 
}


// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string name;
  int num = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> name >> num;
      if (name == "processes") {
      	return num;
      }
    }
  }
  return num;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string name;
  int num = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> name >> num;
      if (name == "procs_running") {
      	return num;
      }
    }
  }
  return num;
}


// Calculate the cpuutil for each pid
float LinuxParser::CpuUtilization(int pid) {
  
  string line;
  string path = kProcDirectory + to_string(pid) + kStatFilename;
  long uptime = LinuxParser::UpTime(pid);
  vector<string> infos{};
  string info;
  
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> info) {
      infos.push_back(info);
    }
  }
  
  float total_time = 0;
  for (int i = 13; i <=16; i++) {
    total_time += stof(infos[i]);
  }
  
  float cpu_usage = 1.0 * total_time / sysconf(_SC_CLK_TCK) / uptime;
  return cpu_usage;
}


// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line{};
  string path = kProcDirectory + to_string(pid) + kCmdlineFilename;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, value;
  string path = kProcDirectory + to_string(pid) + kStatusFilename;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> value;
        value = to_string(stol(value) / 1000);
        return value;
      }
    }
  }
  return value;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, value;
  string path = kProcDirectory + to_string(pid) + kStatusFilename;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> value;
        return value;
      }
    }
  }
  return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, key, xx, value;
  string uid = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> value >> xx >> key;
      if (key == uid) {
        return value;
      }
    }
  }
  return value;
}


// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string path = kProcDirectory + to_string(pid) + kStatFilename;
  string info;
  vector<string> infos;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> info) {
      infos.push_back(info);
    } 
  } 
  return LinuxParser::UpTime() - stol(infos[21]) / sysconf(_SC_CLK_TCK);
}