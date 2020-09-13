
all_objectives <- function() {
  c('FLOWTIME', 'MAKESPAN')
}

all_types <- function() {
  c('PERM', 'NOWAIT', 'NOIDLE')
}

all_stopping_criteria <- function() {
  c('EVALS', 'TIME', 'FIXEDTIME')
}

all_budgets <- function() {
  c('low', 'med', 'high')
}

fixed_time <- function(multiplier) {
  crossing(
    objective = all_objectives(),
    type = all_types(),
    stopping_criterion = paste0('FIXEDTIME_', multiplier),
    budget = all_budgets()[1]
  ) %>%
    mutate(model = str_c(objective, type, stopping_criterion, budget, sep = '_'))
}

all_problem_data <- function() {
  crossing(
    objective = all_objectives(),
    type = all_types(),
    stopping_criterion = all_stopping_criteria(),
    budget = all_budgets()
  ) %>%
    mutate(model = str_c(objective, type, stopping_criterion, budget, sep = '_'))
}
