
get_fixed <- function(parameter_space) {
  parameter_space$names[parameter_space$isFixed]
}

get_not_fixed <- function(parameter_space) {
  parameter_space$names[!parameter_space$isFixed]
}

get_discrete_not_fixed <- function(parameter_space) {
  tunnable_params <- get_not_fixed(parameter_space)
  parameter_space$names[parameter_space$names %in% tunnable_params &
                          parameter_space$types %in% c("i", "c", "o")]
}

get_categorical_not_fixed <- function(parameter_space) {
  tunnable_params <- get_not_fixed(parameter_space)
  parameter_space$names[parameter_space$names %in% tunnable_params &
                          parameter_space$types %in% c("c", "o")]
}