#!/bin/bash
CUR_DIR=`pwd`
BLD_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )


cd ${BLD_DIR}/nvml

make clean && make clobber


cd ${CUR_DIR}
