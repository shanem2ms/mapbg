cdir=$(pwd)
git clone --recurse-submodules https://github.com/bkaradzic/bgfx.cmake.git 
cd bgfx.cmake
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_ARC=FALSE
cd $cdir/app
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64
