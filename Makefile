
SRC_PATH	=	src/
OBJ_PATH	=	obj/
INC_PATH	=	inc/
GCC_LOG		=	gcc_log_file.log

SRCS		=	$(shell ls $(SRC_PATH) | grep .cpp$$)
OBJS		=	$(patsubst %.cpp, $(OBJ_PATH)%.o,$(SRCS))

PLATFORM	:=	$(shell uname)
CC			=	g++
HEADER		=	-I./$(INC_PATH)
FLAGS		=	 -lpthread -g -O3 -Wall -Wextra -Werror -lm -Wno-deprecated-declarations -std=gnu++11 -Wno-unused
NAME		=	vbe

ifeq "$(PLATFORM)" "WIN32"
NAME		+=	.exe
endif

SDL			=	`sdl2-config --cflags --libs`

ifeq "$(PLATFORM)" "Darwin" #MAC
GL			=	-framework OpenGL -framework Cocoa
else ifeq "$(PLATFORM)" "Linux" #LINUX
GL			=	-lGL -lGLU
else ifeq "$(PLATFORM)" "Win32" #WINDOWS
GL			=	-lopengl32
endif

all: $(NAME)

$(NAME): $(OBJS)
ifeq "$(PLATFORM)" "Darwin"
	@$(CC) $(FLAGS) $(HEADER) $(SDL) $(GL) $(OBJS) -o $(NAME)
else ifeq "$(PLATFORM)" "Linux"
	@$(CC) $(FLAGS) $(HEADER) $(OBJS) -o $(NAME) $(SDL) $(GL)
endif

$(patsubst %, $(OBJ_PATH)%,%.o): $(SRC_PATH)$(notdir %.cpp)
	@mkdir -p $(OBJ_PATH)
	@$(CC) -c $(FLAGS) $(HEADER) $(SDL) $(GL) "$<" -o "$@"

clean:
	@rm -rf $(OBJ_PATH)

fclean: clean
	@rm -f $(NAME)

loop:
	@while [ 1 ]; do \
			make re > $(GCC_LOG) 2>&1 ; \
			clear ; \
			cat $(GCC_LOG); \
		if [ -s $(NAME) ] ; then \
			echo "\033[32;1mCompilation done !\033[0m" ; \
			rm -rf $(GCC_LOG); \
			break ; \
		fi ; \
		sleep 1 ; \
	done

config:
	@sed -i '.tmp' 's/bitset/'"$(NAME)"'/g' Makefile
	@echo "Renamed executable to '"$(NAME)"'"
	@rm -rf Makefile.tmp

re: fclean all

l: re
	@./$(NAME)

launch: l

.PHONY: clean fclean re
