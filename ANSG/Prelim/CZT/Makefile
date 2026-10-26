# Builds the analysis classes in src/ into lib/, which the CZT dev shell puts
# on LD_LIBRARY_PATH (and include/ on the include paths). The Geant4 sim is
# built separately, with CMake, in build/.
CXX = $(shell root-config --cxx)
CXXFLAGS = $(shell root-config --cflags) -std=c++17 -Wall -Wextra -O2 -fPIC
ROOTLIBS = $(shell root-config --libs)
# Analysis-Utilities; the dev shell's compiler wrapper already knows its paths.
AULIBS = -lanalysis-utils

SRCDIR = src
INCDIR = include
OBJDIR = obj
LIBDIR = lib

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
HEADERS = $(wildcard $(INCDIR)/*.hh)

LIBNAME = libCZTAnalysis

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  SHAREDLIB = $(LIBDIR)/$(LIBNAME).dylib
  SHARED_FLAGS = -dynamiclib
else
  SHAREDLIB = $(LIBDIR)/$(LIBNAME).so
  SHARED_FLAGS = -shared
endif

STATICLIB = $(LIBDIR)/$(LIBNAME).a

all: $(SHAREDLIB) $(STATICLIB)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(SHAREDLIB): $(OBJECTS) | $(LIBDIR)
	$(CXX) $(SHARED_FLAGS) -o $@ $^ $(ROOTLIBS) $(AULIBS)
	@echo "Built shared library: $(SHAREDLIB)"

$(STATICLIB): $(OBJECTS) | $(LIBDIR)
	ar rcs $@ $^
	@echo "Built static library: $(STATICLIB)"


clean:
	rm -rf $(OBJDIR) $(LIBDIR)

.PHONY: all clean
