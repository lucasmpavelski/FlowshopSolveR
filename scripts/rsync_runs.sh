#!/bin/bash
machine=${1:-uf}
rsync -avz $machine:dev/ig-aos-flowshop/runs .
