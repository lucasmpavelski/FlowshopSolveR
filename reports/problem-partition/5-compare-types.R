library(MOEADr)
library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)
library("ggplot2")

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


all_problems <- all_problems_df() %>%
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
  mutate(id = id * 100 + inst_n) %>%
  nest(instances = c(inst_n, instance)) %>%
  rowwise() %>%
  mutate(meta_objective = which(type == c('PERM', 'NOWAIT', 'NOIDLE'))) %>%
  ungroup()

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


validation_problems <- all_problems %>%
  filter(map_lgl(instances, ~ .x$inst_n > 6)) %>%
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
# solve_function <- function(...) {
#   print("before")
#   res <- fsp_solver_performance_print(...)
#   print("after")
#   flush.console()
#   res
# }

algorithm <- Algorithm(name = "IG", parameters = parameter_space)

eval_config <- function(conf_id, exp_name, ...) {
  config <- df_to_character(list(...))
  cat("evaluating ", conf_id, exp_name, "...\n")
  set.seed(42)
  sample_performance(
    algorithm = algorithm,
    solve_function = solve_function,
    problemSpace = ProblemSpace(problems = validation_problems$problem_space),
    no_samples = 10,
    cache = here("data", "problem_partition", exp_name, sprintf("validation_%d", conf_id)),
    parallel = TRUE,
    config = config
  )
}


read_moead_final_pop <- function(exp_name) {
  readRDS(sprintf("results_%s.rds", exp_name))$X %>%
    population_to_configs(parameter_space)
}

read_moead_final_pop_perf <- function(exp_name) {
  perf <- readRDS(sprintf("results_%s.rds", exp_name))$Y
  perf %>%
    as_tibble() %>%
    mutate(conf_id = row_number()) %>%
    mutate(nd = find_nondominated_points(perf))
}

read_moead_perfs <- function(exp_name) {
  read_moead_final_pop(exp_name) %>%
    mutate(perf = pmap(., eval_config, exp_name = exp_name)) %>%
    arpf_by_objective() %>%
    mutate(strategy = exp_name)
}

irace_best <- readRDS(here("data", "problem_partition", "type-medium-100jobs-20machines-irace"))
plan(multisession)

irace_perf <- irace_best %>%
  filter(row_number() == 1) %>%
  removeConfigurationsMetaData() %>%
  as_tibble() %>%
  mutate(perf = pmap(., function(...) {
    cat("evaluating irace_best...\n")
    # cat(paste(df_to_character(list(...)), collapse = " "))
    config <- df_to_character(list(...))
    config[is.na(config)] <- "1"
    config[config == "NA"] <- "1"
    set.seed(42)
    sample_performance(
      algorithm = algorithm,
      solve_function = solve_function,
      problemSpace = ProblemSpace(problems = validation_problems$problem_space),
      no_samples = 10,
      cache = here("data", "problem_partition", "type-irace-validation"),
      parallel = T,
      config = config
    )
  })) %>%
  mutate(conf_id = 1) %>%
  arpf_by_objective() %>%
  mutate(strategy = "irace")


moead_irace <- read_moead_perfs("type-medium-lin") %>% mutate(strategy = "MOEA/D with irace variation")
moead_ga <- read_moead_perfs("type-medium-lin-ga") %>% mutate(strategy = "MOEA/D")


perfs <- bind_rows(moead_irace, moead_ga, irace_perf)

perfs %>%
  pivot_wider(names_from = meta_objective, values_from = performance) %>%
  rename(PERM = `1`, NOWAIT = `2`, NOIDLE = `3`)  %>%
  pivot_longer(c('PERM', 'NOWAIT', 'NOIDLE')) %>%
  ggplot() +
  geom_path(aes(x = `name`, y = `value`, color = strategy, group = factor(strategy))) +
  theme_bw() +
  theme(legend.position = "bottom") +
  ylab("ARPD") +
  xlab(NULL) +
  scale_y_log10() +
  scale_colour_viridis_d()

ggsave("types-test-problems.png", width=7, height = 4)


irace_final_pop_perf <- irace_best %>%
  removeConfigurationsMetaData() %>%
  as_tibble() %>%
  mutate(perf = pmap(., function(...) {
    set.seed(42)
    sample_performance(
      algorithm = algorithm,
      solve_function = solve_function,
      problemSpace = ProblemSpace(problems = test_problems$problem_space),
      no_samples = 1,
      cache = here("data", "problem_partition", "type-medium-100jobs-20machines-irace-perf-final"),
      parallel = TRUE,
      config = df_to_character(list(...))
    )
  })) %>%
  mutate(conf_id = 1) %>%
  arpf_by_objective() %>%
  mutate(meta_objective = sprintf("f%d", meta_objective)) %>%
  select(-conf_id) %>%
  pivot_wider(names_from = meta_objective, values_from = performance)

final_pop_perfs <- irace_final_pop_perf %>%
  mutate(
    strategy = "irace",
    conf_id = 1,
    nd = TRUE
  ) %>%
  bind_rows(read_moead_final_pop_perf("type-medium-lin") %>%
              mutate(strategy = "MOEA/D with irace variation") %>%
              mutate(conf_id = row_number())
  ) %>%
  bind_rows(
    read_moead_final_pop_perf("type-medium-lin-ga") %>%
      mutate(strategy = "MOEA/D") %>%
      mutate(conf_id = row_number())
  ) %>%
  filter(nd)

final_pop_perfs %>%
  rename(PERM = `f1`, NOWAIT = `f2`, NOIDLE = `f3`)  %>%
  pivot_longer(c('PERM', 'NOWAIT', 'NOIDLE')) %>%
  ggplot() +
  geom_path(aes(x = `name`, y = `value`, color = strategy, group = factor(strategy))) +
  theme_bw() +
  theme(legend.position = "bottom") +
  ylab("ARPD") +
  xlab(NULL) +
  scale_y_log10() +
  scale_colour_viridis_d()

ggsave("types-final-pop.png", width=7, height = 4)

# results <- readRDS(sprintf("results_%s.rds", "objective-medium-ga-hp"))


# read_moead_final_pop("objective-medium-ga-hp") %>% write_csv("pop.csv")
