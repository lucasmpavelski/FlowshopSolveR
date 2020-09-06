library(here)
library(tidyverse)
library(future)

plan(sequential)
plan(multisession)


EXPERIMENTS <- c(
  'rs-ig-destruction-sizes',
  'compare-reward-types',
  'reward-distributions',
  'best-perturb-fitness',
  'best-perturb-destruction-size',
  'aos-tunning',
  'aos-tunning-final-test',
  'aos-tunning-generarization',
  'aos-148-final-comparison',
  'ig-lsps',
  'ig-lsps-aos-tunning-10000evals-generalization'#'ig-lsps-aos-tunning-10000evals-final'
)


# paths
ROOT <- here()
EXPR <- file.path(ROOT, 'runs', EXPERIMENTS[11])
DATA <- file.path(ROOT, 'data')
EXECUTABLE <- file.path(ROOT, 'build', 'main', 'fsp_solver')
OPTIONS <- read_lines(file.path(EXPR, 'params.txt'))

# datasets
problems <- read_csv(file.path(EXPR, 'problems.csv'), comment = "#")
configs <- read_csv(file.path(EXPR, 'configs.csv'), comment = "#")

ncores <- 7

# auxiliar functions
parseParams <- function(params) { 
  params <- str_split(params, '\\s', simplify = T) %>%
    str_split('=', simplify = T)
  values <- params[,2]
  names(values) <- params[,1]
  values
}

solveCmd <- function(mh, seed, params, output, core, ...) {
  problem_data <- list(...)
  problem_model <- as.character(problem_data[colnames(problems)])
  names(problem_model) <- colnames(problems)
  params <- parseParams(params)
  exe_bin <- "taskset"
  args <- c(
    "-c",
    core,
    EXECUTABLE,
    paste0('--data_folder=', DATA),
    paste0('--mh=', mh),
    paste0('--seed=', seed),
    paste0("--", names(problem_model), "=", problem_model),
    paste0("--", names(params), "=", params),
    OPTIONS
  )
  #print(paste0(c('START', exe_bin, args), collapse = ' '))
  data <- system2(exe_bin, args, stdout = TRUE)
  print(paste0(c('DONE', exe_bin, args), collapse = ' '))
  write_lines(data, output)
}

set.seed(31415)
seeds <- c(
  123, # for easy debugging
  as.integer(runif(4) * 1e6)
)

experiments <- crossing(
  problems,
  configs,
  seed = seeds
) %>%
  mutate(
    output = pmap_chr(., function(...) {
      expr <- list(...)
      name <- expr[['name']]
      problem_model <- as.character(expr[colnames(problems)])
      file.path(EXPR, name, paste0(paste(problem_model, collapse = '-'), '-', expr[['seed']], '.out'))
    })
  )

experiments %>%
  write_csv(file.path(EXPR, 'experiments.csv'))

walk(file.path(EXPR, configs$name), dir.create, showWarnings = F)


experiments <- experiments %>%
  mutate(core = seq_len(nrow(experiments)) %% ncores) 

futures <- experiments %>%
  group_split(core) %>%
  map(function(group_expr) {
   future({
      group_expr %>%
        filter(!file.exists(output)) %>%
        mutate(status = pmap(., solveCmd))
    })
  })

values(futures)


plan(sequential)