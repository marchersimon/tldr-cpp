build:
	g++ src/*.cpp -std=c++17 -lzip -lcurl -o tldr

install: 
	cp ./tldr /usr/loca/bin/
