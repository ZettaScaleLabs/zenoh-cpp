<img src="https://raw.githubusercontent.com/eclipse-zenoh/zenoh/master/zenoh-dragon.png" height="150">

[![CI](https://github.com/eclipse-zenoh/zenoh-cpp/workflows/CI/badge.svg)](https://github.com/eclipse-zenoh/zenoh-cpp/actions?query=workflow%3A%22CI%22)
[![Documentation Status](https://readthedocs.org/projects/zenoh-cpp/badge/?version=latest)](https://zenoh-cpp.readthedocs.io/en/latest/?badge=latest)
[![Discussion](https://img.shields.io/badge/discussion-on%20github-blue)](https://github.com/eclipse-zenoh/roadmap/discussions)
[![Discord](https://img.shields.io/badge/chat-on%20discord-blue)](https://discord.gg/2GJ958VuHs)
[![License](https://img.shields.io/badge/License-EPL%202.0-blue)](https://choosealicense.com/licenses/epl-2.0/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Eclipse Zenoh
The Eclipse Zenoh: Zero Overhead Pub/sub, Store/Query and Compute.

Zenoh (pronounce _/zeno/_) unifies data in motion, data at rest and computations. It carefully blends traditional pub/sub with geo-distributed storages, queries and computations, while retaining a level of time and space efficiency that is well beyond any of the mainstream stacks.

Check the website [zenoh.io](http://zenoh.io) and the [roadmap](https://github.com/eclipse-zenoh/roadmap) for more detailed information.

# C++ API

The Zenoh C++ API is a C++ bindings for [zenoh-c] and [zenoh-pico] libraries. The library is headers only,
but building [zenoh-c] requires [Rust](rust-lang) installed. Please check [here](https://www.rust-lang.org/tools/install) to learn how to install it.


C++ bindings are still so the Zenoh team will highly appreciate any help in testing them on various platforms, system architecture, etc. and to report any issue you might encounter. This will help in greatly improving its maturity and robustness.

-------------------------------
## How to build and install it

> :warning: **WARNING** :warning: : Zenoh and its ecosystem are under active development. When you build from git, make sure you also build from git any other Zenoh repository you plan to use (e.g. binding, plugin, backend, etc.). It may happen that some changes in git are not compatible with the most recent packaged Zenoh release (e.g. deb, docker, pip). We put particular effort in mantaining compatibility between the various git repositories in the Zenoh project.

To install [zenoh-cpp] do the following steps:

1. Clone the sources

   ```bash
   git clone https://github.com/eclipse-zenoh/zenoh-cpp.git
   ```

2. Do install.
   Neither [zenoh-c] nor [zenoh-pico] are required for the installation, but both are neccessary for building tests and examples. So, instead of the main project, it's faster to do install from "install" subproject.

   Use option `CMAKE_INSTALL_PREFIX` for specifying installation location. Without this parameter installation is performed to default system location `/usr/local` which requires root privileges.

    ```bash
    mkdir build && cd build
    cmake ../zenoh-cpp/install -DCMAKE_INSTALL_PREFIX=~/.local
    cmake --install .
    ```

## Building and running tests

The [zenoh-cpp] is header-only C++ library that wraps [zenoh-c] and [zenoh-pico] libraries. To make tests and examples it tries to find `zenoh-c` and `zenoh-pico` libraries in the following places:

1. Directories `zenoh-c` and `zenoh-pico` located on the same level as `zenoh-cpp` itself. **WARNING**: If you see building errors, make sure that you don't have obsolete `zenoh-c` or `zenoh-pico` directories nearby

2. Libraries `zenoh-c` and `zenoh-pico` installed to system. **WARNING**: If you see building errors, make sure that no old version of libraries installed to `/usr/local/lib/cmake`

3. Download [zenoh-c] and [zenoh-pico] from GitHub

```bash
mkdir -p build && cd build
cmake ../zenoh-cpp
cmake --build . --target tests
ctest
```

Notice that the output of `cmake ../zenoh-cpp` shows where the dependencies were found

## Building the Examples

Examples are splitted into two subdirectories. Subdirectory `universal` contains [zenoh-cpp] examples buildable with both [zenoh-c] and [zenoh-pico] backends. The `zenohc` subdirectory contains examples with zenoh-c specific functionality.

The examples can be built in two ways. One is to select `examples` as a build target of the main project:

```bash
mkdir -p build && cd build
cmake ../zenoh-cpp
cmake --build . --target examples
```

Examples are placed into `build/examples/zenohc` and `build/examples/zenohpico` directories.

Second way is to build `examples` as a root project, which includes [zenoh-cpp] as subproject

```bash
mkdir -p build && cd build
cmake ../zenoh-cpp/examples
cmake --build .
```

Examples are placed into `build/zenohc` and `build/zenohpico` directories.

## Running the examples

Change current directory to the variant you want (`examples/zenohc` or `examples/zenohpico` in the build directory)

See example sources for command line arguments (key expression, value, router address).

`zenohc` examples can work standalone, but for `zenohpico` examples the working zenoh router is required. So to run `zenohpico` examples download [zenoh] project and run the router ([Rust](rust-lang) should be installed):
```bash
git clone https://github.com/eclipse-zenoh/zenoh
cd zenoh
cargo run
```

### Basic Pub/Sub Example

```bash
./z_sub
```

```bash
./z_pub
```

The `z_pub` should receive message sent by `z_sub`

### Queryable and Query Example
```bash
./z_queryable
```

```bash
./z_get
```

The `z_get` should receive the data from `z_queryable`

### Throughput Examples
```bash
./z_sub_thr_cpp
```

```bash
./z_pub_thr_cpp 1024
```

After 30-40 seconds delay the `z_sub_thr` will start to show the throughput measure results

## Library usage

Below are the steps to include [zenoh-cpp] into CMake project. See also [examples/simple](examples/simple) directory for short examples of CMakeLists.txt.

- include [zenoh-c] or [zenoh-pico] into your CMake project **before** dependency on [zenoh-cpp] itself.
  This is important as the library targets you need (`zenohcxx::zenohpico`, `zenohcxx::zenohc::lib`, `zenohcxx::zenohc::shared` and `zenohcxx::zenohc::static)`
  are defined only if their backend library
  targets (`zenohpico`, `zenohc::lib`, `zenohc::shared` and `zenohc::static` are defined)

- include [zenoh-cpp] itself. This can be done with [add_subdirectory], [find_package] or [FetchContent] CMake commands.
  ```
  add_subdirectory(../zenoh-cpp)
  ```
  ```
  find_package(zenohcxx)
  ```
  ```
  FetchContent_Declare(zenohcxx GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-cpp GIT_TAG b3228d8ed0002e96bbd7fcf08a146ad5431b4415)
  FetchContent_MakeAvailable(zenohcxx)
  ```
- add dependency on zenoh-cpp to your project in one of these ways:
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohpico)
   ```
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohc::lib)
   ```
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohc::shared)
   ```
   ```
   target_link_libraries(yourproject PUBLIC zenohcxx::zenohc::static)
   ```

- include the [zenoh.hxx] header and use namespace `zenoh`.
    ```C++
    #include "zenoh.hxx"
    using namespace zenoh;
    ```

## Library API

### Documentation

The documentation is on [zenoh-cpp.readthedocs.io]. 
Instruction how to build documentation locally is at [docs/README.md].

[rust-lang]: https://www.rust-lang.org
[zenoh]: https://github.com/eclipse-zenoh/zenoh
[zenoh-c]: https://github.com/eclipse-zenoh/zenoh-c
[zenoh-cpp]: https://github.com/eclipse-zenoh/zenoh-cpp
[zenoh-pico]: https://github.com/eclipse-zenoh/zenoh-pico
[zenoh.hxx]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/include/zenoh.hxx
[add_subdirectory]: https://cmake.org/cmake/help/latest/command/add_subdirectory.html
[find_package]: https://cmake.org/cmake/help/latest/command/find_package.html
[FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html
[zenoh-cpp.readthedocs.io]: https://zenoh-cpp.readthedocs.io
[docs/README.md]: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/docs/README.md