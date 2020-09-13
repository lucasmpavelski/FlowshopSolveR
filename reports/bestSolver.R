require("irace")
require("tidyverse")
require("here")
require("metaOpt")
require("FlowshopSolveR")
require("future")

plan(multisession)
NCORES <- 6
# plan(sequential)
# NCORES <- 1

dataFrame2Character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
}

instances <- all_instances(here("data", "instances", "flowshop")) %>%
  filter(
    no_jobs >= 50,
    no_jobs <= 100,
    no_machines <= 20,
    no_machines >= 10,
    inst_n <= 5,
    dist == "taill-like",
    corr == 'rand'
  )

modelling <- fixed_time(15) %>%
  filter(type == 'PERM')

set.seed(98798)

problems <- crossing(instances, modelling) %>%
  mutate(problem = "FSP") %>%
  mutate(problem_space = pmap(., function(...) {
    dt <- list(...)
    Problem(name = str_c(dt$model, dt$instance, sep = "_"), data = list(dataFrame2Character(dt)))
  })) %>%
  pull(problem_space)

problem_space <- ProblemSpace(problems = problems)

algorithm <- Algorithm(
    name = "IG",
    parameters = readParameters(here("data", "specs", "ADAPT-IG.txt"))
  )


solve <- function(algorithm, config, problem, seed, ...) {
  initFactories(here("data"))
  timestamp()
  cat(unlist(problem@data))
  res <- solveFSP(
    mh = algorithm@name,
    rproblem = unlist(problem@data),
    rparams = dataFrame2Character(config),
    seed = seed,
    verbose = F
  )
  timestamp()
  list(
    cost = res$fitness,
    time = res$time
  )
}

results <- train_best_solver(
  problem_space = problem_space,
  algorithm = algorithm,
  solve_function = solve,
  irace_scenario = defaultScenario(list(
    maxExperiments = 5000
  )),
  parallel = NCORES
)

save(results, 'runs/irace-result-taill-like-ADAPT-IG.Rdata')
