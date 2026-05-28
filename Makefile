cage: cage.c cage.h utils.c utils.h
	gcc -o cage cage.c utils.c 

cage.o: cage.c 

utils.o: utils.c 

clean:
	rm -rf cage *.o
