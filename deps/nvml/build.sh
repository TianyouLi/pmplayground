#!/bin/bash
CUR_DIR=`pwd`
BLD_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

cd ${BLD_DIR}

# download googletest from github
git clone https://github.com/TianyouLi/nvml.git

# create build dir
cd ${BLD_DIR}/nvml
make && make install prefix=${BLD_DIR}/../../lib

cd ${CUR_DIR}
