#ifndef _moFirstBestTSexplorer_h
#define _moFirstBestTSexplorer_h

/**
 * Explorer for a Tabu Search
 */
template <class Neighbor>
class moFirstBestTSexplorer : public moNeighborhoodExplorer<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;
  using Neighborhood = moNeighborhood<Neighbor>;

  using moNeighborhoodExplorer<Neighbor>::currentNeighbor;
  using moNeighborhoodExplorer<Neighbor>::selectedNeighbor;

  /**
   * Constructor
   * @param _neighborhood the neighborhood
   * @param _eval the evaluation function
   * @param _neighborComparator a neighbor comparator
   * @param _solNeighborComparator a comparator between a solution and a
   * neighbor
   * @param _tabuList the tabu list
   * @param _intensification the intensification box
   * @param _diversification the diversification box
   * @param _aspiration the aspiration criteria
   */
  moFirstBestTSexplorer(
      Neighborhood& _neighborhood,
      moEval<Neighbor>& _eval,
      moNeighborComparator<Neighbor>& _neighborComparator,
      moSolNeighborComparator<Neighbor>& _solNeighborComparator,
      moTabuList<Neighbor>& _tabuList,
      moIntensification<Neighbor>& _intensification,
      moDiversification<Neighbor>& _diversification,
      moAspiration<Neighbor>& _aspiration)
      : moNeighborhoodExplorer<Neighbor>(_neighborhood, _eval),
        neighborComparator(_neighborComparator),
        solNeighborComparator(_solNeighborComparator),
        tabuList(_tabuList),
        intensification(_intensification),
        diversification(_diversification),
        aspiration(_aspiration) {
    isAccept = false;
  }

  /**
   * Destructor
   */
  virtual ~moFirstBestTSexplorer() {}

  /**
   * init tabu list, intensification box, diversification box and aspiration
   * criteria
   * @param _solution
   */
  virtual void initParam(EOT& _solution) {
    tabuList.init(_solution);
    intensification.init(_solution);
    diversification.init(_solution);
    aspiration.init(_solution);
    bestSoFar = _solution;
    first = true;  // to check the first descent of TS
  };

  /**
   * update params of tabu list, intensification box, diversification box and
   * aspiration criteria
   * @param _solution
   */
  virtual void updateParam(EOT& _solution) {
    if ((*this).moveApplied()) {
      tabuList.add(_solution, selectedNeighbor);
      intensification.add(_solution, selectedNeighbor);
      diversification.add(_solution, selectedNeighbor);
      if (_solution.fitness() > bestSoFar.fitness())
        bestSoFar = _solution;
    }
    tabuList.update(_solution, selectedNeighbor);
    intensification.update(_solution, selectedNeighbor);
    diversification.update(_solution, selectedNeighbor);
    aspiration.update(_solution, selectedNeighbor);
  };

  /**
   * terminate : _solution becomes the best so far
   */
  virtual void terminate(EOT& _solution) { _solution = bestSoFar; };

  /**
   * Explore the neighborhood of a solution
   * @param _solution
   */
  virtual void operator()(EOT& _solution) {
    bool found = false;
    intensification(_solution);
    diversification(_solution);

    if (neighborhood.hasNeighbor(_solution)) {
      if (first) {  // first improvement for the first "descent" to the first LO
                    // code of moFirstTSexplorer

        // init the current neighbor
        neighborhood.init(_solution, currentNeighbor);
        // eval the current neighbor
        eval(_solution, currentNeighbor);

        // Find the first non-tabu element
        if ((!tabuList.check(_solution, currentNeighbor)) ||
            aspiration(_solution, currentNeighbor)) {
          // set selectedNeighbor
          selectedNeighbor = currentNeighbor;
          found = true;
        }
        while (neighborhood.cont(_solution) && !found) {
          // next neighbor
          neighborhood.next(_solution, currentNeighbor);
          // eval
          eval(_solution, currentNeighbor);

          if ((!tabuList.check(_solution, currentNeighbor)) ||
              aspiration(_solution, currentNeighbor)) {
            // set selectedNeighbor
            selectedNeighbor = currentNeighbor;
            found = true;
          }
        }
        // Explore the neighborhood
        if (found) {
          isAccept = true;
          bool improveSol = false;
          if (solNeighborComparator(_solution, currentNeighbor)) {
            improveSol = true;
          }
          while (neighborhood.cont(_solution) && !improveSol) {
            // next neighbor
            neighborhood.next(_solution, currentNeighbor);
            // eval
            eval(_solution, currentNeighbor);

            // check if the current is better than the best and is not tabu or
            // if it is aspirat (by the aspiration criteria of course)
            if ((!tabuList.check(_solution, currentNeighbor) ||
                 aspiration(_solution, currentNeighbor)) &&
                neighborComparator(selectedNeighbor, currentNeighbor)) {
              // set selectedNeighbor
              selectedNeighbor = currentNeighbor;
              if (solNeighborComparator(_solution, currentNeighbor))
                improveSol = true;
            }
          }
          if (!improveSol)  // no improving neighbor -> _solution=LO -> first
                            // improvement=desactivated / full exploration =
                            // activated
            first = false;
        } else {
          isAccept = false;
        }
      } else {  // full exploration of the neighborhood
                // code of moTSexplorer
        // init the current neighbor
        neighborhood.init(_solution, currentNeighbor);
        // eval the current neighbor
        eval(_solution, currentNeighbor);

        // Find the first non-tabu element
        if ((!tabuList.check(_solution, currentNeighbor)) ||
            aspiration(_solution, currentNeighbor)) {
          // set selectedNeighbor
          selectedNeighbor = currentNeighbor;
          found = true;
        }
        while (neighborhood.cont(_solution) && !found) {
          // next neighbor
          neighborhood.next(_solution, currentNeighbor);
          // eval
          eval(_solution, currentNeighbor);

          if ((!tabuList.check(_solution, currentNeighbor)) ||
              aspiration(_solution, currentNeighbor)) {
            // set selectedNeighbor
            selectedNeighbor = currentNeighbor;
            found = true;
          }
        }
        // Explore the neighborhood
        if (found) {
          isAccept = true;
          while (neighborhood.cont(_solution)) {
            // next neighbor
            neighborhood.next(_solution, currentNeighbor);
            // eval
            eval(_solution, currentNeighbor);
            // check if the current is better than the best and is not tabu or
            // if it is aspirat (by the aspiration criteria of course)
            if ((!tabuList.check(_solution, currentNeighbor) ||
                 aspiration(_solution, currentNeighbor)) &&
                neighborComparator(selectedNeighbor, currentNeighbor)) {
              // set selectedNeighbor
              selectedNeighbor = currentNeighbor;
            }
          }
        } else {
          isAccept = false;
        }
      }
    } else {
      isAccept = false;
    }
  };

  /**
   * always continue
   * @param _solution the solution
   * @return true
   */
  virtual bool isContinue(EOT&) { return true; };

  /**
   * accept test if an ameliorated neighbor was found
   * @param _solution the solution
   * @return true if the best neighbor ameliorate the fitness
   */
  virtual bool accept(EOT&) { return isAccept; };

  /**
   * Give the current best found so far
   * @return the best solution so far
   */
  const EOT& getBest() { return bestSoFar; };

 protected:
  using moNeighborhoodExplorer<Neighbor>::neighborhood;
  using moNeighborhoodExplorer<Neighbor>::eval;

  // comparator between solution and neighbor or between neighbors
  moNeighborComparator<Neighbor>& neighborComparator;
  moSolNeighborComparator<Neighbor>& solNeighborComparator;

  // Tabu components
  moTabuList<Neighbor>& tabuList;
  moIntensification<Neighbor>& intensification;
  moDiversification<Neighbor>& diversification;
  moAspiration<Neighbor>& aspiration;

  // Best so far Solution
  EOT bestSoFar;

  // true if the move is accepted
  bool isAccept;
  bool first;
};

#endif
