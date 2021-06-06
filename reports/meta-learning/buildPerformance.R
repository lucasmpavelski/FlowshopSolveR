require("irace")
require("tidyverse")
require("here")
require("metaOpt")
require("FlowshopSolveR")
require("future")


plan("sequential")

dataFrame2Character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
}

instances <- all_instances(here("data", "instances", "flowshop")) %>%
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
    Problem(name = str_c(dt$model, dt$instance, sep = "_"), data = list(dataFrame2Character(dt)))
  })) %>%
  sample_n(30) %>%
  pull(problem_space)

problem_space <- ProblemSpace(problems = problems)

algorithms_space <- AlgorithmSpace(
  algorithms = list(
    Algorithm(
      name = "NEH",
      parameters = readParameters(here("data", "specs", "NEH.txt"))
    )
  )
)

initFactories(here("data"))

solve <- function(algorithm, config, problem, seed, ...) {
  res <- solveFSP(
    mh = algorithm@name,
    rproblem = unlist(problem@data),
    rparams = dataFrame2Character(config),
    seed = seed,
    verbose = F
  )
  list(
    cost = res$fitness,
    time = res$time
  )
}

results <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithms_space,
  solve_function = solve,
  irace_scenario = defaultScenario(list(
    maxExperiments = 180
  )),
  parallel = 2#,
  # cache_folder = here('performance_data', 'NEH')
)
