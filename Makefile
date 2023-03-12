CC = g++

CFLAGS = -Werror\
		-std=c++11\
		-stdlib=libc++\
		-g\
	 	-I/opt/homebrew/Cellar/boost/1.81.0_1/include\
		-D_WEBSOCKETPP_CPP11_STL_\
		-L/opt/homebrew/opt/openssl@3/lib\
		-I/opt/homebrew/opt/openssl@3/include\
		-O3

		
#-Werror\
#-L/opt/homebrew/Cellar/boost/1.81.0_1/lib
# -lboost_system~\
# -lboost_thread-mt\
# -L/opt/homebrew/Cellar/boost/1.81.0_1/lib\
		# -lboost_system\
		# -lboost_thread-mt\

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
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# clean target
clean:
	rm -f $(EXEC) $(OBJS)