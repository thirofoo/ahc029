exec-test:
	g++ main.cpp -o main -std=c++17
	cd tools; \
	cargo run -r --bin tester ../main < ../_in > ../_out 2> ../_err
