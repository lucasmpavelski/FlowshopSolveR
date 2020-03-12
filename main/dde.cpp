#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <fstream>

#include <eo>
#include <mo>
#include <es/eoRealInitBounded.h>
#include <es/eoRealOp.h>

#include "flowshop-solver/MHParamsSpecs.h"
#include "flowshop-solver/MHParamsValues.h"
#include "flowshop-solver/MHSolve.h"

//template<class RNG>
//void destruction(RNG& rng, std::vector<int> sol, int d) {
//  std::uniform_int_distribution<int> dist(0, sol.size() - 1);
//  std::vector<int> random_jobs;
//  random_jobs.reserve(d);
//  int i = 0;
//  while (i < d) {
//    int r = dist(rng);
//    if (std::find(begin(random_jobs), end(random_jobs), r) == random_jobs.end()) {
//      random_jobs.emplace_back(r);
//      i++;
//    }
//  }
//  std::partition(begin(sol), end(sol), [&random_jobs](int a) {
//    return std::find(begin(random_jobs), end(random_jobs), a) == random_jobs.end();
//  });
//}
//using std::cout;
//std::vector<int> individual(25);
//std::iota(begin(individual), end(individual), 0);
//std::random_device real_rng;
//std::mt19937_64 rng;
//rng.seed(real_rng());
//destruction(rng, individual, 5);

struct DECrossoverOperator {
  float F{0.5}, CR{0.9};
  template <class Ind>
  Ind operator()(eoPop<Ind>& pop, int idx) {
    const int n = pop.size();
    int a = 0, b = 0, c = 0;
    a = rng.random(n);
    do {
      b = rng.random(n);
    } while (b == a);
    do {
      c = rng.random(n);
    } while (c == a || c == b);
    Ind mutated(pop[idx]);
    for (int i = 0; i < mutated.size(); i++) {
      if (rng.uniform() < CR)
        mutated[i] = pop[a][i] + F * (pop[b][i] - pop[c][i]);
      else
        mutated[i] = pop[idx][i];
    }
    return mutated;
  }
};

template <class Ngh, class EOT = typename Ngh::EOT>
struct EvalFunction : public eoEvalFunc<MHParamsValues> {
  Problem<Ngh> &prob;
  const EOT &order;
  int no_samples{5};

  EvalFunction(Problem<Ngh> &prob, const EOT &order)
    : prob{prob}, order{order} {}

  void operator()(MHParamsValues& sol) {
    double res = evaluateMean(sol, prob, order, no_samples);
    sol.fitness(res);
  }
};

struct DE {
  int no_gen{100};
  DECrossoverOperator op;
  std::ostream& log{std::cout};

  template<class Sol>
  void operator()(eoPop<Sol>& pop, eoEvalFunc<Sol>& eval, eoRealVectorBounds& bounds) {
    int n = pop.size();
    eoPop<Sol> mut_pop(pop);
    eoUniformMutation<Sol> mutation(bounds, 20, 1.0 / n);
    log << "gen max 25 50 75 min\n0 "
        << pop.nth_element_fitness(0) << ' '
        << pop.nth_element_fitness(n / 4) << ' '
        << pop.nth_element_fitness(n / 2) << ' '
        << pop.nth_element_fitness(3 * n / 4) << ' '
        << pop.nth_element_fitness(n - 1) << ' '
        << '\n';;
    for (int gen = 0; gen < no_gen; ++gen) {
      for (int j = 0; j < n; ++j) {
        Sol de_mut = op(pop, j);
        mutation(de_mut);
        for (int i = 0; i < de_mut.size(); i++)
          bounds.truncate(i, de_mut[i]);
        eval(de_mut);
        mut_pop[j] = de_mut;
      }
      for (int j = 0; j < n; ++j) {
        if (pop[j].fitness() > mut_pop[j].fitness()) {
          pop[j] = mut_pop[j];
        }
      }
      log << gen + 1 << ' '
          << pop.nth_element_fitness(0) << ' '
          << pop.nth_element_fitness(n / 4) << ' '
          << pop.nth_element_fitness(n / 2) << ' '
          << pop.nth_element_fitness(3 * n / 4) << ' '
          << pop.nth_element_fitness(n - 1) << ' '
          << '\n';
    }
  }
};

int main(int argc, char *argv[]) {
  const int pop_size = 10;
  std::string instance = argc > 1 ? argv[1] : "taill-like_20_10_2010101.gen";
  std::string result_fn = argc > 2 ? argv[2] : "final.dat";

  const char mh_name[] = "ILS";

  std::random_device true_rng;
  rng.reseed(true_rng());
  DECrossoverOperator de_xover;

  initFactories("/home/lucasmp/projects/git/evolutionary_tunners/data/instances/generated_intances/generated_instances_all/",
                "/home/lucasmp/projects/git/evolutionary_tunners/data/specs/", true);

  MHParamsSpecs mh_specs = MHParamsSpecsFactory::get(mh_name);

  std::unordered_map<std::string, std::string> prob_data;
  prob_data["problem"] = "FSP";
  prob_data["type"] = "PERM";
  prob_data["objective"] = "MAKESPAN";
  prob_data["budget"] = "low";
  prob_data["instance"] = instance;

  FSPProblem problem = FSPProblemFactory::get(prob_data);
  FSP order = totalProcTimes(problem.getData());

  EvalFunction<FSPProblem::Ngh> eval_func(problem, order);

  using Sol = MHParamsValues;

  eoRealVectorBounds bounds = mh_specs.getBounds();
  eoRealInitBounded<Sol> init(bounds);
  eoUniformMutation<Sol> mutation(bounds, 20, 1.0 / mh_specs.noParams());

  eoPop<Sol> pop;
  for (int i = 0; i < pop_size; i++) {
    Sol sol(&mh_specs);
    init(sol);
    eval_func(sol);
    pop.emplace_back(sol);
  }

  //std::cout << pop << '\n';
  DE de_algo;
  de_algo.no_gen = 0;
  de_algo.op = de_xover;
  de_algo(pop, eval_func, bounds);


  std::fstream result_file(result_fn, std::fstream::out);

  std::ostream& result = result_file;

  result << pop << '\n';

//  std::cout << bounds << '\n';
//  Sol de_mut = de_xover(pop, 0);
//  std::cout << de_mut << '\n';
//  mutation(de_mut);
//  std::cout << de_mut << '\n';
//  for (int i = 0; i < de_mut.size(); i++)
//    bounds.truncate(i, de_mut[i]);
//  std::cout << de_mut << '\n';
//  eval_func(de_mut);
//  std::cout << de_mut << '\n';

  return 0;
}
