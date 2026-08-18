CC = gcc
CFLAGS = -MMD -MP
OBJS := bmp.o
NAME := bmp

# Link
$(NAME): $(OBJS)
	$(CC) -o $@ $^ -lm

# pull in dependency info
-include $(OBJS:.o=.d)

# compile and generate dependency info
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# remove compilation products
clean:
	rm -f $(TESTNAME) $(TESTOBJS) $(TESTOBJS:.o=.d) $(NAME) $(OBJS) $(OBJS:.o=.d)
