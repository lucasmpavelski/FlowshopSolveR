library(FlowshopSolveR)
library(here)
library(furrr)
library(optparse)


lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

option_list <- list( 
  make_option(c("-c", "--config_id"), default=10, type='integer',
              help="Print extra output [default]"),
  
  make_option(c("-p", "--problem"), default="vrf-large", type='character',
              help="Print extra output [default]"),
  
  make_option(c("-i", "--inst"), default="1,2,3,4,5,6,7,8,9,10", type='character',
              help="Print extra output [default]"),
  
  make_option(c("-w", "--workers"), default=2, type='integer',
              help="Print extra output [default]")
)

# write_rds(lon_configs, here('reports/lons_study/lon_configs.rds'))


opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

sample_lon <- function(problem, file_name, sample_n) {
  savePath <- file_name # here('data', 'lons_cache', file_name)
  if (!file.exists(savePath)) {
    print(savePath)
    initFactories(here("data"))
    lon <- sampleLON(
      sampleType = lon_configs[[CONFIG_ID]]$sample_type,
      rproblem = problem,
      rsampling = lon_configs[[CONFIG_ID]]$params,
      seed = 123 * sample_n
    )
    saveRDS(lon, file = savePath)
    rm(lon)
  }
  1
}

no_samples <- ifelse(lon_configs[[CONFIG_ID]]$sample_type == 'snowball', 1, 50)

print(opt$problem)

problems <- all_problems_df() %>%
  crossing(sample_n = 1:no_samples) %>%
  filter(
    problem == opt$problem,
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs == 400
  ) %>%
  unnest(cols = instances) %>%
  filter(inst_n %in% as.integer(str_split(opt$inst, ',', simplify = T))) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(lon_metrics = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, sample_n, ...) {
    c(
      problem = problem,
      type = type,
      objective = objective,
      budget = budget,
      instance = instance,
      stopping_criterion = stopping_criterion
    )
  })) %>%
  mutate(
    file_name = map2_chr(lon_metrics, sample_n, ~here('data', 'lons_cache', sprintf("%s_%s_%d.rds",
                        lon_configs[[CONFIG_ID]]$id,
                        paste(.x, collapse = ";", sep = "_"),
                        .y)))
  ) %>%
   filter(!file.exists(file_name))

plan(multisession(workers= opt$workers))
# plan(remote, workers = rep("linode2", 32), persistent = TRUE)
# plan(sequential)
# 
library(progressr)
handlers(global = TRUE)
handlers("progress")

sample_all_lons <- function() {
  p <- progressor(along = seq_along(problems$lon_metrics))
  y <- future_map(seq_along(problems$lon_metrics), function(i) {
    prob <- problems$lon_metrics[[i]]
    samp <- problems$file_name[i]
    sample_n <- problems$sample_n[i]
    sample_lon(prob, samp, sample_n)
    p(sprintf("x=%g", i))
  })
}

sample_all_lons()
plan(sequential)
