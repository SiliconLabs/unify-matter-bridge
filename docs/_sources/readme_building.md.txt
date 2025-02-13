# Building the Unify Matter Bridge

This build guide helps with cross-compilation for arm64 architecture to be run on Unify's reference platform - a Raspberry Pi 4 (RPi4) with the 64-bit version of Debian Bookworm.

> **Note:**
> In the following subsections the commands should either be run on your local development machine or inside a running Docker container, as distinguished by the structure of the example.
>
> - _some-command_ should be executed on your local machine.
>   - _`dev-machine:~$ some-command`_
> - _some-other-command_ should be executed inside the Docker container.
>   - _`root@docker:/<dir>$ some-other-command`_

## Check Out Submodules

> 🔴 Assuming you have cloned the unify-matter-bridge repo in `~/unify-matter-bridge` 

Check out the necessary submodules with the following command.

```bash
dev-machine:~/unify-matter-bridge$ ./scripts/build_essentials.sh
```

## Build the Docker Container (arm64 compilation)

```bash
dev-machine:~/unify-matter-bridge$ docker build -t unify-matter docker/
```

Start the docker from `unify-matter-bridge/` directory where you cloned the unify-matter-bridge repo: Here we assume `unify-matter-bridge/` is in `~`

```bash
dev-machine:~$ cd unify-matter-bridge/
dev-machine:~/unify-matter-bridge$ docker run -it -v $PWD:/unify-matter-bridge -v ./linux/third_party/UnifySDK:/uic unify-matter
```

## Activate Matter development environment

Once you have all the necessary submodules, source the Matter environment with the following command. This loads a number of build tools and makes sure the correct toolchains and compilers are used for compiling the Unify Matter Bridge.

Make sure you are in `/unify-matter-bridge/linux/third_party/connectedhomeip` directory

```bash
root@docker:/unify-matter-bridge$ cd /unify-matter-bridge/linux/third_party/connectedhomeip
root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip$ git config --global --add safe.directory /unify-matter-bridge/linux/third_party/connectedhomeip
root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip$ git config --global --add safe.directory /unify-matter-bridge/linux/third_party/connectedhomeip/third_party/pigweed/repo
root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip$ source ./scripts/activate.sh
```

## Generate Zap Code

This step is optional for normal build, only to be done for changes in Zap files.

To use Zap to generate code from Unify XML files you need to export UCL_XML_PATH as well.

```bash
root@docker:/unify-matter-bridge$ cd /uic
root@docker:/uic$ export UCL_XML_PATH=/uic/components/uic_dotdot/dotdot-xml
```

Run below Zap script to generate zap files for unify matter bridge.

```bash
root@docker:/unify-matter-bridge$ cd /unify-matter-bridge/linux/zap-handlers
root@docker:/unify-matter-bridge/linux/zap-handlers$ ./run_all_zap.sh
```

## Compile the Unify Matter Bridge

Make sure you are in `/unify-matter-bridge/linux` directory

```bash
root@docker:/unify-matter-bridge$ cd /unify-matter-bridge/linux
root@docker:/unify-matter-bridge/linux$ export PKG_CONFIG_PATH=/unify-matter-bridge/linux/out/arm64/gen/stage/share/pkgconfig:/usr/lib/aarch64-linux-gnu/pkgconfig/
root@docker:/unify-matter-bridge/linux$ gn gen out/arm64 --args='target_cpu="arm64"'
root@docker:/unify-matter-bridge/linux$ ninja -C out/arm64 debian
```
Note: Please update the arch name and triples as needed when building for other variants.

> 🔴 After building, the `unify-matter-bridge` binary is located at `/unify-matter-bridge/linux/out/arm64/obj/bin/unify-matter-bridge` and debian is located at `/unify-matter-bridge/linux/out/arm64/packages/uic-mb_<version>_arm64.deb`.

## Compile the chip-tool

The `chip-tool` is a CLI tool that can be used to commission the bridge and to control end devices.

```bash
root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip$ cd examples/chip-tool

root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool$ gn gen out/arm64 --args='target_cpu="arm64"'

root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool$ ninja -C out/arm64
```

> 🔴 After building, the chip-tool binary is located at `/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool/out/arm64/chip-tool`.

## Troubleshooting

1. If you do not source the `connectedhomeip/scripts/activate.sh` as described above in [Activate Matter development environment](#activate-matter-development-environment),`gn` and other common build tools will not be found.
2. If you are compiling unit tests, do not try to compile the Unify Matter Bridge at
   the same time. This will not work as when compiling unit tests you are also
   compiling unit tests for all other sub-components.
3. Encountering problems with the submodules can be due to trying to check out
   the submodules inside the docker container.
4. If the Unify Matter Bridge gets stuck while booting. Try to pass `--args="chip_config_network_layer_ble=false"` to `gn gen` command while building.<br>`Note: Doing this will disable ble commissioning feature`
