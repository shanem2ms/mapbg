cdir=$(pwd)
git clone --recurse-submodules https://github.com/bkaradzic/bgfx.cmake.git 
cd bgfx.cmake
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_ARC=FALSE -DBGFX_BUILD_TOOLS=off -DBGFX_BUILD_EXAMPLES=off -DBGFX_INSTALL=off
cmake --build .
cd $cdir/app
mkdir bld
cd bld
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS64
