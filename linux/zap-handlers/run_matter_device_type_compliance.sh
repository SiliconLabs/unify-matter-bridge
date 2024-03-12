#!/bin/bash
pushd ../third_party/connectedhomeip

output_dir=../../../linux/zap-generated/device_spec_compliance

rm -rf $output_dir
mkdir -p $output_dir

./scripts/tools/zap/generate.py -t ../../../linux/zap-handlers/device-spec-compliance.json \
  -o $output_dir \
  ../../../unify-matter-bridge-common/unify-matter-bridge.zap

find $output_dir -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit

popd
