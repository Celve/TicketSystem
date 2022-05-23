.PHONY: target
target: 
	cmake --build build -j 12

.PHONY: clean
clean:
	rm -f ./build/test.db ./build/index.db index.db

