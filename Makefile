src := $(wildcard ./includes/*.h)
flags := -lm -pthread -I./includes
build_dir := ./build
all: test kmer

test: test.c $(src)
	gcc test.c -o $(build_dir)/$@ $(flags)

kmer: kmer.c $(src)
	gcc kmer.c -o $(build_dir)/$@ $(flags)

clean: 
	rm ./build/*