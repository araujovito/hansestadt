# Caminhos relativos de propósito: o make não lida bem com espaços, e o
# projeto pode estar numa pasta com espaço no nome.
RAYLIB  := vendor/raylib/src
LIBRAY  := $(RAYLIB)/libraylib.a
TOMLPP  := vendor/tomlplusplus
TOMLHPP := $(TOMLPP)/toml.hpp
# Arte da Kenney (CC0), baixada por script como as bibliotecas; as peças
# próprias ficam em dados/arte, geradas por ferramentas/gerar_pecas.py.
KENNEY  := vendor/kenney/tiny-farm/tilemap_packed.png

CXX      ?= g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Isrc -isystem $(RAYLIB) -isystem $(TOMLPP)
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

# Testes: só a lógica do jogo, sem raylib nem janela, sempre com sanitizers.
# Estes arquivos não podem incluir raylib.h; é isso que mantém a simulação
# testável.
LOGICA        := alertas campanha calendario cidade conteudo eventos fundacao mapa partidas populacao producao relatorio rotas ruido salvamento simulacao transporte
TESTE_FLAGS   := -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Isrc -Itestes -isystem $(TOMLPP) -O1 -g \
                 -fsanitize=address,undefined -fno-omit-frame-pointer
TESTE_FONTES  := $(wildcard testes/*.cpp) $(LOGICA:%=src/%.cpp)
TESTE_BINARIO := build/testes/testes

.PHONY: all run debug test balance clean

all: $(BINARIO)

$(LIBRAY):
	./scripts/baixar-raylib.sh

$(TOMLHPP):
	./scripts/baixar-tomlplusplus.sh

$(KENNEY):
	./scripts/baixar-kenney.sh

$(BINARIO): $(OBJETOS) $(LIBRAY) | $(KENNEY)
	$(CXX) $(LDFLAGS) $(OBJETOS) $(LDLIBS) -o $@

# -MMD gera os .d com as dependências de cabeçalho de cada .cpp
$(SAIDA)/%.o: src/%.cpp | $(LIBRAY) $(TOMLHPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(BINARIO)
	./$(BINARIO)

debug:
	$(MAKE) MODO=debug run

$(TESTE_BINARIO): $(TESTE_FONTES) $(wildcard src/*.hpp testes/*.hpp) | $(TOMLHPP)
	@mkdir -p $(dir $@)
	$(CXX) $(TESTE_FLAGS) $(TESTE_FONTES) -o $@

test: $(TESTE_BINARIO)
	./$(TESTE_BINARIO)

balance:
	python3 ferramentas/balancear.py

clean:
	rm -rf build

-include $(OBJETOS:.o=.d)
