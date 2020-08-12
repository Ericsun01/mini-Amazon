CC = g++
CFLAGS = -std=c++11 -Wall -pedantic
LDFLAGS = -lpthread -pthread
GPBFLAGS = `pkg-config --cflags --libs protobuf`
EXTRAFLAGS = -lpqxx -lpq


amazon: connection.cpp world_amazon.pb.cc amazon_ups.pb.cc
	$(CC) $(CFLAGS) $(LDFLAGS) -o amazon connection.cpp world_amazon.pb.cc amazon_ups.pb.cc $(EXTRAFLAGS) $(GPBFLAGS)

clean:
	rm -f *~ *.o amazon

