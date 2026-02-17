app:
	clang++ -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/build -O2

debug:
	clang++ -g -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/debug

wasm:
	em++ --bind -std=c++23 src/wasm.cpp -Ivendor/json/single_include -o ./build/scrap.js -O2 -s WASM=1 -s EXPORT_ES6=1 -s MODULARIZE=1

run:
	./build/build
