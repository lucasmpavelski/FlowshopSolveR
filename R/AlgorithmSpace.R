Algorithm <- setClass("Algorithm", slots = list(name = "character", parameters = "list"), prototype = list(
  name = "not available",
  parameters = list()
))


AlgorithmSpace <- setClass("AlgorithmSpace",
  slots = list(algorithms = "list"), prototype = list(algorithms = list()),
  validity = function(object) {
    object@algorithms %>%
      purrr::map(class) %>%
      purrr::every(~ .x == "Algorithm")
  }
)
