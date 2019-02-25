all: library test_static test_shared test_dynamic

library: static shared

static:
	gcc -c Array.c
	ar rcs libArray.a Array.o
	
shared:
	gcc -fPIC -c Array.c
	gcc -fPIC -shared -o libArray.so Array.o
	
clean:
	rm -f *.o *.so *.a *.out


test_static:
	gcc -c test.c
	gcc test.o libArray.a -o test_static.out

test_shared:
	gcc test.c -lArray -L./ -Wl,-rpath=./ -o test_shared.out
	
test_dynamic:
	gcc testDyn.c -L./ -ldl -D DLL -o test_dynamic.out


test_static_O1:
	gcc -c test.c
	gcc test.o libArray.a -o test_static.out -O1

test_static_O2:
	gcc -c test.c
	gcc test.o libArray.a -o test_static.out -O2

test_static_O3:
	gcc -c test.c
	gcc test.o libArray.a -o test_static.out -O3

test_static_Os:
	gcc -c test.c
	gcc test.o libArray.a -o test_static.out -Os