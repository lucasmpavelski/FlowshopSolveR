#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <valarray>

#include "flowshop-solver/global.hpp"

struct FSPData {
  using ivec = std::vector<int>;

  FSPData(const std::string& filename);

  FSPData(int _no_jobs = 20, int _no_machines = 5, int max = 99)
      : no_jobs{_no_jobs},
        no_machines{_no_machines},
        max_ct{0},
        total_job_proc_times(_no_jobs),
        total_machine_proc_times(_no_machines) {
    using std::begin;
    using std::end;
    proc_times.resize(no_jobs * no_machines);
    std::generate(begin(proc_times), end(proc_times),
                  [max]() { return rng.random(max) + 1; });
    init();
  }

  FSPData(std::vector<int> pts, int no_jobs)
      : no_jobs{no_jobs},
        no_machines{static_cast<int>(pts.size() / no_jobs)},
        max_ct{0},
        proc_times(pts),
        total_job_proc_times(no_jobs),
        total_machine_proc_times(no_machines) {
    init();
  }

  friend std::ostream& operator<<(std::ostream& o, const FSPData& d) {
    o << "FSPData:\n"                                 //
      << "  no_jobs: " << d.no_jobs << '\n'           //
      << "  no_machines: " << d.no_machines << '\n';  //
    int w = int(std::log(d.max_ct) / std::log(10)) + 2;
    auto sw = std::setw(w);
    int pad = w + w * (static_cast<int>(d.no_jobs) - 1) - 1;
    o << std::left << std::setw(pad) << " " << sw << "job" << '\n'
      << sw << "mac";
    for (int j = 0; j < d.no_jobs; j++)
      o << sw << j;
    o << "sum\n";
    for (int i = 0; i < d.no_machines; i++) {
      o << sw << i;
      for (int j = 0; j < d.no_jobs; j++)
        o << sw << d.proc_times[i * d.no_jobs + j];
      o << '+' << sw << d.total_machine_proc_times[i] << '\n';
    }
    o << sw << "sum";
    for (int j = 0; j < d.no_jobs; j++)
      o << "+" << std::setw(w - 1) << d.total_job_proc_times[j];
    o << "+" << std::setw(w - 1) << d.max_ct;
    return o;
  }

  int noJobs() const { return no_jobs; }
  int noMachines() const { return no_machines; }
  int maxCT() const { return max_ct; }
  int lowerBound() const { return lower_bound; }
  ivec& procTimesRef() { return proc_times; }
  const ivec& procTimesRef() const { return proc_times; }
  ivec& machineProcTimesRef() { return total_machine_proc_times; }
  const ivec& machineProcTimesRef() const { return total_machine_proc_times; }
  ivec& jobProcTimesRef() { return total_job_proc_times; }
  const ivec& jobProcTimesRef() const { return total_job_proc_times; }

  int pt(const int j, const int m) const { return proc_times[m * no_jobs + j]; }
  int& pt(const int j, const int m) { return proc_times[m * no_jobs + j]; }

  int partialSumOnAdjacentMachines(int job, int i, int h) const {
    assert(i <= h);
    int sm = 0;
    for (int j = i; j <= h; j++) {
      sm += pt(job, j);
    }
    return sm;
  }

 private:
  void init() {
    total_job_proc_times.resize(no_jobs);
    total_machine_proc_times.resize(no_machines);
    std::fill(total_job_proc_times.begin(), total_job_proc_times.end(), 0);
    std::fill(total_machine_proc_times.begin(), total_machine_proc_times.end(),
              0);
    max_ct = 0;
    for (int i = 0; i < no_jobs; i++) {
      for (int j = 0; j < no_machines; j++) {
        total_job_proc_times[i] += pt(i, j);
      }
    }
    for (int i = 0; i < no_machines; i++) {
      for (int j = 0; j < no_jobs; j++) {
        total_machine_proc_times[i] += pt(j, i);
      }
    }
    max_ct = std::accumulate(total_job_proc_times.begin(),
                             total_job_proc_times.end(), 0u);
  }

  int no_jobs, no_machines, max_ct, lower_bound;
  ivec proc_times, total_job_proc_times, total_machine_proc_times;
};
