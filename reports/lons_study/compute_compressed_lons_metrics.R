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
  lon_path <- sprintf("%s/%s_%s_clon.rds", lons_folder, lon_config, problem_config)
  save_path <- sprintf("%s/%s_%s_clon_metrics.rds", lons_folder, lon_config, problem_config)
  if (!file.exists(save_path) && file.exists(lon_path)) {
    print(lon_path)
    lon <- read_rds(lon_path)
    lon$edges <- lon$edges %>%
      ungroup()
    clon_metrics <- c(
      lonMetrics(lon),
      graphMetrics(lon)
    )
    names(clon_metrics) <- paste0("clon_", names(clon_metrics))
    write_rds(clon_metrics, save_path)
    rm(lon)
  }
  1
}

problems <- all_problems_df() %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 300
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  }))


library(progressr)
# handlers(global = T)
handlers("progress")

# plan(multisession(workers = 4))

compute_all_compressed_lons <- function() {
  # p <- progressor(along = seq_along(problems$problem_id))
  y <- map(seq_along(problems$problem_id), function(i) {
    prob <- problems$problem_id[[i]]
    lon <- lon_configs[[CONFIG_ID]]$id
    sample_type <- lon_configs[[CONFIG_ID]]$sample_type
    compute_metrics(lons_folder, prob, lon, sample_type)
    # p(sprintf("x=%g", i))
  })
}

compute_all_compressed_lons()

