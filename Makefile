CC = g++

CFLAGS = -Wall\
		-std=c++11\
		-stdlib=libc++\
		-g\
	 	-I/opt/homebrew/Cellar/boost/1.81.0_1/include\
		-I/opt/homebrew/opt/openssl@3/include\
		-D_WEBSOCKETPP_CPP11_STL_\
		-O3

FINAL_FLAGS = $(CFLAGS)

FINAL_FLAGS += -lboost_thread-mt\
			-lboost_system-mt\
			-L/opt/homebrew/opt/openssl@3/lib\
			-L/opt/homebrew/Cellar/boost/1.81.0_1/lib\
			-lcrypto\
			-lssl
			
SRCS =\
	network/connection_metadata.cpp\
	network/websocket_endpoint.cpp\
	main.cpp
	
OBJS = $(SRCS:.cpp=.o)	

EXEC = connect

all: $(EXEC)

build:
	make clean
	make all

$(EXEC): $(OBJS)
	$(CC) $(FINAL_FLAGS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# clean target
clean:
	rm -f $(EXEC) $(OBJS)