
# TODO: add validations: at least one instance
Problem <- setClass(
  "Problem",
  slots = c(
    name = "character",
    instances = "list",
    data = "list"
  ),
  prototype = list(
    name = "not available",
    data = list()
  )
)

# TODO: add validations: problems and instances names must be unique
ProblemSpace <- setClass("ProblemSpace",
  slots = c(problems = "list"), prototype = list(problems = list()),
  validity = function(object) {
    all(object@problems %>% map(class) == "Problem")
  }
)
