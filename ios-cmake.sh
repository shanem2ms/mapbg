cd bgfx.cmake
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_ARC=FALSE
cd app
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64
