CC 			:= gcc
CFLAGS	:= -o cage -Wall -Wextra -g -O0 -DDEBUG -fsanitize=address,undefined -lseccomp

cage: cage.c cage.h utils.c utils.h container.c container.h seccomp.c seccomp.h
	$(CC) $(CFLAGS) cage.c utils.c container.c seccomp.c 

cage.o: cage.c
	$(CC) $(CFLAGS) -c cage.c

utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c

container.o: container.c 
	$(CC) $(CFLAGS) -c container.c

seccomp.o: seccomp.c 
	$(CC) $(CFLAGS) -c seccomp.c

clean:
	rm -rf cage *.o
