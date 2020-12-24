library(irace)
library(tidyverse)
library(FlowshopSolveR)
library(here)

# NEH.Init.NEH.Ratio NEH.Init.NEH.First.Priority NEH.Init.NEH.First.PriorityOrder

compute_ablation <- function(log_path, results_path, ablation_log_path, ablation_res_path, ...) {
  load(log_path)
  # if (iraceResults$scenario$execDir == '/src' || str_detect(iraceResults$scenario$logFile, '/src')) {
    iraceResults$scenario$execDir <- here()
    iraceResults$scenario$logFile <- str_replace(iraceResults$scenario$logFile, '/src', here())
    iraceResults$parameters$conditions[1:length(iraceResults$parameters$conditions)] <- T
    save(iraceResults, file = paste0(log_path, 'extra'))
  # }
  rm(iraceResults)
  result <- readRDS(results_path)
  print(result)
  if (!all(default_configs('NEH') == result[1,names(default_configs('NEH'))], na.rm = T)) {
    ablationResult <- ablation(paste0(log_path, 'extra'), 
                               result,
                               ab.params = c("NEH.Init.NEH.Ratio", "NEH.Init.NEH.First.Priority", 
                                             "NEH.Init.NEH.First.PriorityWeighted", "NEH.Init.NEH.First.PriorityOrder", 
                                             "NEH.Init.NEH.Priority", "NEH.Init.NEH.PriorityOrder", "NEH.Init.NEH.PriorityWeighted", 
                                             "NEH.Init.NEH.Insertion"),
                               ablationLogFile = ablation_log_path,
                               type = 'racing')
    saveRDS(ablationResult, file = ablation_res_path)
  } else {
    print("default!")
  }
}


save_ablation_best <- function(log_path, results_path, ablation_log_path, ablation_res_path, ablation_best_path, ...) {
  cfg <- NULL
  if (!file.exists(ablation_log_path)) {
    cfg <- default_configs('NEH')
  } else {
    load(ablation_log_path)
    trajectory <- ab.log$trajectory
    experiments <- ab.log$experiments
    costs.avg <- colMeans(experiments[, trajectory])
    best_idx <- as.integer(names(which.min(costs.avg)))
    cfg <- ab.log$configurations[best_idx,]
  }
  saveRDS(cfg, file = ablation_best_path)
}

problems_dt <- all_problems_df() %>%
  filter(budget == "low", no_jobs <= 500) %>%
  mutate(
    metaopt_problem = pmap(., as_metaopt_problem),
    performance_exists = map_lgl(
      metaopt_problem,
      ~ file.exists(here("data", "performances", .x@name, "NEH", "result.rds"))
    )
  ) %>%
  filter(performance_exists)

CACHE_FOLDER <- here("data", "performances")
algorithm <- get_algorithm('NEH')

problems_dt %>%
  mutate(
    log_path = map(metaopt_problem, ~file.path(CACHE_FOLDER, .x@name, algorithm@name, 'log.Rdata')),
    results_path = map(metaopt_problem, ~file.path(CACHE_FOLDER, .x@name, algorithm@name, 'result.rds')),
    ablation_log_path = map(metaopt_problem, ~file.path(CACHE_FOLDER, .x@name, algorithm@name, 'ablationLog.Rdata')),
    ablation_res_path = map(metaopt_problem, ~file.path(CACHE_FOLDER, .x@name, algorithm@name, 'ablation.rds')),
    ablation_best_path = map(metaopt_problem, ~file.path(CACHE_FOLDER, .x@name, algorithm@name, 'ablation_best.rds'))
  ) %>%
  # filter(!map_lgl(ablation_best_path, file.exists)) %>%
  mutate(ablation = pmap(., save_ablation_best))
