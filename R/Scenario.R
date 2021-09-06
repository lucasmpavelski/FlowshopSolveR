
Scenario <- setClass(
  "Scenario",
  slots = c(
    problem_space = "ProblemSpace",
    algorithm_space = "AlgorithmSpace",

    only_best = "logical",
    ablation = "logical",

    folder = "character"
  ),
  prototype = list(
    problem_space = ProblemSpace(),
    algorithm_space = AlgorithmSpace(),

    only_best = TRUE,
    ablation = FALSE,

    folder = "data"
  ),

)

setGeneric("performance_folder", function(x) standardGeneric("performance_folder"))
setMethod(
  "performance_folder",
  "Scenario",
  function(x) {
    file.path(x@folder, "performances")
  }
)

setGeneric("features_folder", function(x) standardGeneric("features_folder"))
setMethod(
  "features_folder",
  "Scenario",
  function(x) {
    file.path(x@folder, "features")
  }
)

setGeneric("models_folder", function(x) standardGeneric("models_folder"))
setMethod(
  "models_folder",
  "Scenario",
  function(x) {
    file.path(x@folder, "models")
  }
)

setGeneric("instances_folder", function(x) standardGeneric("instances_folder"))
setMethod(
  "instances_folder",
  "Scenario",
  function(x) {
    file.path(x@folder, "instances")
  }
)

setGeneric("instances_sets_folder", function(x) standardGeneric("instances_sets_folder"))
setMethod(
  "instances_sets_folder",
  "Scenario",
  function(x) {
    file.path(x@folder, "instances_sets")
  }
)

setGeneric("performance_data", function(x) standardGeneric("performance_data"))
setMethod(
  "performance_data",
  "Scenario",
  function(x) {
    expand.grid(
      problem = x@problem_space@problems,
      algorithm = x@algorithm_space@algorithms
    ) %>%
      dplyr::mutate(results = map2(
        problem,
        algorithm,
        function(problem, algorithm) {
          folder <- file.path(performance_folder(x), problem@name, algorithm@name)
          if (x@ablation) {
            readRDS(file.path(folder, 'ablation_best.rds'))
          } else {
            results <- readRDS(file.path(folder, 'result.rds'))
            ifelse(x@only_best && !x@ablation, results[1, ], results)
          }
        }
      )) %>%
      unnest(cols = c(results))
  }
)
