library(MOEADr)
library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)

EXP_NAME <- "objective-medium-hp"

# plan(sequential)
plan(multisession, workers = 8)
# # plan(remote,
# #      workers = rep("linode2", 8),
# #      persistent = TRUE)
# 
# 
# 
# set.seed(54654654)
# 
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
IG.Perturb.NumberOfSwaps           "" i (2,8) | IG.Perturb == "swap" && IG.Perturb.NumberOfSwapsStrategy == "fixed"
'
)


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

all_problems <- type_problems

objective_axis <- unique(all_problems$objective)
all_problems <- all_problems %>%
  rowwise() %>%
  mutate(meta_objective = which(objective == objective_axis)) %>%
  ungroup() %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

train_test_problems <- all_problems %>%
  group_split(sample.int(nrow(all_problems)) < 0.2 * nrow(all_problems))
train_problems <-
  all_problems %>% filter(map_lgl(instances, ~ .x$inst_n < 6)) # train_test_problems[[1]]


lower_bounds <- read_csv(here("data", "lower_bounds.csv")) %>%
  rename(best_cost = cost)

test_problems <-
  all_problems %>% filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
  unnest(instances) %>%
  inner_join(lower_bounds, by = c("problem", "dist", "corr", "no_jobs", "no_machines", "type", "objective", "instance")) %>%
  mutate(problem_space = map2(problem_space, best_cost, ~{
    .x@data['best_cost'] <- .y
    .x
  }))


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

algorithm <- Algorithm(name = "IG", parameters = parameter_space)

best_solvers <- train_best_solver(
  problem_space = ProblemSpace(problems = all_problems$problem_space),
  algorithm = algorithm,
  solve_function = solve_function,
  irace_scenario = defaultScenario(
    list(
      maxExperiments = 44000,
      minNbSurvival = 1
    )
  ),
  parallel = 8,
  quiet = F,
  cache = here("data", "problem_partition", "medium-irace"),
  recover = TRUE
)

set.seed(42)
perf <- sample_performance(
  algorithm = algorithm,
  solve_function = solve_function,
  problemSpace = ProblemSpace(problems = test_problems$problem_space),
  no_samples = 10,
  cache = here("data", "problem_partition", "medium-irace-perf"),
  parallel = TRUE,
  config = df_to_character(best_solvers)
)

# solve_function <- function(...) {
#   print("before")
#   res <- fsp_solver_performance_print(...)
#   print("after")
#   flush.console()
#   res
# }

# test_problems_eval <- make_performance_sample_evaluation(
#   algorithm = algorithm,
#   problem_space = ProblemSpace(problems = test_problems$problem_space),
#   solve_function = solve_function,
#   no_samples = 1,
#   parallel = TRUE,
#   aggregate_performances = arpf_by_objective,
#   cache_folder = here("data", "problem_partition", EXP_NAME),
#   debug = TRUE
# )

# moead_problem <- list(
#   name   = "test_problems_eval",
#   xmin   = real_lower_bounds(parameter_space, fixed = FALSE),
#   xmax   = real_upper_bounds(parameter_space, fixed = FALSE),
#   m      = n_distinct(test_problems$meta_objective)
# )

# variation_irace <- make_irace_variation(
#   algorithm = algorithm,
#   problem_space = ProblemSpace(problems = train_problems$problem_space),
#   solve_function = solve_function,
#   irace_scenario = defaultScenario(
#     list(
#       maxExperiments = 100,
#       minNbSurvival = 1
#     )
#   ),
#   no_samples = 10,
#   cache_folder = here("data", "problem_partition", EXP_NAME)
# )
# 
# results <- moead(
#   problem  = moead_problem,
#   preset   = preset_moead("original"),
#   variation = list(list(name  = "irace")),
#   decomp = list(name = "SLD", H = 7),
#   showpars = list(show.iters = "numbers", showevery = 1),
#   neighbors = list(name       = "lambda",
#                    T          = 2,
#                    delta.p    = 1),
#   stopcrit = list(list(name  = "maxiter",
#                        maxiter  = 50)),
#   seed     = 42
# )
# 
# 
# plan(sequential)
# 
# saveRDS(results, file = sprintf("results_%s.rds", EXP_NAME))
# 
# results <- readRDS(sprintf("results_%s.rds", EXP_NAME))
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
