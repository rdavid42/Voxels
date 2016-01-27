
SRC_PATH	=	src/

OBJ_PATH	=	obj/
INC_PATH	=	inc/

SRCS		=	$(shell ls $(SRC_PATH) | grep .cpp$$)
OBJS		=	$(patsubst %.cpp, $(OBJ_PATH)%.o,$(SRCS))

PLATFORM	:=	$(shell uname)
CC			=	g++
HEADER		=	-I./$(INC_PATH) -I./glfw/include -I/usr/local/cuda-6.5/include -I.
FLAGS		=	-g -O3 -Wall -Wextra -Werror -std=gnu++11 -pedantic
VARS		=	\
# -DDEBUG \
# -DPARSER_DEBUG \

ifeq "$(PLATFORM)" "Darwin" #MAC
GLFW		=	./glfw/libglfw3_darwin.a
LIBS		=	$(GLFW) -lm -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread -framework OpenCL
else ifeq "$(PLATFORM)" "Linux" #LINUX
GLFW		=	./glfw/libglfw3_linux.a
LIBS		=	$(GLFW) -lGL -lX11 -lXrandr -lXi -lXxf86vm -lpthread -lXcursor -lXinerama
# -lXext -lXdamage -ldrm -lXrender
else ifeq "$(PLATFORM)" "Win32" #WINDOWS
GLFW		=	./glfw/libglfw3_win32.a
LIBS		=	$(GLFW) -lopengl32 -lgdi32 -luser32 -lkernel32 -lpthread
endif

NAME		=	voxels

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(FLAGS) $(VARS) $(HEADER) -o $(NAME) $(OBJS) $(LIBS)

$(patsubst %, $(OBJ_PATH)%,%.o): $(SRC_PATH)$(notdir %.cpp)
	@mkdir -p $(OBJ_PATH)
	@$(CC) -c $(FLAGS) $(VARS) $(HEADER) "$<" -o "$@"

clean_glfw:
	@make -C glfw/ clean

clean:
	@rm -rf $(OBJ_PATH)

fclean: clean
	@rm -f $(NAME)

re: fclean all

rl: re
	@./$(NAME)

ml: all
	@./$(NAME)

.PHONY: clean fclean re
