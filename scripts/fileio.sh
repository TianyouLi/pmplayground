#!/bin/bash
CUR_DIR=`pwd`
SRC_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

## test driver
EXE="sudo ${SRC_DIR}/../bin/fileio_test"

## persist memory dir
PMD=/mnt/pmem

## regular storage dir
RDD=/

## run test
for mode in "${PMD}" "${RDD}"; do
		echo "testing in ${mode} ....."
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 16
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 32
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 64
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 128
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 256
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 512
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 1024
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 4096
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 8192
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 16384
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 32768
		${EXE} ${mode}/fileio_test_tmpfile 1073741824 65536
done
