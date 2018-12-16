SHELL := /bin/bash

CXXFLAGS := -flto -std=c++17 -Wall -Wextra -O3 -Ihopscotch-map/include

LDLIBS := -lboost_iostreams

all:: catwords jeffs

clean::
	rm -f catwords jeffs
