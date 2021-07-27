library(MOEADr)
library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)
library(optparse)


parameter_space <- readParameters(
  text = '
IG.Init                            "" c (neh)
IG.Init.NEH.Ratio                  "" c (0)
IG.Init.NEH.Priority               "" c (sum_pij)
IG.Init.NEH.PriorityOrder          "" c (incr)
IG.Init.NEH.PriorityWeighted       "" c (no)
IG.Init.NEH.Insertion              "" c (first_best)
IG.Comp.Strat                      "" c (strict,equal)
IG.Neighborhood.Size               "" r (0.0, 1.0)
IG.Neighborhood.Strat              "" c (random)
IG.LS.Single.Step                  "" c (0, 1)
IG.Accept                          "" c (always, better, temperature)
IG.Accept.Better.Comparison        "" c (strict)
IG.Perturb.Insertion               "" c (random_best,first_best,last_best)
IG.Perturb                         "" c (rs,lsps,swap)
IG.LSPS.Local.Search               "" c (none,first_improvement,best_improvement,random_best_improvement,best_insertion) | IG.Perturb == "lsps"
IG.LSPS.Single.Step                "" c (0,1) | IG.Perturb == "lsps" & IG.LSPS.Local.Search != "none"
IG.Perturb.DestructionSizeStrategy "" c (fixed)
IG.DestructionStrategy             "" c (random)
IG.Local.Search                    "" c (none,first_improvement,best_improvement,random_best_improvement,best_insertion)
IG.Accept.Temperature              "" r (0.0, 5.0) | IG.Accept == "temperature"
IG.Perturb.DestructionSize         "" i (2,8) | IG.Perturb == "rs" || IG.Perturb == "lsps"
IG.Perturb.NumberOfSwapsStrategy   "" c (fixed) | IG.Perturb == "swap"
IG.Perturb.NumberOfSwaps           "" i (1,8) | IG.Perturb == "swap" && IG.Perturb.NumberOfSwapsStrategy == "fixed"
'
)


algorithm <- Algorithm(name = "IG", parameters = parameter_space)

obj_problems <- all_problems_df() %>%
  filter(
    problem == "flowshop",
    no_jobs %in% c(100),
    no_machines %in% c(20),
    type %in% c('PERM'),
    dist %in% c('exponential', 'uniform'),
    stopping_criterion == 'TIME',
    budget == 'low'
  ) %>%
  mutate(stopping_criterion = "FIXEDTIME") %>%
  unnest(instances) %>%
  filter(inst_n <= 6) %>%
  mutate(id = id * 100 + inst_n) %>%
  nest(instances = c(inst_n, instance)) %>%
  rowwise() %>%
  mutate(meta_objective = which(objective == c("FLOWTIME", "MAKESPAN"))) %>%
  ungroup() %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

type_problems <- all_problems_df() %>%
  filter(
    problem == "flowshop",
    no_jobs %in% c(100),
    no_machines %in% c(20),
    type %in% c('PERM', 'NOWAIT', 'NOIDLE'),
    objective %in% c('MAKESPAN'),
    dist %in% c('exponential', 'uniform'),
    stopping_criterion == 'TIME',
    budget == 'low'
  ) %>%
  mutate(stopping_criterion = "FIXEDTIME") %>%
  unnest(instances) %>%
  filter(inst_n <= 6) %>%
  mutate(id = id * 100 + inst_n) %>%
  nest(instances = c(inst_n, instance)) %>%
  rowwise() %>%
  mutate(meta_objective = which(type == c('PERM', 'NOWAIT', 'NOIDLE'))) %>%
  ungroup() %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

lower_bounds <- read_csv(here("data", "lower_bounds.csv")) %>%
  rename(best_cost = cost)
lb_features <- c("problem", "dist", "corr", "no_jobs",
                 "no_machines", "type", "objective", "instance")

experiments <- list(
  "objective-medium-hp" = list(
    # parameters
    parameter_space = parameter_space,
    algorithm = algorithm,
    # problems
    problems = obj_problems,
    objective_feature = "objective",
    objectives = c("FLOWTIME", "MAKESPAN"),
    eval_problems = obj_problems %>% 
      filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
      unnest(instances) %>%
      inner_join(lower_bounds, by = lb_features) %>%
      mutate(problem_space = map2(problem_space, best_cost, ~{
        .x@data['best_cost'] <- .y
        .x
      })),
    # moead parameters
    moead_variation = "irace",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 50,
    # irace variation
    irace_variation_problems = obj_problems %>% filter(map_lgl(instances, ~ .x$inst_n < 6)),
    irace_variation_no_evaluations = 100,
    irace_variation_no_samples = 10
  ),
  "objective-medium-ga-hp" = list(
    # parameters
    parameter_space = parameter_space,
    algorithm = algorithm,
    # problems
    problems = obj_problems,
    objective_feature = "objective",
    objectives = c("FLOWTIME", "MAKESPAN"),
    eval_problems = obj_problems %>% 
      filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
      unnest(instances) %>%
      inner_join(lower_bounds, by = lb_features) %>%
      mutate(problem_space = map2(problem_space, best_cost, ~{
        .x@data['best_cost'] <- .y
        .x
      })),
    # moead parameters
    moead_variation = "ga",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 650
  ),
  "type-medium-lin" = list(
    # parameters
    parameter_space = parameter_space,
    algorithm = algorithm,
    # problems
    problems = type_problems,
    objective_feature = "objective",
    objectives = c("PERM", "NOWAIT", "NOIDLE"),
    eval_problems = type_problems %>% 
      filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
      unnest(instances) %>%
      inner_join(lower_bounds, by = lb_features) %>%
      mutate(problem_space = map2(problem_space, best_cost, ~{
        .x@data['best_cost'] <- .y
        .x
      })),
    eval_no_samples = 1,
    # moead parameters
    moead_variation = "irace",
    moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
    moead_neighbors = list(name = "lambda", T = 3, delta.p = 1),
    moead_max_iter = 50,
    # irace variation
    irace_variation_problems = type_problems %>% 
      filter(map_lgl(instances, ~ .x$inst_n < 6)),
    irace_variation_no_evaluations = 100,
    irace_variation_no_samples = 10
  ),
  "type-medium-lin-ga" = list(
    # parameters
    parameter_space = parameter_space,
    algorithm = algorithm,
    # problems
    problems = type_problems,
    objective_feature = "objective",
    objectives = c("PERM", "NOWAIT", "NOIDLE"),
    eval_problems = type_problems %>% 
      filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
      unnest(instances) %>%
      inner_join(lower_bounds, by = lb_features) %>%
      mutate(problem_space = map2(problem_space, best_cost, ~{
        .x@data['best_cost'] <- .y
        .x
      })),
    eval_no_samples = 1,
    # moead parameters
    moead_variation = "ga",
    moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
    moead_neighbors = list(name = "lambda", T = 3, delta.p = 1),
    moead_max_iter = 467
  )
)

opt <- parse_args(OptionParser(option_list=list(
  make_option(c("-e", "--experiment"), default="type-medium-lin-ga")
)))

EXP_NAME <- opt$experiment
EXP <- experiments[[EXP_NAME]]

plan(sequential)
plan(multisession, workers = 32)
# plan(remote,
#      workers = rep("linode2", 32),
#      persistent = TRUE)
# 
# 
# 
# set.seed(54654654)
# 

arpf_by_objective <- function(perfs) {
  sampled_data <- perfs %>%
    unnest(perf) %>%
    rename(problem_obj = problem) %>%
    mutate(
      problem_data = map(problem_obj, ~.x@data),
      dist = map_chr(problem_data, 'dist'),
      corr = map_chr(problem_data, 'corr'),
      no_jobs = as.integer(map_chr(problem_data, 'no_jobs')),
      no_machines = as.integer(map_chr(problem_data, 'no_machines')),
      problem = map_chr(problem_data, 'problem'),
      type = map_chr(problem_data, 'type'),
      objective = map_chr(problem_data, 'objective'),
      cost = as.integer(map_dbl(result, 'cost')),
      meta_objective = as.integer(map_dbl(problem_data, 'meta_objective'))
    )

  res <- sampled_data %>%
    mutate(best_cost = map_dbl(problem_data, 'best_cost')) %>%
    mutate(arpf = 100 * (cost - best_cost) / best_cost) %>%
    group_by(conf_id, meta_objective) %>%
    summarize(performance = mean(arpf)) %>%
    ungroup()

  res
}


solve_function <- fsp_solver_performance
# solve_function <- function(...) {
#   print("before")
#   res <- fsp_solver_performance_print(...)
#   print("after")
#   flush.console()
#   res
# }

test_problems_eval <- make_performance_sample_evaluation(
  algorithm = algorithm,
  problem_space = ProblemSpace(problems = EXP$eval_problems$problem_space),
  solve_function = solve_function,
  no_samples = EXP$eval_no_samples,
  parallel = TRUE,
  aggregate_performances = arpf_by_objective,
  cache_folder = here("data", "problem_partition", EXP_NAME)
)

moead_problem <- list(
  name   = "test_problems_eval",
  xmin   = real_lower_bounds(parameter_space, fixed = FALSE),
  xmax   = real_upper_bounds(parameter_space, fixed = FALSE),
  m      = length(EXP$objectives)
)

variation <- NULL

variation_irace <- NULL
if (EXP$moead_variation == "irace") {
  variation_irace <<- make_irace_variation(
    algorithm = algorithm,
    problem_space = ProblemSpace(problems = EXP$irace_variation_problems$problem_space),
    solve_function = solve_function,
    irace_scenario = defaultScenario(
      list(
        maxExperiments = EXP$irace_variation_no_evaluations,
        minNbSurvival = 1
      )
    ),
    no_samples = EXP$irace_variation_no_samples,
    cache_folder = here("data", "problem_partition", EXP_NAME)
  )
  variation <<- list(list(name = "irace"))
} else if (EXP$moead_variation == "ga") {
  variation <<- preset_moead("original")$variation
}

results <- moead(
  preset   = preset_moead("original"),
  problem  = moead_problem,
  variation = variation,
  decomp = EXP$moead_decomp,
  showpars = list(show.iters = "numbers", showevery = 1),
  neighbors = EXP$moead_neighbors,
  stopcrit = list(list(name  = "maxiter",
                       maxiter  = EXP$moead_max_iter)),
  seed     = 42
)

saveRDS(results, file = sprintf("results_%s.rds", EXP_NAME))



# 
results <- readRDS(sprintf("results_%s.rds", EXP_NAME))

final_configs <- population_to_configs(results$X, EXP$parameter_space)

for (i in 1:nrow(final_configs)) {
  config <- df_to_character(final_configs[i, ])
  set.seed(42)
  perf <- sample_performance(
    algorithm = EXP$algorithm,
    solve_function = solve_function,
    problemSpace = ProblemSpace(problems = EXP$eval_problems$problem_space),
    no_samples = 10,
    cache = here("data", "problem_partition", EXP_NAME, sprintf("perf_%d", i)),
    parallel = TRUE,
    config = config
  )
}
# 
# 
# plt <- ggplot(as_tibble(results$Y)) +
#   geom_point(aes(x = f1, y = f2)) +
#   xlab("FLOWTIME") +
#   ylab("MAKESPAN")
# 
# ggsave(plt, filename = sprintf("%s_final.png", EXP_NAME), width = 9, height = 6)
# 
# plt <- tibble(
#   generation = 1:40
# ) %>%
#   mutate(fn = here(paste0("data/problem_partition/", EXP_NAME, "/variation_input_objs_", generation, ".csv"))) %>%
#   mutate(res = map(fn, read_csv)) %>%
#   unnest(res) %>%
#   select(FLOWTIME = V1, MAKESPAN = V2, generation) %>%
#   ggplot() +
#   geom_point(aes(x = FLOWTIME, y = MAKESPAN, color = generation))
# 
# 
# ggsave(plt, filename = sprintf("%s_evolution1.png", EXP_NAME), width = 9, height = 6)
# 
# plt <- tibble(
#   generation = 20:40
# ) %>%
#   mutate(fn = here(paste0("data/problem_partition/", EXP_NAME, "/variation_input_objs_", generation, ".csv"))) %>%
#   mutate(res = map(fn, read_csv)) %>%
#   unnest(res) %>%
#   select(FLOWTIME = V1, MAKESPAN = V2, generation) %>%
#   ggplot() +
#   geom_point(aes(x = FLOWTIME, y = MAKESPAN, color = generation))
# 
# 
# ggsave(plt, filename = sprintf("%s_evolution2.png", EXP_NAME), width = 9, height = 6)
