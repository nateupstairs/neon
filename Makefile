app:
	clang++ -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/build -O2

debug:
	clang++ -g -std=c++23 src/test.cpp -Ivendor/json/single_include -o ./build/debug

wasm:
	em++ --bind -std=c++23 src/wasm.cpp -Ivendor/json/single_include -o ./build/scrap.js -O2 -s WASM=1 -s EXPORT_ES6=1 -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1

wasm_standalone:
	em++ -std=c++23 -DNEON_STANDALONE src/wasm.cpp -Ivendor/json/single_include -o ./build/neon.wasm -O2 -s STANDALONE_WASM -s EXPORT_KEEPALIVE=1 --no-entry

test:
	clang++ -std=c++23 src/test_scrap.cpp -Ivendor/json/single_include -o ./build/test_scrap -O2 && ./build/test_scrap

run:
	./build/build
