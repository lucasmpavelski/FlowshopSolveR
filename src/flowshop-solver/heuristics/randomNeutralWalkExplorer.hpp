#pragma once

#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

/**
 * Restart Perturbation : restart when maximum number of iteration with no
 * improvement is reached
 */
template <class Neighbor, class EOT = typename Neighbor::EOT>
class randomNeutralWalkExplorer : public moNeighborhoodExplorer<Neighbor> {
  eoEvalFunc<EOT>& fullEval;
  moSolNeighborComparator<Neighbor>& solNeighborComparator;
  // maximum number of steps to do
  unsigned int nbStep;
  eoMonOp<EOT>& restart;

  // the first neutral solution found in the neighborhood
  Neighbor neutralNgh;
  Neighbor improvingNgh;
  // Pointer on the current neighbor
  Neighbor* current;

  // true if the move is accepted
  bool isAccept;
  bool firstNeutralNghFound;

 public:
  using Neighborhood = moNeighborhood<Neighbor>;
  using moNeighborhoodExplorer<Neighbor>::neighborhood;
  using moNeighborhoodExplorer<Neighbor>::eval;

  /**
   * Constructor
   * @param _neighborhood the neighborhood
   * @param _eval the evaluation function of the neighborhood
   * @param _eval the full evaluation function
   * @param _solNeighborComparator solution vs neighbor comparator
   * @param _nbStep the length of the allowed neutral walk
   * @param _restart the method used to restart the solution when we are blocked
   */
  randomNeutralWalkExplorer(
      Neighborhood& _neighborhood,
      moEval<Neighbor>& _eval,
      eoEvalFunc<EOT>& _fullEval,
      moSolNeighborComparator<Neighbor>& _solNeighborComparator,
      unsigned _nbStep,
      eoMonOp<EOT>& _restart)
      : moNeighborhoodExplorer<Neighbor>(_neighborhood, _eval),
        fullEval(_fullEval),
        solNeighborComparator(_solNeighborComparator),
        nbStep(_nbStep),
        restart(_restart) {
    isAccept = false;
    current = new Neighbor();
    step = 0;
    imprNghFound = false;
    firstNeutralNghFound = false;
  }

  /**
   * Destructor
   */
  ~randomNeutralWalkExplorer() { delete current; }

  /**
   * empty the vector of best solutions
   * @param _solution unused solution
   */
  virtual void initParam(EOT&) {
    step = 0;
    isAccept = true;
    imprNghFound = false;
    firstNeutralNghFound = false;
  };

  /**
   * empty the vector of best solutions
   * @param _solution unused solution
   */
  virtual void updateParam(EOT&) { step++; };

  /**
   * terminate: NOTHING TO DO
   * @param _solution unused solution
   */
  virtual void terminate(EOT& _solution) {
    if (!imprNghFound) {
      restart(_solution);
      _solution.invalidate();
      fullEval(_solution);
    }  // else
       // std::cout << step << std::endl;
    //        std::cout << "dans perturb terminate" << _solution << std::endl;
  };

  /**
   * Explore the neighborhood of a solution
   * @param _solution the current solution
   */
  virtual void operator()(EOT& _solution) {
    //        std::cout << "dans perturb()" << _solution << std::endl;
    // Test if _solution has a Neighbor
    if (neighborhood.hasNeighbor(_solution)) {
      // init the first neighbor
      neighborhood.init(_solution, (*current));

      // eval the _solution moved with the neighbor and save the result in the
      // neighbor
      eval(_solution, (*current));

      firstNeutralNghFound = false;
      imprNghFound = false;
      isAccept = false;

      if (solNeighborComparator(_solution, *current)) {
        imprNghFound = true;
        isAccept = true;
        improvingNgh = *current;
      } else if (!firstNeutralNghFound &&
                 solNeighborComparator.equals(_solution, *current)) {
        firstNeutralNghFound = true;
        neutralNgh = *current;
        isAccept = true;
      }

      // evaluation of neighborhood
      while (neighborhood.cont(_solution) && !imprNghFound &&
             !firstNeutralNghFound) {
        // next neighbor
        neighborhood.next(_solution, (*current));

        // eval
        eval(_solution, (*current));

        // if we found a portal
        if (solNeighborComparator(_solution, *current)) {
          imprNghFound = true;
          isAccept = true;
          improvingNgh = *current;
        } else if (!firstNeutralNghFound &&
                   solNeighborComparator.equals(_solution, *current)) {
          firstNeutralNghFound = true;
          neutralNgh = *current;
          isAccept = true;
        }
      }
    } else {
      // if _solution hasn't neighbor,
      isAccept = false;
    }
  };

  /**
   * continue if a move is accepted
   * @param _solution the solution
   * @return true if an ameliorated neighbor was be found
   */
  virtual bool isContinue(EOT&) {
    return (step < nbStep) && isAccept && !imprNghFound;
  };

  /**
   * move the solution with the best neighbor
   * @param _solution the solution to move
   */
  virtual void move(EOT& _solution) {
    if (firstNeutralNghFound) {
      // move the solution
      neutralNgh.move(_solution);

      // update its fitness
      _solution.fitness(neutralNgh.fitness());
    } else if (imprNghFound) {
      // move the solution
      improvingNgh.move(_solution);

      // update its fitness
      _solution.fitness(improvingNgh.fitness());
    }
  };

  /**
   * accept test if a neutral neighbor was found
   * @param _solution the solution
   * @return true if the a neutral neighbor is found
   */
  virtual bool accept(EOT&) { return isAccept; };

  // current number of step
  unsigned int step;

  // test if an improving neighbor is found
  bool imprNghFound;
};
