
include path.mk

SUB_DIR= decoder/ utils/ feat/ tfutils/ ctc/ ctcbin/ transformer/ transformerbin/ interface/
all: Target   

Target:
	mkdir -p lib
	cp -f $(TF_LIB_DIR)/libtensorflow_cc.so.2.0.3 ./lib
	cp -f $(TF_LIB_DIR)/libtensorflow_framework.so.2.0.3 ./lib
	cd ./lib && ln -sf libtensorflow_cc.so.2.0.3 libtensorflow_cc.so.2
	cd ./lib && ln -sf libtensorflow_cc.so.2 libtensorflow_cc.so
	cd ./lib && ln -sf libtensorflow_framework.so.2.0.3 libtensorflow_framework.so.2
	cd ./lib && ln -sf libtensorflow_framework.so.2 libtensorflow_framework.so
	@for subdir in $(SUB_DIR);do (cd $$subdir && make) done;
	cd ./lib && ln -sf ../ctc/libinference-ctc.so libinference-ctc.so
	cd ./lib && ln -sf ../transformer/libinference-transformer.so libinference-transformer.so


clean:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make clean) done;
	rm -rf ./lib


