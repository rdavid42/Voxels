
SRC_PATH	=	src
OBJ_PATH	=	obj
INC_PATH	=	inc

SRCS		=	$(shell ls $(SRC_PATH) | grep .cpp$$)
OBJS		=	$(patsubst %.cpp, $(OBJ_PATH)/%.o,$(SRCS))

PLATFORM	:=	$(shell uname)
CC			=	g++
HEADER		=	-I./$(INC_PATH) -I$(HOME)/.brew/include
LIBS		=	-L$(HOME)/.brew/lib
FLAGS		=	-O3 -Wall -Wextra -Werror -std=gnu++11

NAME		=	voxels

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $^ $(HEADER) $(LIBS) -lsfml-window -lsfml-system -framework OpenGL

$(patsubst %, $(OBJ_PATH)/%,%.o): $(SRC_PATH)/$(notdir %.cpp)
	mkdir -p $(OBJ_PATH)
	$(CC) -c $(FLAGS) $(HEADER) "$<" -o "$@"

reinstall:
	@brew remove sfml
	@brew install sfml

clean:
	@rm -rf $(OBJ_PATH)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re
