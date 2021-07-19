Rscript reports/problem-partition/problem-partition.R -e type-medium-lin > data/problem_partition/type-medium-lin/log.out 2> data/problem_partition/type-medium-lin/log.err & \
  (sleep 1 && \
    mkdir data/problem_partition/type-medium-lin-ga && \
    Rscript reports/problem-partition/problem-partition.R -e type-medium-lin-ga > data/problem_partition/type-medium-lin-ga/log.out 2> data/problem_partition/type-medium-lin-ga/log.err) & \
  (sleep 2 && Rscript reports/problem-partition/5-type-irace.R > data/problem_partition/type-irace-single-log.out 2> data/problem_partition/type-irace-single-log.err) & \
