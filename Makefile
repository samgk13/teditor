.PHONY: run
build:
	rm -f output
	gcc src/ted.c -o output

run: build
	./output
