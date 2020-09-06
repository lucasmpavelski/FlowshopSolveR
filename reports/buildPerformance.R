require('metaOpt')
require('irace')
require('RFlowshopSolvers')

buildPerformanceData(
  Algorithm(
    name = 'NEH',
    parameters = readParameters('data/specs/NEH.txt'),
    solve = function(experiment, scenario) {
      RFlowshopSolvers::solveFSP(
        mh = 'NEH',
        rproblem = as.character(scenario$problem_data),
        rparams = as.character(experiment$configuration),
        seed = experiment$seed, 
        verbose = F 
      )
    }
  )
)