.PHONY: clean
build:
	rm -f ted
	gcc src/terminal.c src/features.c src/operations.c src/io.c src/main_ted.c -o ted

run: build
	./ted

clean:
	rm -f ted