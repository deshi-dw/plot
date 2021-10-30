NAME 			= plot
VERSION_MAJOR 	= 0
VERSION_MINOR 	= 1
OUTPUT			= DEBUG
PLATFORM		= WINDOWS
CC 				= clang

CFLAGS			= -Wall -pedantic -Wdocumentation -std=c11 -Iinclude
LIBS 			=	-Llib						\
					-lraylibdll					\
					-lopengl32 					\
					-lgdi32 					\
					-lwinmm

ifeq ($(OUTPUT), DEBUG)
	CFLAGS += -g -O0 -v
#	CFLAGS += -DDEBUG_TIME
else
	CFLAGS += -Wl,--subsystem,windows
endif

SRC			   := $(wildcard src/*.c)
OBJ			   := $(patsubst src/%.c,obj/%.o,$(SRC))

.PHONY: clean

$(NAME): $(NAME)

all: $(NAME)

$(NAME): $(OBJ)
	-mkdir bin
	-powershell -c 'cp lib/*.dll bin/'
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o bin/$(NAME).exe

obj/%.o: src/%.c
	-mkdir obj
	$(CC) $(CFLAGS) -DPLOT_VERSION_MAJOR=$(VERSION_MAJOR) -DPLOT_VERSION_MINOR=$(VERSION_MINOR) -D$(PLATFORM) -c $< -o $@

clean:
	-powershell -c 'rm obj/*; rm bin/*'