PREFIX = /usr

CXX ?= g++
CXXFLAGS += -std=c++20 -Wall -Os
LDFLAGS += -lcurl -lzip

tldr: clean
	$(CXX) $(CXXFLAGS) src/*.cpp src/find/*.cpp -D_MANUAL_INSTALL_ $(LDFLAGS) -o tldr

install: tldr
	install -Dm755 tldr $(DESTDIR)$(PREFIX)/bin/

tldr-pkg:
	$(CXX) $(CXXFLAGS) src/*.cpp src/find/*.cpp $(LDFLAGS) -o tldr

install-pkg:
	install -Dm755 tldr $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/tldr

clean:
	-rm tldr