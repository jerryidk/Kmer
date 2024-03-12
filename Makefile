src := $(wildcard ./includes/*.h)
build_dir := ./build
kmer_flags := -pthread -I./includes

all: test kmer

test: $(build_dir)/test.o
	gcc $^ -o $(build_dir)/$@ -lm

$(build_dir)/test.o: test.c $(src)
	gcc -c test.c -o $@ -I./includes

kmer: $(build_dir)/kmer.o
	gcc $^ -o $(build_dir)/$@ -lm

$(build_dir)/kmer.o: kmer.c $(src)
	gcc -c kmer.c -o $@ $(kmer_flags)

clean: 
	rm ./build/*