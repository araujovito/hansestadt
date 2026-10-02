#include "verificar.hpp"

int main() {
    for (const auto& caso : testes::registro()) {
        const int antes = testes::falhas;
        caso.corpo();
        std::printf("%s %s\n", testes::falhas == antes ? "ok   " : "FALHA", caso.nome);
    }
    std::printf("\n%zu testes, %d verificações falharam\n", testes::registro().size(), testes::falhas);
    return testes::falhas == 0 ? 0 : 1;
}
