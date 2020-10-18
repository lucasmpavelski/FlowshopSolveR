library(irace)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(metaOpt)


aclib2_problems <- function(path) {
  folders <- str_split(path, '/', simplify = T)
  no_folders <- length(folders)
  scenario <- folders[,no_folders-1]
  tunner <- 'IRACE3'
  
  scenario_dt <- str_split(scenario, '_', 2, simplify = T)
  algo <- scenario_dt[,1]
  scenario_problem <- scenario_dt[,2]
  
  train_instances_path <- here('aclib2', 'fsp', 'instances', 'fsp', 'sets', scenario_problem, 'training.txt')
  test_instances_path <- here('aclib2', 'fsp', 'instances', 'fsp', 'sets', scenario_problem, 'test.txt')
  
  train_instances <- read_csv(train_instances_path, col_names = F) %>% pull(X6) %>% as.list()
  test_instances <- read_csv(test_instances_path, col_names = F) %>% pull(X6) %>% as.list()
  
  problem_dt <- str_split(scenario_dt[,2], ',', simplify = T)
  problem <- problem_dt[,1]
  objective <- problem_dt[,2]
  type <- problem_dt[,3]
  stopping_criterion <- problem_dt[,4]
  budget <- problem_dt[,5]
  dist <- problem_dt[,6]
  corr <- problem_dt[,7]
  no_jobs <- problem_dt[,8]
  no_machines <- problem_dt[,9]
  
  problem_dt <- list(
    scenario = scenario,
    tunner = tunner,
    algo = algo,
    scenario_problem = scenario_problem,
    problem = problem,
    objective = objective,
    type = type,
    stopping_criterion = stopping_criterion,
    budget = budget,
    dist = dist,
    corr = corr,
    no_jobs = no_jobs,
    no_machines = no_machines,
    path = path
  )
  
  problem_obj <- Problem(
    name = scenario_problem,
    instances = train_instances,
    data = problem_dt
  )
  
  problem_dt$problem_obj <- list(problem_obj)
  
  problem_dt
}

aclib2_test_problems <- function(path) {
  folders <- str_split(path, '/', simplify = T)
  no_folders <- length(folders)
  scenario <- folders[,no_folders-1]
  tunner <- 'IRACE3'
  
  scenario_dt <- str_split(scenario, '_', 2, simplify = T)
  algo <- scenario_dt[,1]
  scenario_problem <- scenario_dt[,2]
  
  # train_instances_path <- here('aclib2', 'fsp', 'instances', 'fsp', 'sets', scenario_problem, 'training.txt')
  test_instances_path <- here('aclib2', 'fsp', 'instances', 'fsp', 'sets', scenario_problem, 'test.txt')
  
  # train_instances <- read_csv(train_instances_path, col_names = F) %>% pull(X6) %>% as.list()
  test_instances <- read_csv(test_instances_path, col_names = F) %>% pull(X6) %>% as.list()
  
  problem_dt <- str_split(scenario_dt[,2], ',', simplify = T)
  problem <- problem_dt[,1]
  objective <- problem_dt[,2]
  type <- problem_dt[,3]
  stopping_criterion <- problem_dt[,4]
  budget <- problem_dt[,5]
  dist <- problem_dt[,6]
  corr <- problem_dt[,7]
  no_jobs <- problem_dt[,8]
  no_machines <- problem_dt[,9]
  
  problem_dt <- list(
    scenario = scenario,
    tunner = tunner,
    algo = algo,
    scenario_problem = scenario_problem,
    problem = problem,
    objective = objective,
    type = type,
    stopping_criterion = stopping_criterion,
    budget = budget,
    dist = dist,
    corr = corr,
    no_jobs = no_jobs,
    no_machines = no_machines,
    path = path
  )
  
  problem_obj <- Problem(
    name = scenario_problem,
    instances = test_instances,
    data = problem_dt
  )
  
  problem_dt$problem_obj <- list(problem_obj)
  
  problem_dt
}

scenarios <- list.files(
  here('aclib2', 'fsp', 'scenarios'), 
  recursive = T,
  full.names = T,
  pattern = 'scenario.txt'
)

problems_dt <- scenarios %>% 
  map_df(aclib2_problems)

problem_space <- ProblemSpace(problems = problems_dt$problem_obj)

algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')

algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('aclib2', 'irace_results')
dir.create(cache_folder, showWarnings = F)

# library(future)
# plan(multisession)

irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = fsp_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000,
    initConfigurations = default_neh#,
    # testType = 't-test-holm'
  )),
  cache_folder = cache_folder,
  parallel = 1
)

test_problems_dt <- scenarios %>% 
  map_df(aclib2_test_problems)

irace_results <- irace_trained %>%
  left_join(test_problems_dt, by = c("problem_names" = "scenario_problem")) %>%
  mutate(config = map(results, ~.x[1,])) %>%
  mutate(perf = map2(config, problem_obj, function(.x, .y) {
    perf <- sample_performance(
      algorithm = algorithm, 
      config = .x, 
      problemSpace = ProblemSpace(problems = list(.y)),
      solve_function = solve,
      no_samples = 1
    )
    tibble(instance = unlist(.y@instances), fitness = map_dbl(perf$result, ~.x$cost))
  }))

results_dt <- irace_results %>%
  unnest(perf) %>%
  mutate(
    tunner = 'IRACE3',
    conf = 'best',
    instance = paste(problem, objective, type, stopping_criterion, budget, instance, sep = ',')
  ) %>%
  rename(scenario_problem = problem_names) %>%
  select(scenario, tunner, conf, algo, scenario_problem, problem, objective, type,
         stopping_criterion, budget, dist, corr, no_jobs, no_machines, path,
         instance, fitness)

saveRDS(results_dt, file = here('aclib2', 'irace_results.rda'))
