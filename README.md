# Project's CMake configuration

Root CMakeLists.txt defines your project files, common flags, includes third-party libs.

The project is using Core library as git submodule. 
CMake is using SSH connection with SSH authorization.
First you need to generate keys on your remote Linux machine. Run this command in console:

> ssh-keygen -t rsa -b 4096 -C "your_email@example.com"

Then you need to print the key and copy. Run this command in console:

> cat ~/.ssh/id_rsa.pub

Paste the key to your github profile 

[How to set SSH key](https://docs.github.com/ru/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account?platform=linux&tool=cli)

To add a new library just add a line in CMakeLists.txt
```cmake
ADD_LIB(<lib_name>)
```
<lib_name> must be the same as target folder name 

To add a new .cpp file in CMakeLists.txt use this line:
```cmake
add_executable(demo_strategy demo_strategy.cpp)
```
You need to link all your libraries with your project using this line:
```cmake
target_link_libraries(demo_strategy basics config_tree control networking order_entry order_gateway strategy_framework)
```
# Building

Requirements:
- Cmake >= 3.26

Set required compiler in the root CMakeLists.txt file
```cmake
set(CMAKE_CXX_COMPILER <path>) # g++ is a default option
```

From your code root directory:

```bash
  cd demo_strategy
  mkdir build && cd build
  cmake -DARCH=native -DCMAKE_BUILD_TYPE=[Debug|Release] ..
  make <target> <args>
```
We recommend using make with flags _-j 8_ or _-j 16_ to speed the process up.

Compiled file is stored in:
- Build
