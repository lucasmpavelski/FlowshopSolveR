#pragma once

#include <paradiseo/mo/mo>
#include "flowshop-solver/heuristics/moFirstTSexplorer.hpp"

/**
 * Tabu Search
 */
template <class Neighbor>
class moFirstTS : public moLocalSearch<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;
  using Neighborhood = moNeighborhood<Neighbor>;

  /**
   * Basic constructor for a tabu search
   * @param _neighborhood the neighborhood
   * @param _fullEval the full evaluation function
   * @param _eval neighbor's evaluation function
   * @param _time the time limit for stopping criteria
   * @param _tabuListSize the size of the tabu list
   */
  moFirstTS(Neighborhood& _neighborhood,
            eoEvalFunc<EOT>& _fullEval,
            moEval<Neighbor>& _eval,
            unsigned int _time,
            unsigned int _tabuListSize)
      : moLocalSearch<Neighbor>(explorer, timeCont, _fullEval),
        timeCont(_time),
        tabuList(_tabuListSize, 0),
        explorer(_neighborhood,
                 _eval,
                 defaultNeighborComp,
                 defaultSolNeighborComp,
                 tabuList,
                 dummyIntensification,
                 dummyDiversification,
                 defaultAspiration) {}

  /**
   * Simple constructor for a tabu search
   * @param _neighborhood the neighborhood
   * @param _fullEval the full evaluation function
   * @param _eval neighbor's evaluation function
   * @param _time the time limit for stopping criteria
   * @param _tabuList the tabu list
   */
  moFirstTS(Neighborhood& _neighborhood,
            eoEvalFunc<EOT>& _fullEval,
            moEval<Neighbor>& _eval,
            unsigned int _time,
            moTabuList<Neighbor>& _tabuList)
      : moLocalSearch<Neighbor>(explorer, timeCont, _fullEval),
        timeCont(_time),
        tabuList(0, 0),
        explorer(_neighborhood,
                 _eval,
                 defaultNeighborComp,
                 defaultSolNeighborComp,
                 _tabuList,
                 dummyIntensification,
                 dummyDiversification,
                 defaultAspiration) {}

  /**
   * General constructor for a tabu search
   * @param _neighborhood the neighborhood
   * @param _fullEval the full evaluation function
   * @param _eval neighbor's evaluation function
   * @param _cont an external continuator
   * @param _tabuList the tabu list
   * @param _aspiration the aspiration Criteria
   */
  moFirstTS(Neighborhood& _neighborhood,
            eoEvalFunc<EOT>& _fullEval,
            moEval<Neighbor>& _eval,
            moContinuator<Neighbor>& _cont,
            moTabuList<Neighbor>& _tabuList,
            moAspiration<Neighbor>& _aspiration)
      : moLocalSearch<Neighbor>(explorer, _cont, _fullEval),
        timeCont(0),
        tabuList(0, 0),
        explorer(_neighborhood,
                 _eval,
                 defaultNeighborComp,
                 defaultSolNeighborComp,
                 _tabuList,
                 dummyIntensification,
                 dummyDiversification,
                 _aspiration) {}

  /**
   * General constructor for a tabu search
   * @param _neighborhood the neighborhood
   * @param _fullEval the full evaluation function
   * @param _eval neighbor's evaluation function
   * @param _neighborComp a comparator between 2 neighbors
   * @param _solNeighborComp a solution vs neighbor comparator
   * @param _cont an external continuator
   * @param _tabuList the tabu list
   * @param _intensification the intensification strategy
   * @param _diversification the diversification strategy
   * @param _aspiration the aspiration Criteria
   */
  moFirstTS(Neighborhood& _neighborhood,
            eoEvalFunc<EOT>& _fullEval,
            moEval<Neighbor>& _eval,
            moNeighborComparator<Neighbor>& _neighborComp,
            moSolNeighborComparator<Neighbor>& _solNeighborComp,
            moContinuator<Neighbor>& _cont,
            moTabuList<Neighbor>& _tabuList,
            moIntensification<Neighbor>& _intensification,
            moDiversification<Neighbor>& _diversification,
            moAspiration<Neighbor>& _aspiration)
      : moLocalSearch<Neighbor>(explorer, _cont, _fullEval),
        timeCont(0),
        tabuList(0, 0),
        explorer(_neighborhood,
                 _eval,
                 _neighborComp,
                 _solNeighborComp,
                 _tabuList,
                 _intensification,
                 _diversification,
                 _aspiration) {}

  moFirstTS(Neighborhood& _neighborhood,
            eoEvalFunc<EOT>& _fullEval,
            moEval<Neighbor>& _eval,
            moNeighborComparator<Neighbor>& _neighborComp,
            moSolNeighborComparator<Neighbor>& _solNeighborComp,
            moContinuator<Neighbor>& _cont,
            moTabuList<Neighbor>& _tabuList,
            moAspiration<Neighbor>& _aspiration)
      : moLocalSearch<Neighbor>(explorer, _cont, _fullEval),
        timeCont(0),
        tabuList(0, 0),
        explorer(_neighborhood,
                 _eval,
                 _neighborComp,
                 _solNeighborComp,
                 _tabuList,
                 dummyIntensification,
                 dummyDiversification,
                 _aspiration) {}

  /*
   * To get the explorer and then to be abble to get the best solution so far
   * @return the TS explorer
   */
  moFirstTSexplorer<Neighbor>& getExplorer() { return explorer; }

 private:
  moTimeContinuator<Neighbor> timeCont;
  moNeighborComparator<Neighbor> defaultNeighborComp;
  moSolNeighborComparator<Neighbor> defaultSolNeighborComp;
  moNeighborVectorTabuList<Neighbor> tabuList;
  moDummyIntensification<Neighbor> dummyIntensification;
  moDummyDiversification<Neighbor> dummyDiversification;
  moBestImprAspiration<Neighbor> defaultAspiration;
  moFirstTSexplorer<Neighbor> explorer;
};
