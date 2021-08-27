build:
	g++ src/*.cpp -std=c++20 -lzip -lcurl -o tldr

install: 
	cp ./tldr $(DESTDIR)/usr/bin/
