## Dependencies

Anaconda

- sudo apt-get install libgl1-mesa-glx libegl1-mesa libxrandr2 libxrandr2 libxss1 libxcursor1 libxcomposite1 libasound2 libxi6 libxtst6
- curl -O https://repo.anaconda.com/archive/Anaconda3-2024.02-1-Linux-x86_64.sh
- sudo chmod +x Anaconda3-2024.02-1-Linux-x86_64.sh
- ./Anaconda3-2024.02-1-Linux-x86_64.sh

G++:

- sudo apt-get update
- sudo apt-get install gcc-12 g++-12
- export CC=/usr/bin/gcc-12
- export CXX=/usr/bin/g++-12

cmake:

- sudo apt-get install cmake
- sudo apt-get install build-essential cmake ninja-build

vcpkg:

- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
- ./bootstrap-vcpkg.sh

Boost:

- wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.bz2
- tar --bzip2 -xf ~/boost_1_85_0.tar.bz2
