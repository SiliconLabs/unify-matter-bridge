#!/bin/bash

function usage() {
	echo "Build ninja build for target"
	echo ""
	echo "Usage: $0 [options] target"
	echo "	options:"
	echo "		-t: run unittests"
	echo "		-c: include code coverage"
	echo "		-s: include sonarqube"
	echo "		-o [options]: gn args"
	echo "		-d [dest]: override destination dir to dest"
	echo "	target: [arm64, armhf, amd64]"
}

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
SOURCE_DIR="$(realpath "${SCRIPT_DIR}/../../linux")" # !!!Remember to update if this script is moved somewhere else!!!

unittest=0
sonarqube=0
coverage=0
gn_args=""
dest=

while getopts "ptmcszo:d:" opt; do
	case ${opt} in
	t)
		unittest=1
		;;
	c)
		coverage=1
		unittest=1
		gn_args="${gn_args} use_coverage=true"
		;;
	s)
		sonarqube=1
		;;
	d)
		dest=$OPTARG
		;;
	o)
		gn_args="${gn_args} $OPTARG"
		;;
	\?)
		usage
		exit 1
		;;
	esac
done

# Shift away the options
shift $((OPTIND - 1))

target=$1

if [ -z "${dest}" ]; then
	dest="out/build_${target}"
fi

build_cmd="echo 'Starting Unify build for ${target}...'"
build_cmd="${build_cmd} && echo 'Destination Folder: ${dest}'"
build_cmd="${build_cmd} && echo 'gn args: ${gn_args}'"

# Set cmake toolchain
if [ "${target}" = "arm64" ]; then
	build_cmd="${build_cmd} && echo 'Setting target_cpu for ${target}'"
	gn_args="target_cpu="arm64" ${gn_args}"
fi

if [ "${target}" = "armhf" ]; then
	build_cmd="${build_cmd} && echo 'Setting target_cpu for ${target}'"
	gn_args="target_cpu="arm" ${gn_args}"
fi

if [ "${target}" = "amd64" ]; then
	build_cmd="${build_cmd} && echo 'Setting target_cpu for ${target}'"
fi
#set up Matter development environment
build_cmd="${build_cmd} && pushd ${SOURCE_DIR}/third_party/connectedhomeip"
build_cmd="${build_cmd} && git config --global --add safe.directory /unify-matter-bridge/linux/third_party/connectedhomeip"
build_cmd="${build_cmd} && git config --global --add safe.directory /unify-matter-bridge/linux/third_party/connectedhomeip/third_party/pigweed/repo"
build_cmd="${build_cmd} && source ./scripts/activate.sh"

### Build command
# Sonarqube stuff
if [ ${sonarqube} -eq 1 ]; then
	build_cmd="${build_cmd} && curl -L https://sonarqube.silabs.net/static/cpp/build-wrapper-linux-x86.zip --output /tmp/sonarqube_build_wrapper.zip && ls /tmp && unzip -o -e /tmp/sonarqube_build_wrapper.zip -d /tmp/"
	build_cmd="${build_cmd} && build_wrapper='/tmp/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir ${dest}/build_wrapper_output_directory '"
fi

# No reason to create and build mocks and unit tests when not running them
if [[ ${unittest} -eq 1 || ${coverage} -eq 1 ]]; then
	gn_args="${gn_args} is_debug=true chip_build_tests=true"
fi

# Go to destination folder and run CMake build
build_cmd="${build_cmd} && pushd ${SOURCE_DIR}"
build_cmd="${build_cmd} && gn gen ${dest} --args='${gn_args}'"
build_cmd="${build_cmd} && \${build_wrapper} ninja -C ${dest} debian"

# Add unittest
if [ ${unittest} -eq 1 ]; then
	build_cmd="${build_cmd} && ninja -C ${dest} check"
	build_cmd="${build_cmd} && ./../scripts/run_tests.sh -b ${dest}"
fi

# Add coverage
if [ ${coverage} -eq 1 ]; then
	build_cmd="${build_cmd} && gcovr -r ${dest} -f src -e src/tests --sonarqube ${dest}/sonarqube.xml"
	build_cmd="${build_cmd} && xmllint --format ${dest}/sonarqube.xml > ${dest}/sonarqube_coverage.xml"
	build_cmd="${build_cmd} && gcovr -r ${dest} -f src -e src/tests --html-details ${dest}/coverage.html"
fi

# When done we go back to origial working directory
build_cmd="${build_cmd} && popd"

echo "complete build cmd = ${build_cmd}"

bash -xc "${build_cmd}"