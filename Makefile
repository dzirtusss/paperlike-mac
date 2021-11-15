PROGRAM_NAME = paperlike
CFLAGS = -framework IOKit -framework CoreFoundation

default:
	gcc $(PROGRAM_NAME).c -o $(PROGRAM_NAME) $(CFLAGS)
