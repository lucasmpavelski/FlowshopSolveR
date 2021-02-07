#!/bin/bash

# (           Rscript reports/02-build_performance_data.R --prob vrf-large --dist vrf         --corr hard              ) &
# (sleep 1 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist erlang      --corr random            ) &
# (sleep 2 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist exponential --corr random            ) &
# (sleep 3 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist uniform     --corr job-correlated    ) &
# (sleep 4 && Rscript reports/02-build_performance_data.R --prob flowshop  --dist uniform     --corr machine-correlated) &
# wait

# (sleep 0 && Rscript reports/02-build_performance_data.R --prob vrf-large --dist vrf --corr hard --no_machines 20 --cores  5) &
# (sleep 1 && Rscript reports/02-build_performance_data.R --prob vrf-large --dist vrf --corr hard --no_machines 40 --cores 11) &
# (sleep 2 && Rscript reports/02-build_performance_data.R --prob vrf-large --dist vrf --corr hard --no_machines 60 --cores 18) &
# (sleep 3 && Rscript reports/03-build-features-data.R) & 
# wait

# (sleep 0 && Rscript reports/02-best-solver.R) &
# (sleep 1 && Rscript reports/03-build-features-data.R) &
# wait

R_PROGRESSR_ENABLE=TRUE Rscript reports/lons_study/sample_lons.R --config_id 9