require('metaOpt')
require('irace')
require('FlowshopSolveR')

FlowshopSolveR::initFactories("data")

# buildPerformanceData(
#   Algorithm(
#     name = 'NEH',
#     parameters = readParameters('data/specs/NEH.txt'),
#     solve = function(experiment, scenario) {
#       FlowshopSolveR::solveFSP(
#         mh = 'NEH',
#         rproblem = as.character(scenario$problem_data),
#         rparams = as.character(experiment$configuration),
#         seed = experiment$seed,
#         verbose = F
#       )
#     }
#   )
# )