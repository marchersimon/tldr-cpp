PREFIX = /usr

CXX ?= g++
CXXFLAGS += -std=c++20 -Wall
LDFLAGS += -lcurl -lzip

tldr:
	$(CXX) $(CXXFLAGS) src/*.cpp src/find/*.cpp $(LDFLAGS) -o tldr

install: tldr
	install -Dm755 tldr $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/tldr

clean:
	-rm tldr