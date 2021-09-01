build:
	g++ -Wall src/*.cpp src/find/*.cpp -std=c++20 -lzip -lcurl -o tldr

install: 
	cp ./tldr $(DESTDIR)/usr/bin/
