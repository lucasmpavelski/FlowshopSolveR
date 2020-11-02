#!/bin/bash

# (           Rscript reports/02-build_performance_data.R --prob vrf-large --dist vrf         --corr hard              ) &
(sleep 1 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist erlang      --corr random            ) &
(sleep 2 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist exponential --corr random            ) &
(sleep 3 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist uniform     --corr job-correlated    ) &
(sleep 4 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist uniform     --corr machine-correlated) &
wait