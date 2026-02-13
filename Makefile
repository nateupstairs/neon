app:
	clang++ -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/build

debug:
	clang++ -g -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/debug

run:
	./build/build
