NAME		= codexion

SRC_DIR		= src
INC_DIR		= include
OBJ_DIR		= obj

SRCS		= main.c parsing.c init.c heap.c heap_internal.c heap_remove.c \
			  dongle.c dongle_queue.c coder.c monitor.c utils.c
OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
DEPS		= $(OBJS:.o=.d)

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
CPPFLAGS	= -I$(INC_DIR) -MMD -MP

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus:

.PHONY: all clean fclean re bonus
