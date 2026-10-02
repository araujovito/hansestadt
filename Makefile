# Caminhos relativos de propósito: o make não lida bem com espaços, e o
# projeto pode estar numa pasta com espaço no nome.
RAYLIB  := vendor/raylib/src
LIBRAY  := $(RAYLIB)/libraylib.a

CXX      ?= g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Isrc -isystem $(RAYLIB)
LDLIBS   := $(LIBRAY) -lGL -lm -lpthread -ldl -lrt -lX11

FONTES := $(wildcard src/*.cpp)

# Release e debug compilam em pastas separadas para não misturar objetos
# com e sem sanitizers.
MODO ?= release
ifeq ($(MODO),debug)
    CXXFLAGS += -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS  += -fsanitize=address,undefined
else
    CXXFLAGS += -O2 -DNDEBUG
endif

SAIDA   := build/$(MODO)
OBJETOS := $(FONTES:src/%.cpp=$(SAIDA)/%.o)
BINARIO := $(SAIDA)/hansestadt

.PHONY: all run debug clean

all: $(BINARIO)

$(LIBRAY):
	./scripts/baixar-raylib.sh

$(BINARIO): $(OBJETOS) $(LIBRAY)
	$(CXX) $(LDFLAGS) $(OBJETOS) $(LDLIBS) -o $@

# -MMD gera os .d com as dependências de cabeçalho de cada .cpp
$(SAIDA)/%.o: src/%.cpp | $(LIBRAY)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(BINARIO)
	./$(BINARIO)

debug:
	$(MAKE) MODO=debug run

clean:
	rm -rf build

-include $(OBJETOS:.o=.d)
