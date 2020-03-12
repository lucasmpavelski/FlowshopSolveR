#include <vector>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <random>
#include <mo>

#include "problems/FSPEvalFunc.hpp"

/*
#include "eigen3/Eigen/Dense"
template<class T>
void print(T b, T e) {
  std::for_each(b, e, [](const auto& e) {
    printf("%2d ", e);
  });
}

template<class T>
void print(const std::vector<T> v) {
  print(std::begin(v), std::end(v));
}

template<class T>
void print(const std::vector<T> v, int r) {
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

template<class RNG>
std::vector<int> randomSolution(int no_jobs, RNG& rng) {
  std::vector<int> sol(no_jobs);
  std::iota(std::begin(sol), std::end(sol), 0);
  std::shuffle(std::begin(sol), std::end(sol), rng);
  return sol;
}

struct MMASParams {
  int n;
  float rho = 0.9;
  float p0 = 0.5;
  float tau_min = 0.1;
  float tau_max = 0.9;
  // pheromones(i,j) = desire of setting job i at the j th position
  Eigen::MatrixXf pheromones;

  using EOT = FSP;

  MMASParams(int n) :
    n(n), pheromones(n, n) {
    pheromones = Eigen::MatrixXf::Constant(n, n, 0.5);
  }

  template<class RNG>
  EOT constructSolution(RNG& rng) {
    EOT sol;
    sol.reserve(n);
    std::uniform_real_distribution<float> unif(0.0f, 1.0f);
    std::uniform_int_distribution<int> unif_int(0, n - 1);
    Eigen::MatrixXf probs = pheromones;
    long int chosen_job = -1;
    for (int j = 0; j < n; j++) {
      if (unif(rng) < p0) {
        // find the job that has the higher desire to be in j th position
        probs.col(j).maxCoeff(&chosen_job);
      }
      else {
        float sum = probs.col(j).sum();
        if (sum <= 1e-6) {
          // choose randomly if there are not enough values (or all ties at zero)
          do {
              chosen_job = unif_int(rng);
          } while (std::find(sol.begin(), sol.end(), chosen_job) != sol.end());
        } else {
          // use the probabilities as a distribution
          float r = unif(rng) * sum;
          float cum_sum = 0.0;
          chosen_job = -1;
          do {
            chosen_job++;
            cum_sum += probs(chosen_job, j);
          } while (cum_sum < r);
        }
      }
      sol.push_back(chosen_job);
      probs.row(chosen_job) = Eigen::VectorXf::Constant(n, 0);
    }
    return sol;
  }

  void update(const EOT& sol) {
    pheromones = rho * pheromones;
    for (int i = 0; i < sol.size(); i++)
      pheromones(i, sol[i]) += 1.0 / sol.fitness();
  }
};

void test1() {
  std::mt19937_64 rng;
  rng.seed(123u);
  MMASParams mmas(5);
  mmas.p0 = 1;
  mmas.pheromones <<
       0,.8,1,0,0,
      .5, 1,0,0,0,
       1, 0,0,0,0,
       0, 0,0,0,1,
       0, 0,0,1,0;
  std::vector<int> sol = mmas.constructSolution(rng);
  assert(sol[0] == 2);
  assert(sol[1] == 1);
  assert(sol[2] == 0);
  assert(sol[3] == 4);
  assert(sol[4] == 3);
}

void test2() {
  std::mt19937_64 rng;
  rng.seed(123u);
  MMASParams mmas(5);
  mmas.p0 = 0;
  mmas.pheromones <<
       0, 0,1,0,0,
       0, 1,0,0,0,
       1, 0,0,0,0,
       0, 0,0,0,1,
       0, 0,0,1,0;
  std::cout << mmas.pheromones << "\n";
  std::vector<int> sol = mmas.constructSolution(rng);
  assert(sol[0] == 2);
  assert(sol[1] == 1);
  assert(sol[2] == 0);
  assert(sol[3] == 4);
  assert(sol[4] == 3);
}

void test3() {
  std::mt19937_64 rng;
  rng.seed(123u);
  MMASParams mmas(5);
  mmas.p0 = 0;
  mmas.pheromones <<
       0.0,0.0,0.5,0.0,0.0,
       0.0,0.5,0.5,0.0,0.0,
       0.5,0.5,0.0,0.0,0.5,
       0.5,0.0,0.0,0.5,0.5,
       0.0,0.0,0.0,0.5,0.0;
  std::cout << mmas.pheromones << "\n";
  std::vector<int> sol = mmas.constructSolution(rng);
  std::vector<int> perm(5);
  std::iota(perm.begin(), perm.end(), 0);print(sol);
  print(perm);
  assert(sol[0] == 2 || sol[0] == 3);
  assert(sol[1] == 1 || (sol[1] == 2 && sol[0] == 3));
  assert(sol[1] == 1 || (sol[1] == 2 && sol[0] == 3));
  assert(std::is_permutation(sol.begin(), sol.end(), perm.begin()));
}

void testCanGenerateAnySequence() {
  std::mt19937_64 rng;
  rng.seed(123u);
  std::vector<std::vector<int>> perms;
  perms.reserve(120);
  std::vector<int> current(5);
  std::iota(current.begin(), current.end(), 0);
  std::vector<int> perm = current;
  do {
    perms.push_back(perm);
  } while(std::next_permutation(perm.begin(), perm.end()));
  MMASParams mmas(5);
  mmas.p0 = 0;
  mmas.pheromones <<
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5;
  std::vector<int> counts(perms.size(), 0);
  for (int i = 0; i < 1000000; i++) {
    std::vector<int> sol = mmas.constructSolution(rng);
    for (int j = 0; j < perms.size(); j++) {
      if (std::equal(perms[j].begin(), perms[j].end(), sol.begin())) {
        counts[j]++;
        break;
      }
    }
  }
  for (int j = 0; j < perms.size(); j++) {
    print(perms[j]);
    printf(" - %d\n", counts[j]);
  }
}



void testUpdate() {
  std::mt19937_64 rng;
  rng.seed(123u);
  std::vector<std::vector<int>> perms;
  perms.reserve(120);
  std::vector<int> current(5);
  std::iota(current.begin(), current.end(), 0);
  std::vector<int> perm = current;
  do {
    perms.push_back(perm);
  } while(std::next_permutation(perm.begin(), perm.end()));
  MMASParams mmas(5);
  mmas.p0 = 0;
  mmas.pheromones <<
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5,
      0.5,0.5,0.5,0.5,0.5;
  std::vector<int> counts(perms.size(), 0);
  for (int i = 0; i < 5000; i++) {
    FSP fsp;
    std::copy(perms[1].begin(), perms[1].end(), std::back_inserter(fsp));
    fsp.fitness(10);
    mmas.update(fsp);
    fsp.resize(0);
    std::copy(perms[0].begin(), perms[0].end(), std::back_inserter(fsp));
    fsp.fitness(100);
    mmas.update(fsp);
  }
  std::cout << mmas.pheromones;
}

#include "flowshop-solver/fspproblemfactory.h"
#include "flowshop-solver/MHSolve.h"

void runMMAS() {
  initFactories("/home/lucasmp/projects/git/evolutionary_tunners/data/instances/generated_intances/generated_instances_all/",
                "/home/lucasmp/projects/git/evolutionary_tunners/data/specs/", true);
  std::unordered_map<std::string, std::string> prob_data;


  prob_data["problem"] = "FSP";
  prob_data["budget"] = "high";
  prob_data["type"] = "NOIDLE";
  prob_data["objective"] = "FLOWTIME";
  prob_data["instance"] = "taill-like_30_20_3020104.gen";
  auto prob = FSPProblemFactory::get(prob_data);


  using Ngh = typename FSPProblem::Ngh;
  using EOT = Ngh::EOT;

  const int N = prob.size(0);
  const int M = prob.size(1);
  const int max_nh_size = pow(N - 1, 2);
  const std::string mh = "ACO";
  const int maxEval = prob.maxEvals();
  const double max_ct = prob.upperBound();

  // validate problem state
  if (prob.noEvals() >= maxEval)
     throw std::runtime_error("Error: the problem does not have evaluations left! " +
                              std::to_string(prob.noEvals()) + "/" + std::to_string(maxEval));
  else if (prob.noEvals() != 0)
    std::cerr << "Warning: number of evaluations is not cleared! (" << prob.noEvals() << ")";

  // continuator
  moContinuator<Ngh> &continuator = prob.continuator();
  eoEvalFuncCounter<EOT> &fullEval = prob.eval();
  moEvalCounter<Ngh> &evalN = prob.neighborEval();

  moCheckpoint<Ngh> checkpoint(continuator);
  moBestSoFarStat<FSP> bestFound(true);
  checkpoint.add(bestFound);
  moCheckpoint<Ngh> checkpointGlobal(continuator);
  moBestSoFarStat<FSP> bestFoundGlobal(false);
  checkpoint.add(bestFoundGlobal);

//  // debug
////  prefixedPrinter print("local_best:", " ");
////  print.add(bestFound);
////  prefixedPrinter printg("global_best:", " ");
////  printg.add(bestFoundGlobal);
////  checkpoint.add(print);
////  checkpointGlobal.add(printg);

//  // initialization
  eoInitPermutation<FSP> init0(N);
  // comparator strategy
  moSolComparator<FSP> compSS0;              // comp sol/sol strict
  moSolNeighborComparator<Ngh> compSN0;      // comp sol/Ngh strict
  moNeighborComparator<Ngh> compNN0;         // comp Ngh/Ngh strict
  int nh_size = max_nh_size;

  moOrderNeighborhood<Ngh> neighborhood0(nh_size);
  moFirstImprHC<Ngh> fi(neighborhood0, fullEval, evalN, checkpoint, compNN0, compSN0);

  MMASParams mmas(N);
  std::mt19937_64 rng;
  rng.seed(123u);

  EOT sol;
  init0(sol);
  fullEval(sol);
  std::cout << sol << "\n";
  mmas.update(sol);

  while (checkpointGlobal(sol)) {
    sol = mmas.constructSolution(rng);
    fullEval(sol);
    fi(sol);
    mmas.update(sol);
  }

  std::cout << sol << "\n";
  std::cout << "best_global: " << bestFoundGlobal.value().fitness() << "\n";
  std::cout << "best_local: " << bestFound.value().fitness() << "\n";
}

int main(int argc, char *argv[]) {
  std::mt19937_64 rng;
  rng.seed(123u);
  runMMAS();
//  int no_jobs = 10, no_machines = 5;
//  std::vector<int> initial = randomSolution(no_jobs, rng);
//  print(initial);
//  MMASParams mmas(5);
//  mmas.p0 = 0;
//  mmas.pheromones <<
//       0.0,0.0,0.5,0.0,0.0,
//       0.0,0.5,0.5,0.0,0.0,
//       0.5,0.5,0.0,0.0,0.5,
//       0.5,0.0,0.0,0.5,0.5,
//       0.0,0.0,0.0,0.5,0.0;
//  std::cout << mmas.pheromones << "\n";
//  std::vector<int> sol = mmas.constructSolution(rng);
//  test3();
  return 0;
}
*/
int main(int argc, char *argv[])
{

  return 0;
}
