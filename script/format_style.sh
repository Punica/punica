#!/bin/bash

set -e

TMP_BUILD_DIR="$(mktemp -d)"
PROJECT_ROOT_DIR="$(cd $(dirname "$0")/.. && pwd -P)"

ASTYLE_EXEC_DIR="${PROJECT_ROOT_DIR}/script/astyle"
ASTYLE="${ASTYLE_EXEC_DIR}/astyle"
ASTYLE_OPTS="--options=${PROJECT_ROOT_DIR}/.astyle.conf $@"
ASTYLE_FILES="$(find ${PROJECT_ROOT_DIR}/{src,include} -regextype posix-basic -regex '^.*\.[ch]\(pp\)\{0,1\}$' | xargs echo)"
ASTYLE_URL="https://sourceforge.net/projects/astyle/files/astyle/astyle%203.1/astyle_3.1_linux.tar.gz"
ASTYLE_ARCHIVE="${TMP_BUILD_DIR}/astyle.tar.gz"
ASTYLE_DIR="${TMP_BUILD_DIR}/astyle"

get_astyle () {
    eval "wget ${ASTYLE_URL} -O ${ASTYLE_ARCHIVE}"
    eval "tar xzvf ${ASTYLE_ARCHIVE} --directory ${TMP_BUILD_DIR}/"
    eval "cd ${ASTYLE_DIR}/build/gcc"
    make
    eval "mv ${ASTYLE_DIR}/build/gcc/bin/astyle ${ASTYLE}"
    cd -
}

if [ ! -d "${ASTYLE_EXEC_DIR}" ]; then
    eval "mkdir ${ASTYLE_EXEC_DIR}"
fi

if [ ! -x ${ASTYLE} ]; then
    get_astyle || (rm -rf ${TMP_BUILD_DIR}; exit 1)
fi
rm -rf ${TMP_BUILD_DIR}

version_output=$(eval "${ASTYLE} --version")
if [ "${version_output}" != "Artistic Style Version 3.1" ]; then
    echo "Error! Astyle version must be 3.1!"
    exit 1
fi

style_check_output=`eval "${ASTYLE} ${ASTYLE_OPTS} ${ASTYLE_FILES}"`
if [ -z "${style_check_output}" ]; then
    echo "Code style pass!"
else
    echo "Code style fail!"
    exit 1
fi
