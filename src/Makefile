SUB_DIR= decoder/ utils/ ctcbin/ 

all: Target   

Target:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make) done;


clean:
	@for subdir in $(SUB_DIR);do (cd $$subdir && make clean) done;
	rm -rf ../output


