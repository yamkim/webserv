TARGET = webserv
WEBSERV_VERSION = 0.0.5
FILES = $(shell ls main | grep .cpp)
#FILES = main.cpp
#FILES += CGISession.cpp
#FILES += ConnectionSocket.cpp
#FILES += ErrorHandler.cpp
#FILES += FileController.cpp
#FILES += HTTPHandler.cpp
#FILES += HTTPRequestHandler.cpp
#FILES += HTTPResponseHandler.cpp
#FILES += KernelQueue.cpp
#FILES += ListeningSocket.cpp
#FILES += NginxConfig.cpp
#FILES += NginxParser.cpp
#FILES += Parser.cpp
#FILES += Socket.cpp
#FILES += Timer.cpp

SOURCE_PATH = ./main/
OBJ_PATH = ./objs/
HEADER = $(SOURCE_PATH)

CPP_CODES = $(addprefix $(SOURCE_PATH), $(FILES))
OBJS = $(subst $(SOURCE_PATH), $(OBJ_PATH), ${CPP_CODES:%.cpp=%.o})
COMPILER = clang++
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
CFLAGS += -DWEBSERV_VERSION=\"$(WEBSERV_VERSION)\"

GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
END = \033[0m

debug:
	echo $(CPP_CODES)
	echo $(OBJS)

all: $(TARGET)

clean:
	@echo "$(RED)Delete Object files...$(END)"
	rm -rf $(OBJ_PATH)

fclean: clean
	@echo "$(RED)Delete webserv...$(END)"
	rm -f $(TARGET)

$(OBJ_PATH)%.o: $(SOURCE_PATH)%.cpp
	@echo "$(YELLOW)generate object files ($< -> $@)$(END)" 1> /dev/null
	@mkdir -p $(OBJ_PATH)
	@$(COMPILER) $(CFLAGS) -I$(HEADER) -c -o $@ $<

$(TARGET): $(OBJS)
	@echo "$(YELLOW)webserv(normal) Build$(END)"
	@$(COMPILER) $(CFLAGS) $(OBJS) -I$(HEADER) -o $(TARGET)
	@echo "$(GREEN)Build Finish!$(END)"

re: fclean all
