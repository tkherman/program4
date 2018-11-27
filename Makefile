#Name: Kwan Ho Herman Tong, Jessica Hardey, Josefa Osorio
#Netid: ktong1, jhardey, josorio2

CXX=		g++
CXFLAGS=	-g -std=c++11 -lpthread
TARGETS=	netpong

all: $(TARGETS)

netpong: netpong.o client.o server.o
	$(CXX) $(CXFLAGS) -lncurses -o $@ $^

netpong.o: netpong.cpp 
	$(CXX) $(CXFLAGS) -lncurses -o $@ -c $^

client.o: client.cpp client.h
	$(CXX) $(CXFLAGS) -o $@ -c $<

server.o: server.cpp server.h
	$(CXX) $(CXFLAGS) -o $@ -c $<

clean:
	rm $(TARGETS) *.o
