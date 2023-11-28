#!/usr/bin/env bash
set -ex

SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
platform=""
output="./deps.tar.gz"

function task_print_usage() {
    echo "Usage: generate_deps.sh -p [platform]"
    echo "Example: generate_deps.sh -p xcode"
    echo ""
    echo ""
    echo "------------------------------------------------------------------------------"
    echo "Platform:"
    echo "  choose a platform with -p | --platform <target>   , e.g -p xcode."
    echo "      ndk                   Build deps for ndk environment"
    echo "      xcode                 Build deps from xcode environment"
    echo ""
    echo ""
    "------------------------------------------------------------------------------"
    echo "Output:"
    echo "  choose an output path where deps tar.gz will be generated -o | --output <output>   , e.g -o ./my-deps.tar.gz"
    echo ""
    echo ""
}

function generate_deps() {
    # do task by platform
    echo "Generate DEPS folder for platform $platform"
    rm -rf tmp && mkdir tmp
    cd tmp
    mkdir omvll-deps
    # Generate common elements
    #${SCRIPT_PATH}/common/compile_cpython310.sh
    #${SCRIPT_PATH}/common/compile_pybind11.sh
    #${SCRIPT_PATH}/common/compile_spdlog.sh
    if [ "$platform" == "ndk" ]; then
        ${SCRIPT_PATH}/ndk/compile_llvm_r25.sh
    elif [ "$platform" == "xcode" ]; then
        ${SCRIPT_PATH}/xcode/compile_llvm_xcode15_1.sh
    fi
    # Generate tar
    tar -czvf deps.tar.gz omvll-deps
    cd ..
    mv tmp/deps.tar.gz $output
    rm -rf tmp
}

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h | --help )   task_print_usage
                    exit 0
                    ;;
    -p | --platform)
                    case $2 in
                        ndk | xcode ) if [ "$platform" != "" ]; then
                                            task_print_usage
                                            exit 1
                                        fi
                                        platform="$2"
                        ;;
                    esac
                    shift # past argument
                    shift # past value
                    ;;
    -o | --output)
                    output="$2"
                    shift # past argument
                    shift # past value
                    ;;
    *)    # unknown option
    shift # past argument
    ;;
esac
done

if [ $platform != "android" ] && [ $platform  != "xcode" ]; then
   echo "Please specify a valid --platform"
   echo "For more information use --help"
   exit 1
fi

generate_deps
rc=$?
return rc
