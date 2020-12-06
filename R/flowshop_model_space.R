
MODELS_ATTRS <- list(
  objective = c("FLOWTIME", "MAKESPAN"),
  type = c("PERM", "NOWAIT", "NOIDLE"),
  stopping_criterium = c("TIME"),
  budget = c("low", "high")
)

models_attrs_df <- function(attrs = MODELS_ATTRS) {
  tidyr::crossing(
    objective = attrs$objective,
    type = attrs$type,
    stopping_criterion = attrs$stopping_criterium,
    budget = attrs$budget
  ) %>%
    mutate(model = str_c(objective, type, stopping_criterion, budget, sep = ","))
}

fixed_time_attrs_df <- function(multiplier) {
  attrs <- MODELS_ATTRS
  attrs$stopping_criterium <- paste0("FIXEDTIME_", multiplier)
  models_attrs_df(attrs)
}
