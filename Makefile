exec-test:
	g++ main.cpp -o main -std=c++17
	cd tools; \
	cargo run -r --bin tester ../main < ../_in > ../_out 2> ../_err

exec-sample:
	cd tools; \
	cargo run -r --bin tester ../sample/sample_submission < ../_in > ../_out 2> ../_err
