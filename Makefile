CPP=g++
# LDFLAGS+=
CPPFLAGS+=-std=c++11 -Wall -g --pedantic

all: bf-via-template

bf-via-template: BrainFuckCompileTime.cpp
	$(CPP) $^ $(CPPFLAGS) $(LDFLAGS) -o $@
