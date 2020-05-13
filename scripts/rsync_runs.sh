#!/bin/bash
machine=${1:-uf}
rsync -avz runs $machine:dev/ig-aos-flowshop/

