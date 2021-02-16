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
  
  make_option(c("-i", "--inst"), default="1", type='character',
              help="Print extra output [default]"),
  
  make_option(c("-w", "--workers"), default=, type='integer',
              help="Print extra output [default]")
)

write_rds(lon_configs, here('reports/lons_study/lon_configs.rds'))


opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

sample_lon <- function(problem, sample_n) {
  file_name <- sprintf("%s_%s_%d.rds",
                       lon_configs[[CONFIG_ID]]$id,
                       paste(problem, collapse = ";", sep = "_"),
                       sample_n)
  savePath <- here('data', 'lons_cache', file_name)
  if (!file.exists(savePath)) {
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
    no_jobs <= 400
  ) %>%
  unnest(cols = instances) %>%
  filter(inst_n %in% as.integer(str_split(opt$inst, ',', simplify = T))) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(lon_metrics = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, sample_n, ...) {
    prob <- c(
      problem = problem,
      type = type,
      objective = objective,
      budget = budget,
      instance = instance,
      stopping_criterion = stopping_criterion
    )
  }))

plan(multisession(workers= opt$workers))
# 
library(progressr)
handlers(global = TRUE)
handlers("progress")

sample_all_lons <- function() {
  p <- progressor(along = seq_along(problems$lon_metrics))
  y <- future_map(seq_along(problems$lon_metrics), function(i) {
    prob <- problems$lon_metrics[[i]]
    samp <- problems$sample_n[i]
    sample_lon(prob, samp)
    p(sprintf("x=%g", i))
  })
}

sample_all_lons()
