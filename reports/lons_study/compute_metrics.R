library(FlowshopSolveR)
library(here)
library(tidygraph)
library(furrr)
library(optparse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

option_list <- list( 
  make_option(c("-c", "--config_id"), action="dist", default=9, type='integer',
              help="Print extra output [default]")
)

opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

lons_folder <- here("data", "lons_cache")



compute_metrics <- function(lons_folder, problem_config, lon_config, sample_type) {
  lons_folder <- here("data", "lons_cache")
  save_path <- sprintf("%s/%s_%s_metrics.rds", lons_folder, lon_config, problem_config)
  lon_path <- ifelse(sample_type == 'snowball',
                     sprintf("%s/%s_%s_1.rds", lons_folder, lon_config, problem_config),
                     sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config)
                     )
  if (!file.exists(save_path) && file.exists(lon_path)) {
    print(sprintf("%s_%s", lon_config, problem_config))
    full_lon <- readRDS(lon_path)
    lon_fla <- c(
      lonMetrics(full_lon),
      neutralMetrics(full_lon),
      graphMetrics(full_lon),
      meanDistanceBetweenLocalOptimas(full_lon)
    )
    rm(full_lon)
    write_rds(lon_fla, file = save_path)
  }
  #else {
  # print(lon_path)
  #  full_lon <- read_rds(lon_path)
  #  full_lon$nodes$solutions <- map(full_lon$nodes$solutions, as.integer)
  #  saveRDS(full_lon, file = lon_path)
   # new_fla <- c(
   #   lonMetrics(full_lon),
   #   neutralMetrics(full_lon)
   # )
   # fla[names(new_fla)] <- new_fla
   # rm(full_lon)
  # }
  save_path
}

problems <- all_problems_df() %>%
  filter(
    problem == 'vrf-large',
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 400
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  })) %>%
  mutate(
    save_path = map_chr(problem_id, ~sprintf("%s/%s_%s_metrics.rds", lons_folder,
                                lon_configs[[CONFIG_ID]]$id,
                                .x))
  ) %>%
  filter(!file.exists(save_path))


library(progressr)
handlers(global = T)
handlers("progress")

# plan(multisession(workers = 2))
# plan(remote, workers = rep("linode2", 10), persistent = TRUE)

compute_all_metrics <- function() {
  p <- progressor(along = seq_along(problems$problem_id))
  y <- map(seq_along(problems$problem_id), function(i) {
    prob <- problems$problem_id[[i]]
    lon <- lon_configs[[CONFIG_ID]]$id
    sample_type <- lon_configs[[CONFIG_ID]]$sample_type
    compute_metrics(lons_folder, prob, lon, sample_type)
    p(sprintf("x=%g", i))
  })
}

compute_all_metrics()

