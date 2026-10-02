#pragma once

#include <cstdio>
#include <functional>
#include <vector>

// Mini-framework de testes: o projeto ainda não justifica uma dependência
// externa (Catch2, doctest) só para meia dúzia de asserts.
namespace testes {

struct Caso {
    const char* nome;
    std::function<void()> corpo;
};

inline std::vector<Caso>& registro() {
    static std::vector<Caso> casos;
    return casos;
}

inline int falhas = 0;

struct Registrar {
    Registrar(const char* nome, std::function<void()> corpo) { registro().push_back({ nome, std::move(corpo) }); }
};

}  // namespace testes

#define TESTE_CONCAT2(a, b) a##b
#define TESTE_CONCAT(a, b) TESTE_CONCAT2(a, b)
#define TESTE(nome)                                                                  \
    static void TESTE_CONCAT(teste_, __LINE__)();                                    \
    static testes::Registrar TESTE_CONCAT(registro_, __LINE__)(nome, TESTE_CONCAT(teste_, __LINE__)); \
    static void TESTE_CONCAT(teste_, __LINE__)()

#define VERIFICAR(expr)                                                              \
    do {                                                                             \
        if (!(expr)) {                                                               \
            std::printf("    falhou: %s (%s:%d)\n", #expr, __FILE__, __LINE__);    \
            ++testes::falhas;                                                        \
        }                                                                            \
    } while (0)
