cage: cage.c cage.h utils.c utils.h container.c container.h seccomp.c seccomp.h
	gcc -g -o cage cage.c utils.c container.c seccomp.c -lseccomp

cage.o: cage.c 

utils.o: utils.c 

clean:
	rm -rf cage *.o
