

all_problems_df <- function() {
  crossing(
    all_instances_df(),
    models_attrs_df()
  ) %>%
    group_nest(across(!c("inst_n", "instance")), .key = "instances") %>%
    mutate(instance_features = paste(problem, dist, corr, no_jobs, no_machines, sep = ',')) %>%
    mutate(id = row_number())
}

generate_train_test_sets <- function() {
  dir.create(here("data", "instance_sets"), showWarnings = F)
  train_path <- here("data", "instance_sets", "train.csv")
  test_path <- here("data", "instance_sets", "test.csv")
  if (file.exists(here("data", "instance_sets", "train.csv"))) {
    warning("train and test files already exists!")
  } else {
    set.seed(7287287)
    problems <- all_problems_df()
    test_idx <- sample.int(nrow(problems), size = round(nrow(problems) * 0.2))
    problems[test_idx, ] %>%
      unnest(instances) %>%
      write_csv(test_path)
    problems[-test_idx, ] %>%
      unnest(instances) %>%
      write_csv(train_path)
  }
  list(
    train = train_path,
    test = test_path
  )
}

train_problems_df <- function() {
  train_path <- here("data", "instance_sets", "train.csv")
  read_csv(train_path) %>%
    group_nest(across(!c("inst_n", "instance")), .key = "instances")
}

test_problems_df <- function() {
  train_path <- here("data", "instance_sets", "test.csv")
  read_csv(train_path) %>%
    group_nest(across(!c("inst_n", "instance")), .key = "instances")
}
