library(here)
library(tidyverse)
library(irace)
library(future)

plan(sequential)
# plan(multisession)


EXPERIMENTS <- c(
  'irace-aos'
)

# paths
ROOT <- here()
EXPR <- file.path(ROOT, 'runs', EXPERIMENTS[1])
DATA <- EXPR
EXECUTABLE <- file.path(ROOT, 'build', 'main', 'fsp_solver')
N_CORES <- 1
OPTIONS <- ''

# datasets
problems <- read_csv(file.path(EXPR, 'problems.csv'), comment = "#")

# auxiliar functions
parseParams <- function(params) { 
  params <- str_split(params, '\\s', simplify = T) %>%
    str_split('=', simplify = T)
  values <- params[,2]
  names(values) <- params[,1]
  values
}

solveCmd <- function(problem, config, seed, core) {
  exe_bin <- "taskset"
  args <- c(
    "-c",
    core,
    EXECUTABLE,
    paste0('--data_folder=/home/lucasmp/dev/ig-aos-flowshop/data'),
    paste0('--mh=IG'),
    paste0('--seed=', seed),
    paste0("--", names(problem), "=", problem),
    paste0("--", names(config), "=", config),
    '--printBestFitness'
  )
  #print(paste0(c('START', exe_bin, args), collapse = ' '))
  data <- system2(exe_bin, args, stdout = TRUE)
  as.integer(str_split(last(data), ',', simplify = T)[,2])
}

fspTargetRunnerCmdSequential <- function(experiments_dt) {
  experiments_dt %>%
    pmap(., solveCmd)
}


fspTargetRunnerCmdParallel <- function(experiments, scenario, config, ...) {
  problems <- scenario$targetRunnerData
  expetiments_dt <- tibble(
    problem = map(experiments, ~problems[.x$instance,]),
    config = map(experiments, ~.x$config),
    seed = map_int(experiments, ~.x$seed),
    core = rep(1:N_CORES, length.out = length(experiments))
  )
  experiments_futures <- expetiments_dt %>%
    group_split(core) %>%
    map(function(exp_dt) {
      futureCall(fspTargetRunnerCmdSequential, 
                 args = list(experiments_dt = exp_dt))
    })
  experiments_futures %>%
    map(value) %>%
    unlist() %>%
    map(~list(time = 0, cost = .x))
}

MH <- 'IG'
specs <- file.path(DATA, "specs", paste0(MH, '.txt'))
parameters <- readParameters(specs)
print(parameters)
max_config_eval <- 5000
scenario <- defaultScenario(list(
  targetRunnerParallel = fspTargetRunnerCmdParallel,
  maxExperiments = max_config_eval,
  instances = 1:nrow(problems),
  targetRunnerData = problems,
  #logFile = file.path(results_fdr, logNameForConfig(config)),
  parallel = 1#,
  #testType = "T-test"
))
    # write(inst_name, stderr())
    # write(inst_name, stdout())
    irace.result <- irace(scenario = scenario, parameters = parameters)
#    save(irace.result, file = file.path(results_fdr, result_name))



plan(sequential)