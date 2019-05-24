CXXFLAGS=--std=c++1y -I`$(LLVMCONFIG) --includedir`
LLVMCONFIG=llvm-config
LDLIBS=-lpthread -ldl -lcurses

basiccompiler: basiccompiler.o parser.o lexer.o
	$(CXX) $(LDFLAGS) -o $@ $^ `$(LLVMCONFIG) --ldflags` `$(LLVMCONFIG) --libs engine bitwriter` $(LDLIBS)
