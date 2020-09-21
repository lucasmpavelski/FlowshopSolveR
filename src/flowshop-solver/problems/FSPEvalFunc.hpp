#pragma once

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

#include <paradiseo/eo/eo>

#include "flowshop-solver/problems/FSPData.hpp"

enum class Objective { MAKESPAN, FLOWTIME };

[[maybe_unused]] static auto operator<<(std::ostream& o, const Objective& obj)
    -> std::ostream& {
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
  std::vector<int> Ct;  // vector of completion times
  Objective ObjT;

  FSPEvalFunc(FSPData fd, Objective ObjT = Objective::MAKESPAN)
      : fsp_data{std::move(fd)}, Ct(fsp_data.noJobs()), ObjT(ObjT) {}

  void operator()(EOT& s) override {
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

  [[nodiscard]] auto noJobs() const -> int { return fsp_data.noJobs(); }
  [[nodiscard]] auto noMachines() const -> int { return fsp_data.noMachines(); }
  [[nodiscard]] auto getData() const -> const FSPData& { return fsp_data; }
  [[nodiscard]] virtual auto type() const -> std::string { return "PERM"; };
  [[nodiscard]] auto objective() const -> Objective { return ObjT; };

  void printOn(std::ostream& o) {
    o << "FSPEvalFunc\n"
      << "  type: " << type() << '\n'
      << "  objective: " << objective() << '\n'
      << fsp_data;
  }

 protected:
  virtual auto makespan(const EOT& s) -> double { return Ct[s.size() - 1]; }

  virtual auto flowtime(const EOT& s) -> double {
    // total flowtime computation = sum of Ci
    auto beg = std::begin(Ct);
    auto end = std::next(beg, s.size());
    return std::accumulate(beg, end, 0);
  }

  virtual void completionTime(const EOT& _fsp, std::vector<int>& ct) = 0;
};

template <class EOT>
class PermFSPEvalFunc : public FSPEvalFunc<EOT> {
 public:
  using FSPEvalFunc<EOT>::noJobs;
  using FSPEvalFunc<EOT>::noMachines;
  using FSPEvalFunc<EOT>::fsp_data;

  PermFSPEvalFunc(const FSPData& fd, Objective ObjT = Objective::MAKESPAN)
      : FSPEvalFunc<EOT>(fd, ObjT), part_ct(noJobs() * noMachines()) {}

  [[nodiscard]] auto type() const -> std::string final { return "PERM"; }

 protected:
  std::vector<int> part_ct;

  void completionTime(const EOT& _fsp, std::vector<int>& Ct) override {
    const int _N = _fsp.size();
    const int N = noJobs();
    const int M = noMachines();
    const auto& p = fsp_data.procTimesRef();
    // std::cout << _fsp << '\n';


    /** extra **/
    // int idle = 0;
    // std::cout << "idle ";

    // int wait = 0;
    // std::cout << "wait ";
    /** extra **/

    part_ct[0 * N + 0] = p[0 * N + _fsp[0]];
    for (int i = 1; i < _N; i++) {
      part_ct[0 * N + i] = part_ct[0 * N + i - 1] + p[0 * N + _fsp[i]];
    }

    for (int j = 1; j < M; j++) {
      part_ct[j * N + 0] = part_ct[(j - 1) * N + 0] + p[j * N + _fsp[0]];
    }

    for (int j = 1; j < M; j++) {
      for (int i = 1; i < _N; i++) {
        part_ct[j * N + i] =
            std::max(part_ct[j * N + i - 1], part_ct[(j - 1) * N + i]) +
            p[j * N + _fsp[i]];

        /** extra **/
        // idle += std::max(part_ct[(j - 1) * N + i] - part_ct[j * N + i - 1], 0);
        // std::cout << '(' << i << ',' << j << ')' << std::max(part_ct[(j - 1) * N + i] - part_ct[j * N + i - 1], 0) << ' ';
        /** extra **/
            
        /** extra **/
        // wait += std::max(part_ct[j * N + i - 1] - part_ct[(j - 1) * N + i] , 0);
        // std::cout << '(' << i << ',' << j << ')' << std::max(part_ct[(j - 1) * N + i] - part_ct[j * N + i - 1], 0) << ' ';
        /** extra **/
      }
    }

    /** extra **/
    // std::cout << "final idle " << idle << '\n';
    // std::cout << "final wait " << wait << '\n';
    /** extra **/

    auto from = part_ct.begin() + (M - 1) * N;
    auto to = from + _N;
    Ct.assign(from, to);
  }
};
