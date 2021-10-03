library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)

# plan(sequential)
plan(multisession, workers = 4)

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

lower_bounds <- read_csv(here("data", "lower_bounds.csv")) %>%
  rename(best_cost = cost)
lb_features <- c("problem", "dist", "corr", "no_jobs",
                 "no_machines", "type", "objective", "instance")

obj_problems <- all_problems_df() %>%
  filter(
    problem == "flowshop",
    no_jobs %in% c(50),
    no_machines %in% c(10),
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
  mutate(problem_space = pmap(., as_metaopt_problem)) %>% 
  filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
  unnest(instances) %>%
  inner_join(lower_bounds, by = lb_features) %>%
  mutate(problem_space = map2(problem_space, best_cost, ~{
    .x@data['best_cost'] <- .y
    .x
  }))

type_problems <- all_problems_df() %>%
  filter(
    problem == "flowshop",
    no_jobs %in% c(50),
    no_machines %in% c(10),
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
  mutate(problem_space = pmap(., as_metaopt_problem)) %>%
  filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
  unnest(instances) %>%
  inner_join(lower_bounds, by = lb_features) %>%
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



experiments <- tribble(
  ~name, ~name_print, ~experiment_data,
  "flowshop-objective-50j10m", "MOEA/D+irace", list(
    strategy = "moead",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = obj_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    eval_no_samples = 4,
    # moead parameters
    moead_variation = "irace",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 50,
    # irace variation
    irace_variation_problems = obj_problems,
    irace_variation_no_evaluations = 300,
    irace_variation_no_samples = 4
  ),
  "flowshop-objective-50j10m-ga", "MOEA/D", list(
    strategy = "moead",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = obj_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    eval_no_samples = 4,
    # moead parameters
    moead_variation = "ga",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 519
  ),
  "flowshop-objective-50j10m-irace", "irace", list(
    strategy = "irace",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = obj_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    eval_no_samples = 4,
    # irace parameters
    irace_max_evals = 132864
  ),
  "flowshop-type-50j10m", "MOEA/D+irace", list(
    strategy = "moead",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = type_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    eval_no_samples = 1,
    # moead parameters
    moead_variation = "irace",
    moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 50,
    # irace variation
    irace_variation_problems = type_problems,
    irace_variation_no_evaluations = 300,
    irace_variation_no_samples = 4
  ),
  "flowshop-type-50j10m-ga", "MOEA/D", list(
    strategy = "moead",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = type_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    eval_no_samples = 1,
    # moead parameters
    moead_variation = "ga",
    moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 1300
  ),
  "flowshop-type-50j10m-irace", "irace", list(
    strategy = "irace",
    # parameters
    algorithm = algorithm,
    # problems
    eval_problems = type_problems,
    solve_function = fsp_solver_performance,
    aggregation_function = arpf_by_objective,
    # moead parameters
    irace_max_evals = 249600
  ) #,
  # "type-medium-lin" = list(
  #   # parameters
  #   parameter_space = parameter_space,
  #   algorithm = algorithm,
  #   # problems
  #   problems = type_problems,
  #   objective_feature = "objective",
  #   objectives = c("PERM", "NOWAIT", "NOIDLE"),
  #   eval_problems = type_problems %>% 
  #     filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
  #     unnest(instances) %>%
  #     inner_join(lower_bounds, by = lb_features) %>%
  #     mutate(problem_space = map2(problem_space, best_cost, ~{
  #       .x@data['best_cost'] <- .y
  #       .x
  #     })),
  #   eval_no_samples = 1,
  #   # moead parameters
  #   moead_variation = "irace",
  #   moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
  #   moead_neighbors = list(name = "lambda", T = 3, delta.p = 1),
  #   moead_max_iter = 50,
  #   # irace variation
  #   irace_variation_problems = type_problems %>% 
  #     filter(map_lgl(instances, ~ .x$inst_n < 6)),
  #   irace_variation_no_evaluations = 100,
  #   irace_variation_no_samples = 10
  # ),
  # "type-medium-lin-ga" = list(
  #   # parameters
  #   parameter_space = parameter_space,
  #   algorithm = algorithm,
  #   # problems
  #   problems = type_problems,
  #   objective_feature = "objective",
  #   objectives = c("PERM", "NOWAIT", "NOIDLE"),
  #   eval_problems = type_problems %>% 
  #     filter(map_lgl(instances, ~ .x$inst_n == 6)) %>%
  #     unnest(instances) %>%
  #     inner_join(lower_bounds, by = lb_features) %>%
  #     mutate(problem_space = map2(problem_space, best_cost, ~{
  #       .x@data['best_cost'] <- .y
  #       .x
  #     })),
  #   eval_no_samples = 1,
  #   # moead parameters
  #   moead_variation = "ga",
  #   moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
  #   moead_neighbors = list(name = "lambda", T = 3, delta.p = 1),
  #   moead_max_iter = 467
  # )
) %>%
  mutate(
    configs = pmap(., run_experiment, 
                   folder = here("data", "problem_partition"))
  ) %>%
  mutate(
    validation = pmap(., run_validation,
                      folder = here("data", "problem_partition"),
                      aggregation_function = arpf_by_objective)
  )



plt_dt <- experiments %>%
  filter(startsWith(name, "flowshop-objective")) %>%
  mutate(perfs = map(validation, 'perf')) %>% 
  select(name, name_print, perfs) %>%
  unnest(perfs) %>%
  pivot_wider(names_from = meta_objective, values_from = performance) %>%
  left_join(
    experiments %>%
      mutate(pop = map(validation, 1)) %>% 
      select(name, pop) %>% unnest(pop),
    by = c('name', 'conf_id')
  ) %>%
  select(name, name_print, flowtime = `1`, makespan = `2`, conf_id,
         IG.Perturb.DestructionSize)

plt_dt %>%
  ggplot() +
  geom_point(aes(x = flowtime, y = makespan, color = name_print,
                 size = IG.Perturb.DestructionSize), alpha = .85) +
  theme_minimal() +
  scale_x_log10() +
  scale_y_log10() +
  labs(color = NULL, size = "d")

ggsave("flowshop-objective-front.pdf", width = 9, height = 5, device=cairo_pdf)


plt_dt <- experiments %>%
  filter(startsWith(name, "flowshop-type")) %>%
  mutate(perfs = map(validation, 'perf')) %>% 
  select(name, name_print, perfs) %>%
  unnest(perfs) %>%
  mutate(
    meta_objective = case_when(
      meta_objective == 1 ~ "permutation",
      meta_objective == 2 ~ "no-wait",
      meta_objective == 3 ~ "no-idle"
    )
  ) %>%
  left_join(
    experiments %>%
      mutate(pop = map(validation, 1)) %>% 
      select(name, pop) %>% unnest(pop),
    by = c('name', 'conf_id')
  ) %>%
  select(name, name_print, meta_objective, performance, conf_id)

nd_points <- plt_dt %>% 
  select(name_print, conf_id, meta_objective, performance) %>% 
  pivot_wider(names_from = meta_objective, values_from = performance) %>%
  mutate(rn = paste(name_print, conf_id, sep = ";")) %>%
  column_to_rownames("rn") %>%
  select(-name_print, -conf_id) %>%
  mutate(non_dominated = MOEADr::find_nondominated_points(.)) %>%
  rownames_to_column("name_print_conf_id") %>%
  separate(name_print_conf_id, c("name_print", "conf_id"), sep = ";") %>%
  mutate(conf_id = as.integer(conf_id))

plt_dt %>%
  left_join(nd_points, by = c("name_print", "conf_id")) %>%
  ggplot() +
  geom_line(aes(
    x = meta_objective, 
    y = performance, 
    linetype = non_dominated,
    color = name_print,
    group = paste(name_print, conf_id)),
    alpha = .85
  ) +
  scale_y_log10() +
  theme_minimal() +
  labs(color = NULL, linetype = "non-dominated")

ggsave("flowshop-type-front.pdf", width = 9, height = 5, device=cairo_pdf)
