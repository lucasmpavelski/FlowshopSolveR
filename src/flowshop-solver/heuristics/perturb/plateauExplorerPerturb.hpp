#pragma once

#include <memory/moCountMoveMemory.h>
#include <utils/eoRNG.h>
#include <cassert>
#include <utility>
#include <vector>
#include "perturb/moPerturbation.h"

/**
 * Plateau Perturbation
 */
template <class Neighbor>
class plateauExplorerPerturb : public moPerturbation<Neighbor>,
                               public moCountMoveMemory<Neighbor> {
 public:
  using EOT = typename Neighbor::EOT;
  using Fitness = typename EOT::Fitness;
  using pair = typename std::pair<int, int>;

  plateauExplorerPerturb(fspEval<EOT>& _fullEval,
                         unsigned _MNS,
                         eoMonOp<EOT>& _restart)
      : eval(_fullEval), MNS(_MNS), restart(_restart) {
    M = eval.getM();
    N = eval.getN();
    p = eval.getP();

    RandJOB.resize(0);
    for (int i = 0; i < N; ++i) {
      RandJOB.push_back(i);
    }

    MNSvalue = _MNS;
  }

  /**
   * Apply restart when necessary
   * @param _solution to move
   * @return true
   */
  bool operator()(EOT& _solution) {
    //		if( (int) _solution.fitness()<= 3904)
    //			MNS=20000;
    //		else
    //			MNS=MNSvalue;
    //
    //		cout << _solution << "*******" << MNS << endl;

    step = 0;

    evalNbh(_solution);  // NB: _solution is an LO

    if (nghNeutr_pos.size() == 0) {  // LO type 1
      restart(_solution);
      eval(_solution);
    } else {
      int k = rng.random(nghNeutr_pos.size());
      move2(_solution, nghNeutr_pos[k].first, nghNeutr_pos[k].second,
            nghNeutr_fit[k]);
      step++;

      while ((step < MNS) && !improvingNgh) {
        evalNbh(_solution);

        //				cout << nghNeutr_pos.size() << endl;

        if (improvingNgh)
          _solution = imprSol;
        else {
          step++;
          if (nghNeutr_pos.size() > 0) {
            k = rng.random(nghNeutr_pos.size());
            move2(_solution, nghNeutr_pos[k].first, nghNeutr_pos[k].second,
                  nghNeutr_fit[k]);
          } else
            step = MNS;
        }
      }

      if (step == MNS) {
        restart(_solution);
        eval(_solution);
      }
    }
  }

  bool evalNbh(EOT& _solution) {
    std::random_shuffle(RandJOB.begin(), RandJOB.end(), rndGen);
    nghNeutr_pos.resize(0);
    nghNeutr_fit.resize(0);
    improvingNgh = false;
    found = false;

    int pos, job;

    for (int k = 0; k < RandJOB.size() && !found; ++k) {
      pos = 0;
      job = RandJOB[k];

      while (_solution[pos] != job) {
        pos++;
      }

      solTMP = _solution;
      solTMP.erase(solTMP.begin() + pos);

      std::vector<std::vector<int> > e(N + 1), q(N + 1), f(N + 1);
      for (unsigned int i = 0; i < N + 1; i++) {
        e[i].assign(M + 2, 0);
        q[i].assign(M + 2, 0);
        f[i].assign(M + 2, 0);
      }

      for (int j = 1; j < M + 1; ++j) {
        for (int i = 1; i < N; ++i) {
          e[i][j] =
              std::max(e[i][j - 1], e[i - 1][j]) + p[j - 1][solTMP[i - 1]];
          q[N - i][M + 1 - j] =
              std::max(q[N - i][M + 1 - j + 1], q[N - i + 1][M + 1 - j]) +
              p[M + 1 - j - 1][solTMP[N - i - 1]];
          f[i][j] = std::max(f[i][j - 1], e[i - 1][j]) + p[j - 1][job];
        }
        f[N][j] = std::max(f[N][j - 1], e[N - 1][j]) + p[j - 1][job];
      }

      int fit;

      for (int i = 1; i < N + 1; ++i) {
        fit = -1;
        for (int j = 1; j < M + 1; ++j) {
          if (f[i][j] + q[i][j] > fit)
            fit = f[i][j] + q[i][j];
        }

        if (fit < _solution.fitness()) {
          if (!improvingNgh || fit < imprSol.fitness()) {
            improvingNgh = true;
            imprSol = solTMP;
            move1(imprSol, job, i - 1, fit);
            found = true;
          }
        } else if (fit == _solution.fitness() && pos != (i - 1)) {
          nghNeutr_pos.push_back(std::make_pair(pos, i - 1));
          nghNeutr_fit.push_back(fit);
          found = true;
        }
      }
    }
  }

  void move1(EOT& _sol, int job, int pos, int fit) {
    _sol.insert(_sol.begin() + pos, job);
    _sol.fitness(fit);
  }

  void move2(EOT& _sol, int first, int second, int fit) {
    int job = _sol[first];
    _sol.erase(_sol.begin() + first);
    _sol.insert(_sol.begin() + second, job);
    _sol.fitness(fit);
  }

 private:
  fspEval<EOT>& eval;
  eoMonOp<EOT>& restart;
  unsigned int MNS;
  unsigned int M;
  unsigned int N;
  std::vector<std::vector<unsigned int> > p;

  EOT solTMP;

  bool improvingNgh;
  EOT imprSol;

  std::vector<pair>
      nghNeutr_pos;  // first=position de départ; second=position d'arriver;
  std::vector<Fitness> nghNeutr_fit;

  std::vector<int> RandJOB;
  UF_random_generator<unsigned int> rndGen;

  bool found;

  unsigned int step;

  unsigned int MNSvalue;
};
