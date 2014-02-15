all: test

test: test.c xxHash.c md5.c xxHash.h md5.h
	gcc test.c xxHash.c md5.c -o test

clean:
	rm -rf test