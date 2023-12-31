#!/bin/bash
# build.sh

# check compile needs of received source file
# return true or false
is_need_compile() {
    local source_file="${1:?"source file not found"}"
    local object_file="${source_file/'.c'/'.o'}"
    local retval=false

    # need compile if object file not found or object file is older than source file
    if [[ ! -e "${object_file}" ]] || [[ $(date +"%s" -r "${object_file}") < $(date +"%s" -r "${source_file}") ]]; then
        retval=true
    fi

    echo ${retval};
}


# build one source
# build stale libs
# return 0 if built
# return 1 if no need to build
# return 255 or other if compile error occured
unit_build() {
    if [[ $# == 2 ]]; then
        local force_flag="${1}"
        shift
    fi
    local MAIN="${1:?"build source undefined"}"
    local SRCS="${MAIN} $(find ${g_LIBDIR}/ -type f -name *.c) "
    local OBJS="${SRCS//'.c '/'.o '}"
    local _TARGET="${g_BINDIR}/${MAIN##*/}"
    local TARGET="${_TARGET%.c}"
    local CC=gcc
    local OPTIMIZATION=-O3
    local DEBUGOPT=""
    if [[ ${DEBUG} == 1 ]]; then
        OPTIMIZATION=-O0
        DEBUGOPT=-g
    fi
    local CFLAGS="${DEBUGOPT} -std=c11 -D_POSIX_C_SOURCE=200809L -fstack-protector -lm ${USERFLAG} -I ${g_HEADERDIR} ${OPTIMIZATION} -Wall -Wextra -Werror"

    # ensure dirs exists
    mkdir -p "${g_HEADERDIR}" "${g_SOURCEDIR}" "${g_LIBDIR}" "${g_BINDIR}"

    # compile object files in parallel
    echo "${SRCS// /$'\n'}" | xargs -i{} -P$(nproc) \
        bash -c 'source build.sh # read "is_need_compile" function
                if $(is_need_compile {}); then
                    echo '"${CC}"' '"${CFLAGS}"' -o $(sed "s/^\(.*\).c$/\1.o/" <<< "{}") -c {}
                         '"${CC}"' '"${CFLAGS}"' -o $(sed "s/^\(.*\).c$/\1.o/" <<< "{}") -c {} 2>&1 # stdout compile error msg
                    [[ $? == 0 ]] && exit 1 || exit 255
                fi
                exit 0' 2>/dev/null # throw away bash exitcode message like `exited with status 255; aborting`
    # rc is 0 if there was no compile.
    # rc is 123 if there was compile at least once.
    # rc is 124 if there was compile error at least once.
    local xargs_rc=$?

    # create binary
    # Now, object file is created so obj is later than src.
    # do nothing if force_flag is undefined and there was no obj compile.
    if [[ ! -v force_flag ]] && [[ ${xargs_rc} == 0 ]]; then
        return 1
    fi
    # compile error
    if [[ ${xargs_rc} == 124 ]]; then
        return 255
    fi
    echo ${CC} ${SRCS} ${CFLAGS} -o ${TARGET}
         ${CC} ${SRCS} ${CFLAGS} -o ${TARGET}
    local ld_rc=$?
    # compile (ld) error
    if [[ ${ld_rc} != 0 ]]; then
        return 255;
    fi
    return 0;
}

# return 0 if succeed (including no compile occured)
# return 1 if compile error
# return other (xargs exitcode) is unexpected
subcmd_all() {
    # check compile necessity in parallel
    echo "$(find ${g_LIBDIR}/ -type f -name *.c)" | xargs -i{} -P$(nproc) \
        bash -c 'source build.sh # read "is_need_compile" function
                if $(is_need_compile {}); then
                    exit 1
                fi
                exit 0'
    # rc is 0 if there is no need to compile.
    # rc is 123 if there is need to compile.
    local xargs_rc=$?
    if [[ ${xargs_rc} == 123 ]]; then
        local force_flag="-f"
    fi

    local MAIN_SRCS="$(find ${g_SOURCEDIR}/ -maxdepth 1 -type f -name *.c)"

    echo "${MAIN_SRCS// /$'\n'}" | xargs -i{} -P$(nproc) \
        bash -c 'source ./build.sh '"${OPT_ALL}"'
                unit_build '"${force_flag}"' {} 2>&1 ; unit_build_rc=$?
                [[ ${unit_build_rc} == 0 ]] && exit 1 # succeed
                [[ ${unit_build_rc} == 1 ]] && exit 0 # no build
                exit ${unit_build_rc} # compile error' 2>/dev/null # throw away bash exitcode message like `exited with status 255; aborting`
    # xargs tells exitcode 123 when there was non zero exit in the xargs scrpt, after all xargs script done.
    # and exitcode 0 is only when all xargs script exit with 0.
    local xargs_rc=$?

    case ${xargs_rc} in
        0)
            # there all failed to compile.
            echo "Nothing to build." && return 0
        ;;
        123)
            # succees to compile at least once.
            return 0
        ;;
        124)
            # compile error
            return 1
        ;;
        *)
            # unexpected
            return ${xargs_rc}
        ;;
    esac
}

# arg: path to testfile
# return 0 if succeed
# return 1 or other if compile error or test fail
unit_test() {
    local MAIN_TEST="${1:?"test source not found"}"
    local _TESTBIN="${MAIN_TEST##*/}"
    local TESTBIN="${g_TESTDIR}/${_TESTBIN%.cc}.out"
    local CC=g++
    local CFLAGS="-g -pthread -lgtest -lgtest_main -I ${g_HEADERDIR} -std=c++20 -lm ${USERFLAG}"
    local LIBSRCS="$(find ${g_LIBDIR}/ -type f -name *.c)"

    mkdir -p "${g_TESTDIR}"

    # compile in parallel
    # echo "${TESTS// /$'\n'}" | xargs -i{} -P$(nproc) \
    #     bash -c '
    #                 '"${CC}"' '"${CFLAGS}"' -o "$(sed "s%^.*/\(.*\)\.cc$%'"${g_TESTDIR}"'/\1.out%g" <<< {})" {} '"${LIBSRCS//$'\n'/ }"'
    #             '

    # compile
    ${CC} ${MAIN_TEST} ${LIBSRCS} ${CFLAGS} -o ${TESTBIN}

    local rc=$?

    do_test() {
        ./${TESTBIN}
    }

    [[ ${rc} == 0 ]] && do_test ; return $?
    return ${rc}
}

# return 0 if succeed
# return 1 if compile error
# return 123 or other if unexpected error
subcmd_test_all() {
    local TESTS="$(find -type f -name *_test.cc)"

    mkdir -p "${g_TESTDIR}"

    # test in iterate
    echo "${TESTS// /$'\n'}" | xargs -i{} \
        bash -c '
                    source ./build.sh '"${OPT_ALL}"'
                    unit_test {} 2>&1 ; unit_test_rc=$?
                    exit ${unit_test_rc}
                ' 2>/dev/null
    local xargs_rc=$?

    case ${xargs_rc} in
            0)
                # all passed
                echo "PASSED all test."
                return 0
            ;;
            123)
                # compile error or test fail or other error occured
                return 1
            ;;
            *)
                # unexpected xargs error
                return ${xargs_rc}
            ;;
    esac
}


subcmd_clean() {
    echo rm -rf $(find -type f -name "*.o") ${g_BINDIR} ${g_TESTDIR}/*.out
         rm -rf $(find -type f -name "*.o") ${g_BINDIR} ${g_TESTDIR}/*.out
}

subcmd_format() {
    local TARGETS="${@}"
    if [[ "${TARGETS}" == "" ]]; then
        TARGETS="$(find -type f \( -name "*.h" -o -name "*.c" -o -name "*.cc" \))"
    fi
    clang-format --style=Google -i ${TARGETS//$'\n'/ }
}


main() {
    local subcmd="${1:-all}"
    shift
    case "${subcmd}" in
        "all")
            subcmd_all
            local rc=$?
            exit ${rc}
        ;;
        "test")
            subcmd_test_all
            local rc=$?
            exit ${rc}
        ;;
        "clean")
            subcmd_clean
            exit 0
        ;;
        "format")
            subcmd_format "${@}"
            exit 0
        ;;
        "help")
            echo "Usage:"
            echo -e "\tbuild.sh [options] [subcmd/target_source]"
            echo -e ""
            echo -e "\toption:"
            echo -e "\t\t-d\tEnable debug mode"
            echo -e "\t\t-D<NAME>\tAdd compile option"
            echo -e "\t\t-h\tShow this text"
            echo -e ""
            echo -e "\tsubcmd:"
            echo -e "\t\tall\tBuild all (default)"
            echo -e "\t\ttest\tRun test"
            echo -e "\t\tclean\tRemove bin and obj files"
            echo -e "\t\tformat [files..]\tFormat source files"
            echo -e "\t\thelp\tShow this text"
            echo -e "\t\t\`target_source\`\tPath to source file to build"
            echo ""
            echo "Note: Do nothing if called by any script excepting shell."
            exit 0
        ;;
        *)
            local target_src="${subcmd}"
            if [[ -e ${target_src} ]]; then
                unit_build "${target_src}"
                local rc=$?
                case ${rc} in
                    1)
                        echo "Nothing to build."
                        exit 0
                    ;;
                    255)
                        exit 1
                    ;;
                    *)
                        exit ${rc}
                    ;;
                esac
            else
                echo "build source not found."
                echo "To show command help, run \"build.sh help\"."
                exit 1
            fi
        ;;
    esac
}


readonly g_HEADERDIR=include
readonly g_SOURCEDIR=src
# readonly g_LIBDIR=${g_SOURCEDIR}/lib
readonly g_LIBDIR=lib
readonly g_BINDIR=bin
readonly g_TESTDIR=test

OPT_ALL=""
DEBUG=0
USERFLAG=""
while getopts dD:h OPT
do
case "${OPT}" in
    d)
        DEBUG=1
        OPT_ALL+="-d "
    ;;
    D)
        USERFLAG+="-D${OPTARG} "
        OPT_ALL+="-D${OPTARG} "
    ;;
    h)
        main help
    ;;
esac
done

# remove flag args
readonly FLAGARGPATTERN='^-.*'
while [[ "${1}" =~ ${FLAGARGPATTERN} ]]; do
shift
done

# soruce guard
# run main when called by shell
if [[ ${0##*/} == "build.sh" ]]; then
    main "${@}"
else
    :
fi
