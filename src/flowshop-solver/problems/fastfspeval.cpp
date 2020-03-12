#include "fastfspeval.hpp"


std::string FastFSPSolution::className() const {
  return "FastFSPSolution";
};

FSP::Fitness FastFSPSolution::neighborFitness(const FSPData& fspData, unsigned first, unsigned second, Objective objective) {
  if (compiledSolution.size() == 0) {
    compiledSchedules.assign(size(), CompiledSchedule(fspData.noJobs(), fspData.noMachines()));
    isCompiled.assign(size(), 0);
  }
  if (!std::equal(compiledSolution.begin(), compiledSolution.end(), begin(), end())) {
    compiledSolution = *this;
    isCompiled.assign(size(), 0);
  }
  if (!isCompiled[first]) {
    ivec perm_i = compiledSolution;
    std::rotate(perm_i.begin() + first,
                perm_i.begin() + first + 1,
                perm_i.end());
    compiledSchedules[first].compile(fspData, perm_i);
    isCompiled[first] = 1;
  }
  if (first < second)
      second--;
  switch (objective) {
  case Objective::MAKESPAN:
    return compiledSchedules[first].getMakespan(second);
  case Objective::FLOWTIME:
    return compiledSchedules[first].getFlowtime(second);
  }
}
