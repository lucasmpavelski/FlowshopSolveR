#pragma once

#include <eoEvalFunc.h>
#include <eoInt.h>
#include <eoScalarFitness.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "problems/FSPData.hpp"

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP = FSPMin;

enum class Objective { MAKESPAN, FLOWTIME };

static std::ostream& operator<<(std::ostream& o, const Objective& obj) {
  switch (obj) {
    case Objective::MAKESPAN:
      o << "MAKESPAN";
      break;
    case Objective::FLOWTIME:
      o << "FLOWTIME";
      break;
    default:
      o << "Unknown objective";
  }
  return o;
}

template <class EOT>
class FSPEvalFunc : public eoEvalFunc<EOT> {
 public:
  using Fitness = typename EOT::Fitness;
  FSPData fsp_data;
  int no_evals;
  Objective ObjT;

  FSPEvalFunc(FSPData fd, Objective ObjT = Objective::MAKESPAN)
      : fsp_data{std::move(fd)}, Ct(fsp_data.noJobs()), ObjT(ObjT) {
    no_evals = 0;
  }

  void operator()(EOT& s) {
    no_evals++;
    completionTime(s, Ct);
    double fit = 0.0, max = fsp_data.maxCT();
    switch (ObjT) {
      case Objective::MAKESPAN:
        fit = makespan(s);
        break;
      case Objective::FLOWTIME:
        fit = flowtime(s);
        max = max * max;
        break;
    }
    if (std::is_same<Fitness, eoMaximizingFitness>::value)
      s.fitness(max - fit);
    else
      s.fitness(fit);
  }

  int noJobs() const { return fsp_data.noJobs(); }
  int noMachines() const { return fsp_data.noMachines(); }
  const FSPData& getData() const { return fsp_data; }
  virtual std::string type() const { return "PERM"; };
  virtual Objective objective() const { return Objective; };

  void printOn(std::ostream& o) {
    o << "FSPEvalFunc\n"
      << "  type: " << type() << '\n'
      << "  objective: " << objective << '\n'
      << fsp_data;
  }

 protected:
  std::valarray<int> Ct;  // vector of completion times

  virtual double makespan(const EOT& s) { return Ct[s.size() - 1]; }

  virtual double flowtime(const EOT& s) {
    // total flowtime computation = sum of Ci
    auto beg = std::begin(Ct);
    auto end = std::next(beg, s.size());
    return std::accumulate(beg, end, 0);
  }

  virtual void completionTime(const EOT& _fsp, std::valarray<int>& ct) = 0;
};

template <class EOT>
class PermFSPEvalFunc : public FSPEvalFunc<EOT> {
 public:
  using FSPEvalFunc<EOT>::noJobs;
  using FSPEvalFunc<EOT>::noMachines;
  using FSPEvalFunc<EOT>::fsp_data;

  PermFSPEvalFunc(FSPData fd, Objective ObjT = Objective::MAKESPAN)
      : FSPEvalFunc<EOT>(std::move(fd), ObjT),
        part_ct(noJobs() * noMachines()) {}

  std::string type() const final override { return "PERM"; }

 protected:
  std::valarray<int> part_ct;  // matrice des comp time

  virtual void completionTime(const EOT& _fsp,
                              std::valarray<int>& Ct) override final {
    const int _N = _fsp.size();
    const int N = noJobs();
    const int M = noMachines();
    const auto& p = fsp_data.procTimesRef();
    // std::cout << _fsp << '\n';

    part_ct[0 * N + 0] = p[0 * N + _fsp[0]];
    for (int i = 1; i < _N; i++)
      part_ct[0 * N + i] = part_ct[0 * N + i - 1] + p[0 * N + _fsp[i]];
    for (int j = 1; j < M; j++)
      part_ct[j * N + 0] = part_ct[(j - 1) * N + 0] + p[j * N + _fsp[0]];
    for (int j = 1; j < M; j++) {
      for (int i = 1; i < _N; i++) {
        part_ct[j * N + i] =
            std::max(part_ct[j * N + i - 1], part_ct[(j - 1) * N + i]) +
            p[j * N + _fsp[i]];
        // std::cout << part_ct[j * N + i - 1] << " ";
      }
      // std::cout << "\n";
    }
    Ct = part_ct[std::slice((M - 1) * N, _N, 1)];
  }
};
