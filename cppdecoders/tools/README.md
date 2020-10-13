
# Install Tensorflow and Dependencies

#### required
+ bazel version: 0.26.1
+ tensorflow version: 2.0
+ python version: 3.6.8

#### install bazel
+ run the script
```
sudo sh install_bazel.sh

```

#### install tensorflow
+ run the script
```
sudo sh install_tensorflow.sh
```



# Issues

1. Tensorflow configure error
+ configure failed
```
Fetching @io_bazel_rules_docker;
Cloning 251f6a68b439744094faff800cd029798edf9faa of https://github.com/bazelbuild/rules_docker.git failed

```
+ solution
    - Open tensorflow/WORKSPACE file
    - Add the following scripts before the origin http_archive items
    ```
    http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "aed1c249d4ec8f703edddf35cbe9dfaca0b5f5ea6e4cd9e83e99f3b0d1136c3d",
    strip_prefix = "rules_docker-0.7.0",
    urls = ["https://github.com/bazelbuild/rules_docker/archive/v0.7.0.tar.gz"],
    )

    ```    

2. Download dependencies failed
we can download all the dependencies manually, for CPU version, just the following
4 dependencies are needed.

+ build directory
```
cd tools/tensorflow
mkdir -p tensorflow/contrib/makefile/downloads
cd tensorflow/contrib/makefile/downloads

```
+ download eigen
```
http://eigen.tuxfamily.org/index.php?title=Main_Page
wget or curl the latest Eigen (now version=3.3.8)
uncompress the Eigen in downloads directory
rename it as eigen

```
+ download absl
```
git clone https://github.com/abseil/abseil-cpp.git
# rename the directory
mv abseil-cpp absl

```
+ download nsync
```
git clone https://github.com/google/nsync.git
```
+ download protobuf
```
git clone https://github.com/protocolbuffers/protobuf.git
```

