set -e

cmake -B build
cd build
make -j8

cd ..
./build/bin/rimo_zoom_bot_manager