NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS		= main.c \
			  args.c \
			  init.c \
			  coder.c \
			  coder_utils.c \
			  dongle_take.c \
			  monitor.c \
			  queue.c \
			  heap.c \
			  utils.c

OBJS		= $(SRCS:.c=.o)

HEADER		= codexion.h

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
