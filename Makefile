collect:collect.c collect.h set.c set.h crc32.c crc32.h
	gcc -o collect collect.c collect.h set.c set.h crc32.c crc32.h -lpthread
clean:
	rm collect
