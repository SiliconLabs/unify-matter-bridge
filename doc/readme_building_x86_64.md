# Building the Unify Matter Bridge for Debian Bullseye (Or similar Ubuntu) x86_64 

> **Note:**
> In the following subsections the commands should either be run on your local development machine or inside a running Docker container, as distinguished by the structure of the example.
>
> - _some-command_ should be executed on your local machine.
>   - _`dev-machine:~$ some-command`_
> - _some-other-command_ should be executed inside the Docker container.
>   - _`root@docker:/<dir>$ some-other-command`_


## Clone and Stage the Unify SDK Repository 

> 🔴 Assuming you have cloned the unify-matter-bridge repo in `~/unify-matter-bridge` 

Check out the necessary submodules with the following command.

```bash
dev-machine:~/unify-matter-bridge$ ./scripts/build_essentials.sh
```

## Build the Docker Container (host compilation)

```bash
dev-machine:~/unify-matter-bridge$ docker build -t unify-matter-host --build-arg ARCH=amd64 docker/
```
## Run the docker container (host compilation)

```bash
dev-machine:~$ cd unify-matter-bridge/
dev-machine:~/unify-matter-bridge$ docker run -it -v $PWD:/unify-matter-bridge -v ./linux/third_party/UnifySDK:/uic unify-matter-host
```

If you want to be able to use Zap to generate code from Unify XML files you need to export UCL_XML_PATH as well.

```bash
root@docker:/uic$ export UCL_XML_PATH=/uic/stage/share/uic/ucl
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

## Compile the Unify Matter Bridge 

Make sure you are in `/sunify-matter-bridge/linux` directory

```bash
root@docker:/unify-matter-bridge$ cd /unify-matter-bridge/linux
root@docker:/unify-matter-bridge/linux$ gn gen out/host
root@docker:/unify-matter-bridge/linux$ ninja -C out/host debian
```

> 🔴 After building, the `unify-matter-bridge` binary is located at `/unify-matter-bridge/linux/out/host/obj/bin/unify-matter-bridge`.


The `chip-tool` is a CLI tool that can be used to commission the bridge and to control end devices.

```bash
root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip$ cd examples/chip-tool

root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool$ gn gen out/amd64

root@docker:/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool$ ninja -C out/amd64
```

> 🔴 After building, the chip-tool binary is located at `/unify-matter-bridge/linux/third_party/connectedhomeip/examples/chip-tool/out/amd64/chip-tool`.

## Unit Testing

Unit testing is always a good idea for quality software. Documentation on writing unit tests for the Matter Unify Bridge is in the
[README.md](https://github.com/SiliconLabs/matter/blob/latest/silabs_examples/unify-matter-bridge/linux/src/tests/README.md).

## Troubleshooting

Refer to Troubleshooting section in [readme_building.md](readme_building.md) 
