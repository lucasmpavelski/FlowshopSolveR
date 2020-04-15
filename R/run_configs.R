library(here)
library(tidyverse)
library(future)

plan(multisession)


# paths
ROOT <- here()
EXPR <- file.path(ROOT, 'runs', 'aos-ig')
DATA <- file.path(ROOT, 'data')
EXECUTABLE <- file.path(ROOT, 'build', 'main', 'fsp_solver')


# datasets
problems <- read_csv(file.path(EXPR, 'problems.csv'))
configs <- read_csv(file.path(EXPR, 'configs.csv'))

# auxiliar functions
parseParams <- function(params) { 
  params <- str_split(params, ' ', simplify = T) %>%
    str_split('=', simplify = T)
  values <- as.numeric(params[,2])
  names(values) <- params[,1]
  values
}

solveCmd <- function(mh, seed, params, output, core, ...) {
  problem_data <- list(...)
  problem_model <- as.character(problem_data[colnames(problems)])
  names(problem_model) <- colnames(problems)
  params <- parseParams(params)
  exe_bin <- 
  args <- c(
    "taskset",
    "-c",
    core,
    EXECUTABLE,
    paste0('--data_folder=', DATA),
    paste0('--mh=', mh),
    paste0('--problem_names=', paste0(names(problem_model), collapse = ',')),
    paste0('--problem_values=', paste0(problem_model, collapse = ',')),
    paste0('--params_names=', paste0(names(params), collapse = ',')),
    paste0('--params_values=', paste0(params, collapse = ','))
  )
  system2(exe_bin, args, stdout = output)
}


experiments <- crossing(
  problems,
  configs,
  seed = c(123, 456, 789, 159, 753)
) %>%
  mutate(
    output = pmap_chr(., function(...) {
      expr <- list(...)
      name <- expr[['name']]
      problem_model <- as.character(expr[colnames(problems)])
      file.path(EXPR, name, paste0(paste(problem_model, collapse = '-'), '.out'))
    })
  )

experiments %>%
  write_csv(file.path(EXPR, 'experiments.csv'))

walk(file.path(EXPR, configs$name), dir.create, showWarnings = F)

ncores <- 3
core_groups <- experiments %>%
  group_split(seq_len(nrow(experiments)) %% ncores)
  
futures <- map(core_groups, function(group_expr) {
  future({
    group_expr %>%
      filter(!file.exists(output)) %>%
      mutate(status = pmap(., solveCmd))
  })
})

values(futures)


