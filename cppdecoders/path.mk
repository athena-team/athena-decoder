
MPATH := $(abspath $(lastword $(MAKEFILE_LIST)))
# DIR is the absolute path of path.mk
DIR := $(dir $(MPATH))


TENSORFLOW := ${DIR}/tools/tensorflow

EXTRA_CXXFLAGS = -Wno-sign-compare \
	 -I$(TENSORFLOW) \
	 -I$(TENSORFLOW)/bazel-genfiles \
	 -I$(TENSORFLOW)/tensorflow/contrib/makefile/downloads/eigen \
	 -I$(TENSORFLOW)/tensorflow/contrib/makefile/downloads/nsync/public \
	 -I$(TENSORFLOW)/tensorflow/contrib/makefile/downloads/protobuf/src \
	 -I${TENSORFLOW}/tensorflow/contrib/makefile/downloads/absl

TF_LIB_DIR := $(TENSORFLOW)/bazel-bin/tensorflow


