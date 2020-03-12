#include "FSPData.hpp"

FSPData::FSPData(const std::string &filename) {
  std::string buffer;
  std::string::size_type start, end;
  // opening of the benchmark file
  std::ifstream inputFile(filename, std::ios::in);
  auto getline_check = [&filename](std::ifstream &inputFile,
                                   std::string &buff) {
    getline(inputFile, buff, '\n');
    if (!inputFile) {
      std::string err = "ERROR: Unable to read the benchmark file " + filename;
      throw std::runtime_error(err);
    }
  };
  // number of jobs (N)
  getline_check(inputFile, buffer);
  no_jobs = atoi(buffer.data());
  // number of machines M
  getline_check(inputFile, buffer);
  no_machines = atoi(buffer.data());
  // initial and current seeds (not used)
  getline_check(inputFile, buffer);
  // processing times and due-dates
  proc_times.resize(no_jobs * no_machines);
  total_job_proc_times.resize(no_jobs);
  total_machine_proc_times.resize(no_machines);
  // for each job...
  for (int j = 0; j < no_jobs; j++) {
    // index of the job (<=> j)
    getline_check(inputFile, buffer);
    // due-date of the job j
    getline_check(inputFile, buffer);
    // processing times of the job j on each machine
    getline_check(inputFile, buffer);
    start = buffer.find_first_not_of(" ");
    for (int i = 0; i < no_machines; i++) {
      end = buffer.find_first_of(" ", start);
      proc_times[i * no_jobs + j] =
          atoi(buffer.substr(start, end - start).data());
      start = buffer.find_first_not_of(" ", end);
    }
  }
  // closing of the input file
  inputFile.close();
  init();
}
