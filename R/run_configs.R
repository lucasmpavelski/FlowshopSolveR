library(here)
library(tidyverse)

# paths
ROOT <- here()
EXPR <- file.path(ROOT, 'runs', 'vanilla-ig')
DATA <- file.path(ROOT, 'data')
EXECUTABLE <- file.path(ROOT, 'build', 'main', 'fsp_solver')

# auxiliar functions
parseParams <- function(params) { 
  params <- str_split(params, ' ', simplify = T) %>%
    str_split('=', simplify = T)
  values <- as.numeric(params[,2])
  names(values) <- params[,1]
  values
}

solveCmd <- function(mh, seed, problem_model, params, output, ...) {
  exe_bin <- EXECUTABLE
  args <- c(
    paste0('--data_folder=', DATA),
    paste0('--mh=', mh),
    paste0('--problem_names=', paste0(names(problem_model), collapse = ',')),
    paste0('--problem_values=', paste0(problem_model, collapse = ',')),
    paste0('--params_names=', paste0(names(params), collapse = ',')),
    paste0('--params_values=', paste0(params, collapse = ','))
  )
  system2(exe_bin, args, stdout = output)
}

# datasets
problems <- read_csv(file.path(EXPR, 'problems.csv')) %>%
  mutate(problem_model = pmap(., compose(unlist, list))) %>%
  select(problem_model)

configs <- read_csv(file.path(EXPR, 'configs.csv'))  %>%
  mutate(params = map(params, parseParams))

experiments <- crossing(
    problems,
    configs,
    seed = 123
  ) %>%
  mutate(
    output = map2_chr(name, problem_model, function(x, y) {
      file.path(EXPR, x, paste0(paste(y, collapse = '-'), '.out'))
    })
  )

# write_csv(experiments, file.path(EXPR, 'experiments.csv'))

walk(file.path(EXPR, configs$name), dir.create, showWarnings = F)

experiments %>%
  filter(!file.exists(output)) %>%
  mutate(status = pmap(., solveCmd))
