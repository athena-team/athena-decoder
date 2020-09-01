
include path.mk

SUB_DIR= decoder/ utils/ ctcbin/ feat/ tfutils/ ctc/ transformer/
all: Target   

Target:
	mkdir -p lib
	cp -f $(TF_LIB_DIR)/libtensorflow_cc.so.1.14.1 ./lib
	cp -f $(TF_LIB_DIR)/libtensorflow_framework.so.1.14.1 ./lib
	cd ./lib && ln -sf libtensorflow_cc.so.1.14.1 libtensorflow_cc.so.1
	cd ./lib && ln -sf libtensorflow_cc.so.1 libtensorflow_cc.so
	cd ./lib && ln -sf libtensorflow_framework.so.1.14.1 libtensorflow_framework.so.1
	cd ./lib && ln -sf libtensorflow_framework.so.1 libtensorflow_framework.so
	@for subdir in $(SUB_DIR);do (cd $$subdir && make) done;


clean:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make clean) done;
	rm -rf ./lib


