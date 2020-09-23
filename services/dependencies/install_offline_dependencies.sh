

if [ ! -d glog ];then
    git clone https://github.com/google/glog.git
    cd glog
    mkdir -p build && cd build
    cmake ..
    make
    cp ../src/glog/*.h ./glog
    cd ../../
fi

if [ ! -d libevent ];then
    git clone -b patches-2.1 https://github.com/libevent/libevent.git
    cd libevent
    mkdir -p build && cd build
    cmake ..
    make -j
    cd ../../
fi


if [ ! -d evpp ];then
    git clone https://github.com/Qihoo360/evpp.git
    if [ -e glog/build/libglog.a ];then
        cp glog/build/libglog.a evpp/3rdparty/
        cp -r glog/build/glog evpp/3rdparty/
    else
        echo "do not find libglog.a" && exit -1
    fi

    if [ -e libevent/build/lib/libevent.a ];then
        cp libevent/build/lib/libevent.a evpp/3rdparty/
    else
        echo "do not find libevent.a" && exit -1
    fi

    cd evpp
    sed -i 's/add_subdirectory (test)/#add_subdirectory (test)/' CMakeLists.txt
    sed -i 's/add_subdirectory (examples)/#add_subdirectory (examples)/' CMakeLists.txt
    sed -i 's/add_subdirectory (benchmark)/#add_subdirectory (benchmark)/' CMakeLists.txt
    mkdir -p build && cd build
    cmake ..
    make evpp_static
    cd ../../

fi


