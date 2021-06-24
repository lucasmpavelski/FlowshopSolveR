library(FlowshopSolveR)
library(tidyverse)
library(here)

all_problems <- all_problems_df() %>%
  filter(
    problem %in% c("flowshop"),
    budget == "high",
    dist %in% c("exponential", "uniform"),
    # corr == "random",
    no_jobs %in% c(20, 50),
    no_machines %in% c(10)
  ) %>%
  mutate(
    stopping_criterion = "FIXEDTIME",
    budget = "low"
  )

extra_problems <- all_problems_df() %>%
  filter(
    problem %in% c("flowshop"),
    budget == "high",
    dist %in% c("exponential", "uniform"),
    # corr == "random",
    no_jobs %in% c(100,200),
    no_machines %in% c(20)
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
    ),
  extra_problems %>%
    mutate(
      instances = map(instances, ~filter(.x, inst_n %in% c(6))),
      set_type = "extra"
    )
)

train_test_sets <- train_test_sets[sample(nrow(train_test_sets)),]

train_test_sets_df <- train_test_sets %>%
  mutate(
    problem_space = pmap(., as_metaopt_problem),
    path = "all"
  ) %>%
  group_nest(set_type, path) %>%
  rename(problems = data)

write_rds(train_test_sets_df, here("reports", "aos", "data", "train_test_sets_df.rds"))


