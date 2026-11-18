.PHONY: all test example clean

# Domyślny target
all: test

# 1. Konfiguracja CMake w folderze build/ (automatycznie wykrywa zmiany w tests/), budowanie i uruchomienie testów
test:
	@if [ ! -f build/CMakeCache.txt ]; then \
		cmake -S . -B build; \
	fi
	@cmake --build build --target unit_tests
	@./build/unit_tests

# 2. Kompilacja i uruchomienie przykładu
example: strong_example.cpp strong.h
	g++ -Wall -Wextra -O2 -std=c++23 strong_example.cpp -o strong_example
	./strong_example

# 3. Kompilacja pliku strongcpp i uruchomienie a.out
try-compile: strong.cpp strong.h
	g++ -Wall -Wextra -O2 -std=c++23 strong.cpp -o a.out
	./a.out

# Czyszczenie plików tymczasowych
clean:
	rm -rf build strong_example a.out