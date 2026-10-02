#pragma once

#include "raylib.h"

// Câmera de visão de cima sobre o mapa. Trabalha em coordenadas de mundo
// (pixels com zoom 1) e não sabe nada de células: o mapa só informa o
// tamanho total para que a câmera não saia dele.
class CameraJogo {
public:
    CameraJogo(float largura_mundo, float altura_mundo);

    // Teclado (WASD/setas), roda do mouse (zoom) e botão do meio ou direito
    // arrastando (pan).
    void atualizar(float dt);

    const Camera2D& camera2d() const { return camera_; }
    Vector2 mouse_no_mundo() const;
    // Retângulo do mundo visível na tela, usado para desenhar só o necessário.
    Rectangle area_visivel() const;

private:
    void limitar_ao_mundo();

    Camera2D camera_{};
    float largura_mundo_;
    float altura_mundo_;
};
