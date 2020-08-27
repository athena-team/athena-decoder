SUB_DIR= fst/ decoder/ kaldi/ misc/ ctcbin/ 

all: Target   

Target:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make) done;


clean:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make clean) done;
	rm -rf ../output


