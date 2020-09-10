require("irace")
require("tidyverse")
require("here")

require("metaOpt")

require("FlowshopSolveR")

print(here())
instances <- all_instances() %>%
  filter(
    no_jobs >= 30,
    inst_n == 1,
    dist != "taillard"
  )

modelling <- all_problem_data() %>%
  filter(
    stopping_criterion == "EVALS",
    budget == "med"
  )

set.seed(98798)

problems <- crossing(instances, modelling) %>%
  mutate(problem = "FSP") %>%
  mutate(problem_space = pmap(., function(...) {
    dt <- list(...)
    Problem(name = str_c(dt$model, dt$instance, sep = "_"), data = dt)
  })) %>%
  sample_n(30) %>%
  pull(problem_space)

problem_space <- ProblemSpace(problems = problems)

dataFrame2Character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
}

initFactories(here("data"))

algorithms_space <- AlgorithmSpace(
  algorithms = list(
    Algorithm(
      name = "NEH",
      parameters = readParameters(here("data", "specs", "NEH.txt")),
      solve = function(experiment, scenario) {
        problem <- scenario$targetRunnerData
        res <- solveFSP(
          mh = "NEH",
          rproblem = dataFrame2Character(problem@data),
          rparams = dataFrame2Character(experiment$configuration),
          seed = experiment$seed,
          verbose = F
        )
        cost <- res$fitness
        list(
          cost = cost
        )
      }
    )
  )
)

results <- build_performance_data(
  problemSpace = problem_space,
  algorithmSpace = algorithms_space,
  iraceScenario = defaultScenario(list(
    maxExperiments = 180
  )),
  cacheFolder = "results/performance_data/NEH"
)
