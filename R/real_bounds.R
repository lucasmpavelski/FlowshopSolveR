
real_lower_bound <- function(domain, type) {
  switch (
    type,
    "c" = 1,
    "r" = domain[1],
    "i" = domain[1]
  )
}

real_lower_bounds <- function(parameter_space, fixed = FALSE) {
  tunnable_params <- parameter_space$names[!parameter_space$isFixed | fixed]
  map2_dbl(parameter_space$domain[tunnable_params],
           parameter_space$type[tunnable_params],
           real_lower_bound)
}

real_upper_bound <- function(domain, type) {
  switch (
    type,
    "c" = length(domain) + 1 - 1e-9,
    "r" = domain[2],
    "i" = domain[2] + 1 - 1e-9
  )
}

real_upper_bounds <- function(parameter_space, fixed = FALSE) {  
  tunnable_params <- parameter_space$names[!parameter_space$isFixed | fixed]
  tunnable_params <- parameter_space$names[!parameter_space$isFixed]
  map2_dbl(parameter_space$domain[tunnable_params],
           parameter_space$type[tunnable_params],
           real_upper_bound)
}