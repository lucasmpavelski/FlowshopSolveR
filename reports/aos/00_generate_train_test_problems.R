library(FlowshopSolveR)
library(tidyverse)
library(here)

all_problems <- all_problems_df() %>%
  filter(
    problem %in% c("flowshop"),
    budget == "high",
    dist == "uniform",
    corr == "random",
    no_jobs %in% c(100, 200, 300)
  ) %>%
  mutate(
    stopping_criterion = "FIXEDTIME",
    budget = "low"
  )


train_test_sets <- bind_rows(
  all_problems %>%
    mutate(
      instances = map(instances, ~filter(.x, inst_n %in% c(1))),
      set_type = "train"
    ),
  all_problems %>%
    mutate(
      instances = map(instances, ~filter(.x, inst_n %in% c(6))),
      set_type = "test"
    )
)

train_test_sets <- train_test_sets[sample(nrow(train_test_sets)),]

train_test_sets_df <- train_test_sets %>%
  mutate(
    problem_space = pmap(., as_metaopt_problem),
    path = file.path(type, objective)
  ) %>%
  select(objective, type, set_type, path, problem_space) %>%
  group_nest(objective, type, set_type, path) %>%
  rename(problems = data)

write_rds(train_test_sets_df, here("reports", "aos", "data", "train_test_sets_df.rds"))


