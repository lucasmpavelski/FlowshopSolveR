library(irace)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(metaOpt)
library(optparse)

option_list <- list( 
  make_option(c("-d", "--dist"), action="dist", default=NULL, type='character',
              help="Print extra output [default]"),
  make_option(c("-c", "--corr"), action="corr", default=NULL, type='character',
              help="Print extra output [default]"),
  make_option(c("-p", "--prob"), action="prob", default=NULL, type='character',
              help="Print extra output [default]")
)
opt <- parse_args(OptionParser(option_list=option_list))
dist_op <- opt$dist
corr_op <- opt$corr
prob_op <- opt$prob

problems_dt <- all_problems_df() %>%
  filter(budget == 'low', no_jobs <= 500)

if (!is.null(prob_op)) {
  problems_dt <- problems_dt %>%
    filter(problem == prob_op)
}
if (!is.null(dist_op)) {
  problems_dt <- problems_dt %>%
    filter(dist == dist_op)
}
if (!is.null(corr_op)) {
  problems_dt <- problems_dt %>%
    filter(corr == corr_op)
}

print(prob_op)
print(dist_op)
print(corr_op)
print(problems_dt %>% select(dist, corr))

problem_space <- fsp_problem_space(problems_dt)
algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')
algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('runs', 'neh')
dir.create(cache_folder, showWarnings = F)

library(future)
plan(multisession)

irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = fsp_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000,
    initConfigurations = default_neh
  )),
  cache_folder = cache_folder,
  parallel = 16
)
