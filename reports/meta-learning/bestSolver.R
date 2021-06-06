require("irace")
require("tidyverse")
require("here")
require("metaOpt")
require("FlowshopSolveR")
require("future")
require("PMCMRplus")

set.seed(98798)

plan(multisession)
NCORES <- 6
# plan(sequential)
# NCORES <- 1

cache_path <- here('reports','bestSolverStudyData', 'cache')

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
  parallel = NCORES,
  cache = file.path(cache_path, 'tunning_result.rds')
)

get_best_configuration_irace <- function(irace_result) {
  as.list(irace::removeConfigurationsMetaData(results)[1,])
}

best_config <- get_best_configuration_irace(result)

ig_adapt_performances <- sample_performance(
  algorithm = algorithm,
  config = best_config,
  problemSpace = problem_space,
  solve_function = solve,
  no_samples = 30,
  parallel = NCORES,
  cache = file.path(cache_path, 'ig_adapt_performances.rds')
)

ig_classic_config = list(
  IG.Comp.Strat = "strict",
  IG.Neighborhood.Size = "1",
  IG.Neighborhood.Strat = "ordered",
  IG.Local.Search = "best_insertion",
  IG.LS.Single.Step = "0",
  IG.Accept = "temperature",
  IG.Perturb = "rs",
  IG.Perturb.DestructionSizeStrategy = "fixed",
  IG.Perturb.Insertion = "first_best",
  IG.Init.NEH.Priority = "sum_pij",
  IG.Init.NEH.PriorityOrder = "incr",
  IG.Init.NEH.PriorityWeighted = "0",
  IG.Init.NEH.Insertion = "first_best",
  IG.Accept.Temperature = "0.5",
  IG.Perturb.DestructionSize = "4"
)

classic_ig_performance <- sample_performance(
  algorithm = algorithm,
  config = ig_classic_config,
  problemSpace = problem_space,
  solve_function = solve,
  no_samples = 30,
  parallel = NCORES,
  cache = file.path(cache_path, 'classic_ig_performance.rds')
)


perfs <- bind_rows(
  ig_adapt_performances %>%
    mutate(algorithm_name = 'adapt_ig'),
  classic_ig_performance %>%
    mutate(algorithm_name = 'classic_ig')
) %>%
  mutate(
    problem_name = map_chr(problem, ~.x@name),
    objective = map_chr(problem, ~.x@data[[1]]['objective']),
    perf = map_dbl(result, ~.x$cost)
  ) %>%
  group_by(problem_name, objective) %>%
  mutate(
    relative_perf = 100 * (perf - min(perf)) / min(perf)
  ) %>%
  group_by(algorithm_name, problem_name, objective) %>%
  summarise(
    avg_relative_perf = mean(relative_perf)
  )

ggplot(perfs_summary) +
  facet_wrap(~objective) +
  geom_violin(aes(x = paste0(algorithm_name), y = avg_relative_perf))

perfs_summary %>%
  group_by(objective) %>%
  nest() %>%
  mutate(is_normal = map_lgl(data, ~shapiro.test(.x$avg_relative_perf)$p.value > 0.05)) %>%
  select(-data)

perfs_summary %>%
  group_by(objective) %>%
  nest() %>%
  mutate(is_different = map_lgl(data, 
                                ~frdAllPairsConoverTest(.x$avg_relative_perf,
                                                        .x$algorithm_name, 
                                                        .x$problem_name)$p.value < 0.05)) %>%
  select(-data)

