Setup instruction https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/

Google test is a framework for writing C++ unit tests. In this short post, I explain how to set it up in Ubuntu.

Start by installing the gtest development package:

sudo apt-get install libgtest-dev
Note that this package only install source files. You have to compile the code yourself to create the necessary library files. These source files should be located at /usr/src/gtest. Browse to this folder and use cmake to compile the library:

```
sudo apt-get install cmake # install cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
 
# copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib
```

To compile the code, I’ve set up a small CMakeLists.txt file below to compile the tests. This file locates the google test library and links it with the test application. Note that we also have to link to the pthread library or the application won’t compile.

```
cmake_minimum_required(VERSION 2.6)
 
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
 
# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests tests.cpp)
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
```

Compile and run the tests:

```
cmake CMakeLists.txt
make
./runTests
```
