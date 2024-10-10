NAME	:= demo

SRC_DIR	:= src/ActiveMQ_Demo
BIN_DIR := build/bin/
OBJ_DIR := build/obj/src/demo

CC 		:= g++
CFLAGS 	:= -pthread -luuid -lssl -lcrypto -lapr-1
INC 	:= -I /usr/include/apr-1.0/ -I /usr/local/include/activemq-cpp-3.10.0/ -lactivemq-cpp
SRCS	:= $(SRC_DIR)/consumer.cpp

demo:
	$(CC) $(CFLAGS) $(SRCS) $(INC) -o $(BIN_DIR)/$(NAME)

demo_clean:
	rm -f $(BIN_DIR)/$(NAME)
	rm -f $(OBJ_DIR)/*.o

demo_fclean: clean
	mkdir -p $(BIN_DIR) $(OBJ_DIR)