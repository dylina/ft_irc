.SILENT:

NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = Socket.cpp Server.cpp main.cpp Client.cpp Channel.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
		$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)
		echo "Compilation done ! with FLAGS :" '`'$(CXXFLAGS)'`'

clean:
		rm -f $(OBJ)
# 		delete fifo for debug tester
		rm -f /tmp/ircserv_dump

fclean: clean
		rm -f $(NAME)
# 		delete .log for debug tester
		rm -f ../tester/client/result.log
		echo "All files are clean up !"

re: fclean all

.PHONY: all clean fclean re
