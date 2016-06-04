all: young

young: young.o yng_connection.o yng_util.o yng_request.o
	gcc young.o yng_connection.o yng_util.o yng_request.o -o young

young.o: src/young.c
	gcc -I src/ -c src/young.c

yng_connection.o: src/yng_connection.c
	gcc -I src/ -c src/yng_connection.c

yng_util.o: src/yng_util.c
	gcc -I src/ -c src/yng_util.c

yng_request.o: src/yng_request.c
	gcc -I src/ -c src/yng_request.c

clean:
	rm *o young
