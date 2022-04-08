.PHONY: all nebula test clean

all:
	tup

nebula:
	tup nebula

test:
	tup test && ./test

clean:
	rm *.o && rm nebula test
