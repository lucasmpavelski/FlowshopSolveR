
adjacencyDistance <- function(a, b) {
  # number of times jobs i,j is adjacent in both a and b
  n <- length(a)
  d <- 0
  for (i in 1:(n - 1)) {
    p_ai <- which(b == a[i])[[1]]
    if (p_ai < n) {
      d <- d + (b[p_ai + 1] == a[i + 1])
    }
  }
  max_dist <- n - 1
  (max_dist - d) / max_dist
}

precedenceDistance <- function(a, b) {
  # nummber of times some job j is preceded by job i in both a and b
  n <- length(a)
  d <- 0
  cache_positions <- array(0, n)
  for (i in 1:n)
    cache_positions[i] <- which(b == a[i])[[1]]
  for (i in 1:(n - 1)) {
    for (j in (i + 1):n) {
      d <- d + (cache_positions[i] < cache_positions[j])
    }
  }
  max_dist <- n * (n - 1) / 2
  (max_dist - d) / max_dist
}

absolutePositionDistance <- function(a, b) {
  # number of exact positional matches of jobs
  n <- length(a)
  d <- sum(a == b)
  max_dist <- n
  (max_dist - d) / max_dist
}

deviationDistance <- function(a, b) {
  # given the inverse permutation sigma the amount of positional deviation
  n <- length(a)
  sigma_a <- array(0, n)
  sigma_b <- array(0, n)
  sigma_a[a + 1] <- 1:n
  sigma_b[b + 1] <- 1:n
  d <- sum(abs(sigma_a - sigma_b))
  rm(sigma_a, sigma_b)
  max_dist <- if_else(n %% 2 == 0, n^2 / 2, (n^2 - 1) / 2)
  d / max_dist
}

aproximatedSwapDistance <- function(a, b) {
  # number of swap move from a to b (exact distance is non-trivial)
  n <- length(a)
  d <- 0
  while (any(a != b)) {
    for (i in 1:n) {
      if (a[i] != b[i]) {
        b_i <- which(a == b[i])[[1]]
        tmp <- a[i]
        a[i] <- a[b_i]
        a[b_i] <- tmp
        d <- d + 1
      }
    }
  } 
  max_dist <- n - 1
  d / max_dist
}

LCSLength <- function(x, y) {
  m <- length(x)
  n <- length(y)
  C <- matrix(0, m + 1, n + 1)
  C[,0] <- 0
  C[0,] <- 0
  for (i in 2:(m+1)) {
    for (j in 2:(n+1)) {
      if (x[i - 1] == y[j - 1]) {
        C[i, j] <- C[i - 1, j - 1] + 1
      } else {
        C[i, j] <- max(C[i, j - 1], C[i - 1, j])
      }
    }
  }
  ll <- C[m+1,n+1]
  rm(C)
  ll
}

shiftDistance <- function(a, b) {
  # number of insertion moves from a to b
  n <- length(a)
  d <- n - LCSLength(a, b)
  max_dist <- n - 1
  d / max_dist
}

ALL_DISTANCES_FUNCTIONS <- dplyr::lst(
  adjacencyDistance,
  precedenceDistance,
  absolutePositionDistance,
  deviationDistance,
  # aproximatedSwapDistance,
  shiftDistance
)
