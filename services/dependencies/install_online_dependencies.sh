
if [ ! -d websocketpp ];then
    git clone https://github.com/zaphoyd/websocketpp.git
fi

if [ ! -d boost_1_72_0 ];then
    wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz
    tar zxvf boost_1_72_0.tar.gz
fi


if [ ! -d ThreadPool ];then
    git clone https://github.com/progschj/ThreadPool.git
fi

if [ ! -d glog ];then
    git clone https://github.com/google/glog.git
    cd glog
    mkdir -p build && cd build
    cmake ..
    make
    cp ../src/glog/*.h ./glog
    cd ../../
fi
