FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential openjdk-11-jdk git python3-pip python3-dev r-base ruby swig3.0 && \
    apt-get clean && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/swig3.0 /usr/bin/swig && \
    git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/automl/GenericWrapper4AC && \
    cd GenericWrapper4AC/ && \
    python3 setup.py install && \
    cd .. && \
    git clone --depth 1 --recurse-submodules --shallow-submodules https://bitbucket.org/mlindauer/aclib2.git && \
    cd aclib2 && \
    pip3 install pyrfr && \
    pip3 install cython && \
    # ConfigSpace dependency issue: https://github.com/automl/ConfigSpace/issues/130
    pip3 install pybind11 && \
    pip3 install -r requirements.txt && \
    cd configurators/ && \
    make install

ENV JAVA_HOME /usr/lib/jvm/java-11-openjdk-amd64/
RUN export JAVA_HOME

RUN cd /aclib2/configurators/smac3/ && \
    pip3 install -r requirements.txt && \
    python3 setup.py install && \
    chmod +x /aclib2/configurators/smac3/scripts/smac

ADD fsp /aclib2/
COPY run_experiments.sh /
CMD [ "./run_experiments.sh" ]
