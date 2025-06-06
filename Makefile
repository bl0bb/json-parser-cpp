CC_VERSION = 20

ERROR_FLAGS = \
# -Wall\
# -Wextra\
# -pedantic

CORE_FILES = 

FILES = $(CORE_FILES) src/main.cpp



# windows
CC = g++

# macos
# CC = g++

# linux
# CC = g++


all:
	$(CC) -std=c++$(CC_VERSION) $(ERROR_FLAGS) $(FILES) -g -o main.exe