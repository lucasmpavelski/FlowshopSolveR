features_file_path <- function(problem, instance, features_folder) {
  file.path(
    features_folder,
    paste0(
      problem@name,
      ifelse(is.null(instance), "", paste0(",", instance)),
      ".rds"
    )
  )
}

load_problem_features <- function(problem, instance = NULL,
                                  features_folder = here("data", "features")) {
  path <- features_file_path(problem, instance, features_folder)
  if (file.exists(path)) {
    readRDS(path)
  } else {
    tibble(
      name = problem@name,
      instance = instance
    )
  }
}

save_problem_features <- function(problem, instance = NULL, features,
                                  features_folder = here("data", "features")) {
  path <- features_file_path(problem, instance, features_folder)
  saveRDS(features, file = path)
}

append_problem_features <- function(new_features, problem, instance = NULL,
                                    features_folder = here("data", "features")) {
  features <- load_problem_features(problem, instance, features_folder)
  features[, names(new_features)] <- new_features[, names(new_features)]
  save_problem_features(problem, instance, features)
  features
}
