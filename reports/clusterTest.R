library("future")
library("listenv")

## Set up cluster nodes on login node
nodes %<-% { .keepme <- parallel::makeCluster(c("hex1")) }

## Specify future topology
## login node -> { cluster nodes } -> { multiple cores }
plan(list(
  tweak(cluster, workers = nodes)
))


## (a) This will be evaluated on the cluster login computer
x %<-% {
  thost <- Sys.info()[["nodename"]]
  tpid <- Sys.getpid()
  y <- listenv()
  for (task in 1:4) {
    ## (b) This will be evaluated on a compute node on the cluster
    y[[task]] %<-% {
      mhost <- Sys.info()[["nodename"]]
      mpid <- Sys.getpid()
      z <- listenv()
      for (jj in 1:2) {
        ## (c) These will be evaluated in separate processes on the same compute node
        z[[jj]] %<-% data.frame(task = task,
                                top.host = thost, top.pid = tpid,
                                mid.host = mhost, mid.pid = mpid,
                                host = Sys.info()[["nodename"]],
                                pid = Sys.getpid())
      }
      Reduce(rbind, z)
    }
  }
  Reduce(rbind, y)
}

print(x)