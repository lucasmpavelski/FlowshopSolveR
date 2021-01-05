#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

template <class T> void print(T b, T e) {
  using el = typename T::value_type;
  std::for_each(b, e, [](const el &e) { printf("%2d ", e); });
}

template <class T> void print(const std::vector<T> v) {
  print(std::begin(v), std::end(v));
}

template <class T> void print(const std::vector<T> v, int r) {
  int c = v.size() / r;
  auto beg = std::begin(v), end = beg;
  std::advance(end, c);
  for (int i = 0; i < r; i++) {
    print(beg, end);
    printf("\n");
    std::advance(beg, c);
    std::advance(end, c);
  }
}

template <class RNG>
std::vector<int> randomSolution(int no_jobs, int no_machines, RNG &rng) {
  std::vector<int> solution(no_machines * no_jobs);
  auto beg = std::begin(solution), end = beg;
  std::advance(end, no_machines);
  for (int i = 1; i <= no_jobs; i++) {
    std::fill(beg, end, i);
    std::advance(beg, no_machines);
    std::advance(end, no_machines);
  }
  std::shuffle(std::begin(solution), std::end(solution), rng);
  return solution;
}

struct JSP {
  int no_jobs, no_machines;
  std::vector<int> times;
  std::vector<int> orders;

  JSP(int no_jobs, int no_machines)
      : no_jobs(no_jobs), no_machines(no_machines),
        times(no_jobs * no_machines), orders(no_jobs * no_machines) {}
};

template <class RNG> void randomStructureJSP(JSP &jsp, RNG &rng) {
  auto beg = std::begin(jsp.orders), end = beg;
  std::advance(end, jsp.no_machines);
  for (int i = 0; i < jsp.no_jobs; i++) {
    std::iota(beg, end, 1);
    std::shuffle(beg, end, rng);
    std::advance(beg, jsp.no_machines);
    std::advance(end, jsp.no_machines);
  }
}

int schedule(const JSP &jsp, std::vector<int> solution) {
  //  tj = [0]*j   # end of previous task for each job
  //  tm = [0]*m   # end of previous task on each machine
  std::vector<int> machineTimes(jsp.no_machines, 0);
  std::vector<int> jobsTimes(jsp.no_jobs, 0);
  //  ij = [0]*j   # task to schedule next for each job
  std::vector<int> jobsMachinesScheduled(jsp.no_jobs, 0);
  std::vector<int> scheduledTimes(jsp.no_machines * jsp.no_jobs);

  //  for i in schedule:
  for (int operation : solution) {
    int job = operation;
    int job_idx = job;
    int machine_order = jobsMachinesScheduled[job];
    int machine_idx = jsp.orders[job_idx * jsp.no_machines + machine_order] - 1;
    int duration = jsp.times[job_idx * jsp.no_machines + machine_order];
    // accumulate job and machine starting times
    int start = std::max(jobsTimes[job_idx], machineTimes[machine_idx]);
    scheduledTimes[job_idx * jsp.no_machines + machine_idx] = start;
    jobsTimes[job_idx] = start + duration;
    machineTimes[machine_idx] = start + duration;
    // next time, process this job on the next machine
    jobsMachinesScheduled[job]++;
  }
  print(scheduledTimes, jsp.no_jobs);
  printf("\n\n");
  print(jobsTimes);
  printf("\n\n");
  print(machineTimes);
  printf("\n\n");
  return 0;
}

int schedule2(const JSP &jsp, const std::vector<int> &) {
  std::vector<int> edges((jsp.no_jobs + 2) * (jsp.no_jobs + 2));

  return 0;
}

/*template<class In, class Size, class Out>
Out copy_n(In first, Size n, Out result) {
    while( n-- ) *result++ = *first++;
    return result;
}*/

std::vector<int> line2IntVector(std::istream &line, int size) {
  std::vector<int> vec;
  vec.reserve(size);
  std::copy_n(std::istream_iterator<int>(line), size, std::back_inserter(vec));
  return vec;
}

std::vector<JSP> load(int no_jobs, int no_machines) {
  std::string folder =
      "data/instances/JSP/";
  std::string filename = "tai" + std::to_string(no_jobs) + "_" +
                         std::to_string(no_machines) + ".txt";
  std::string filePath = folder + filename;
  std::ifstream file(filePath.c_str());
  std::vector<JSP> insts;
  for (int inst = 0; inst < 10; inst++) {
    std::string line;
    // skip: Nb of jobs, Nb of Machines, Time seed, Machine seed, Upper bound,
    // Lower bound
    std::getline(file, line);
    std::vector<int> meta; // = line2IntVector(file, 6);
    std::copy_n(std::istream_iterator<int>(file), 6, std::back_inserter(meta));
    // skip: Times
    std::getline(file, line);
    std::getline(file, line);

    JSP jsp(meta[0], meta[1]);
    jsp.times.resize(0);
    jsp.times.reserve(jsp.no_jobs * jsp.no_machines);
    for (int i = 0; i < jsp.no_jobs; i++)
      std::copy_n(std::istream_iterator<int>(file), jsp.no_machines,
                  std::back_inserter(jsp.times));
    std::getline(file, line);
    // skip: Machines
    std::getline(file, line);
    jsp.orders.resize(0);
    jsp.orders.reserve(jsp.no_jobs * jsp.no_machines);
    for (int i = 0; i < jsp.no_jobs; i++)
      std::copy_n(std::istream_iterator<int>(file), jsp.no_machines,
                  std::back_inserter(jsp.orders));
    std::getline(file, line);

    insts.emplace_back(jsp);
  }
  return insts;
}

int main() {
  auto jsps{load(20, 15)};
  //  std::mt19937_64 rng;
  //  rng.seed(123);
  //  JSP jsp(4, 3);
  //  std::generate(std::begin(jsp.times), std::end(jsp.times), [&rng](){
  //    std::uniform_int_distribution<int> dist(1, 10);
  //    return dist(rng);
  //  });
  //  randomStructureJSP(jsp, rng);
  //  auto sol = randomSolution(jsp.no_jobs, jsp.no_machines, rng);
  //  print(sol);
  //  printf("\n\n");
  std::vector<int> sol = {
      11, 10, 1,  6,  19, 8,  17, 4,  0,  2,  7,  14, 18, 15, 9,  5,  16, 3,
      13, 12, 19, 17, 4,  6,  18, 7,  1,  8,  9,  5,  12, 11, 13, 14, 15, 10,
      2,  3,  16, 0,  1,  6,  2,  12, 9,  15, 16, 11, 4,  14, 19, 0,  3,  17,
      8,  13, 7,  18, 5,  10, 4,  2,  7,  9,  5,  12, 11, 19, 13, 16, 6,  1,
      17, 18, 8,  14, 3,  0,  15, 10, 19, 11, 6,  7,  1,  10, 13, 12, 16, 8,
      5,  9,  4,  0,  18, 2,  14, 15, 3,  17, 8,  17, 1,  3,  2,  12, 16, 19,
      6,  7,  4,  9,  13, 5,  11, 0,  14, 18, 10, 15, 9,  10, 11, 6,  2,  4,
      18, 15, 13, 8,  19, 7,  14, 5,  1,  16, 12, 0,  3,  17, 16, 15, 8,  3,
      0,  13, 18, 19, 2,  12, 9,  6,  17, 4,  10, 14, 5,  11, 1,  7,  17, 16,
      8,  0,  4,  18, 14, 7,  9,  5,  13, 11, 10, 19, 6,  12, 1,  15, 3,  2,
      3,  16, 0,  2,  12, 9,  4,  15, 6,  14, 13, 18, 8,  11, 5,  10, 17, 7,
      1,  19, 1,  3,  13, 15, 0,  14, 18, 2,  8,  12, 7,  6,  19, 10, 4,  17,
      11, 5,  16, 9,  6,  17, 0,  16, 13, 15, 7,  18, 5,  12, 11, 19, 1,  14,
      10, 9,  4,  8,  3,  2,  0,  7,  16, 8,  12, 2,  6,  18, 14, 5,  9,  19,
      10, 4,  3,  17, 15, 11, 13, 1,  19, 16, 3,  4,  0,  8,  14, 5,  12, 2,
      7,  1,  10, 6,  13, 9,  11, 18, 17, 15, 17, 15, 3,  12, 13, 18, 4,  10,
      8,  2,  9,  1,  5,  16, 6,  14, 19, 0,  11, 7,
  };
  print(jsps[3].times, jsps[3].no_jobs);
  printf("\n");
  print(jsps[3].orders, jsps[3].no_jobs);
  printf("\n");
  schedule(jsps[2], sol);
}

// solution
// 4  1  3  2  2  3  4  1  1  2  4  3
// times
// 4  6 10
// 8  2  2
// 10  3  9
// 5  3  6
// orders
// 2  3  1
// 1  2  3
// 1  3  2
// 2  1  3
// starting times
// 21  5 13
// 10 18 20
// 0 20 10
// 18  0 22
// jobs finishing times
// 31 22 29 28
// machines finishing times
// 31 29 28
// schedule
// 1	2	3	4	5	6	7	8	9	10
// 11 12 13 14	15	16	17	18	19	20	21	22 23 24
// 25	26	27	28	29	30	31
// 1	3	3	3	3	3	3	3	3	3
// 3
// 2
// 2
// 2	2	2	2	2	2	4	4	4	1
// 1 1 1	1	1	1	1	1	1
// 2	4	4	4	4	4	1	1	1	1
// 2	2	3	3	3	3	3	3	3	3
// 3
// 3 3
// 3
// 3
// 1
// 1	1	1	1	1		2	2	4	4
// 4 4 4	4
