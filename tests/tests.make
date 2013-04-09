DUETTO_COMPILER=/opt/duetto/bin/duetto-compiler
CLANG=/opt/duetto/bin/clang

%.js: %.bc
	${DUETTO_COMPILER} $^

%.bc: %.cpp
	${CLANG} -emit-llvm -m32 -c $^ -o $@