require('metaOpt')
require('irace')
require('tidyverse')
require('FlowshopSolveR')

instances <- all_instances() %>%
  filter(no_jobs >= 30,
         inst_n == 1,
         dist != 'taillard')

modelling <- all_problem_data() %>%
  filter(stopping_criterion == 'EVALS',
         budget == 'med')

problems <- crossing(instances, modelling) %>%
  mutate(problem = "FSP") %>%
  mutate(problem_space = pmap(., function(...) {
    dt <- list(...)
    Problem(name = str_c(dt$model, dt$instance, sep = '_'), data = dt)
  })) %>%
  pull(problem_space)

problem_space <- ProblemSpace(problems = problems)

dataFrame2Character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
}

algorithms_space <- AlgorithmSpace(
  algorithms = list(
    Algorithm(
      name = 'NEH',
      parameters = readParameters('data/specs/NEH.txt'),
      solve = function(experiment, scenario) {
        problem <- scenario$targetRunnerData
        res <- solveFSP(
          mh = 'NEH',
          rproblem = dataFrame2Character(problem@data),
          rparams = dataFrame2Character(experiment$configuration),
          seed = experiment$seed,
          verbose = F
        )
        list(
          cost = res$fitness
        )
      }
    )
  )
)

results <- buildPerformanceData(
  problemSpace = problem_space,
  algorithmSpace = algorithms_space,
  iraceScenario = defaultScenario(list(
    maxExperiments = 1000
  ))
)
