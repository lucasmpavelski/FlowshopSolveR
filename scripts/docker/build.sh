#!/bin/bash

Rscript -e "devtools::install_github('lucasmpavelski/schedulingInstancesGeneratoR')"
Rscript -e "devtools::install_github('lucasmpavelski/metaOpt')"

INSTALL_POSTFIX=docker R CMD INSTALL --preclean --no-lock .