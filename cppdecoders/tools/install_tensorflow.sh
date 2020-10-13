# Download tensorflow choose r2.0
git clone -b r2.0 https://github.com/tensorflow/tensorflow.git tensorflow

# Configure your system build. using default config options
cd tensorflow && ./configure

# Build tensorflow from soure using Bazel
# errors may occure, please reference to README.md
bazel build --config=opt //tensorflow:libtensorflow_cc.so

# Download and Build tensorflow dependencies
# Pull down the required versions of the frameworks we need.
# errors may occure, please reference to README.md
sh tensorflow/contrib/makefile/download_dependencies.sh
# Compile protobuf.
sh tensorflow/contrib/makefile/compile_linux_protobuf.sh
