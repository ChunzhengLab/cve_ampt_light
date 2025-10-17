# Makefile for building AMPT analysis programs using ROOT

# Compiler settings
CXX      := g++
CXXFLAGS := -std=c++20 $(shell root-config --cflags)
LDFLAGS  := $(shell root-config --libs)

# Target executables
TARGET1  := analysis_cve
TARGET2  := analysis_lambda_feeddown

SRCS1    := analysis_cve.cxx
SRCS2    := analysis_lambda_feeddown.cxx

.PHONY: all clean feeddown

# Build all targets
all: $(TARGET1) $(TARGET2)

# Build only feeddown analysis
feeddown: $(TARGET2)

$(TARGET1): $(SRCS1)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $(TARGET1)"

$(TARGET2): $(SRCS2)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $(TARGET2)"

clean:
	rm -f $(TARGET1) $(TARGET2)
