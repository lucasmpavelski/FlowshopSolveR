abs_project_folder="$(dirname $(readlink -e .))"

docker build -t lmpavelski/aclib2:20.04 . && \
docker run --network=host \
    -v $abs_project_folder/data/:/data/ \
    -v $abs_project_folder/_install/:/_install/ \
    -v `pwd`/results:/results \
    -v `pwd`/fsp:/fsp \
    lmpavelski/aclib2:20.04
#    -it lmpavelski/aclib2:20.04 bash