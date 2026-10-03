#include "camera.hpp"

#include <algorithm>

#include "raymath.h"

namespace {

constexpr float ZOOM_MIN = 0.25f;
constexpr float ZOOM_MAX = 3.0f;
constexpr float VELOCIDADE_PAN = 700.0f;  // pixels de tela por segundo
constexpr float PASSO_ZOOM = 1.15f;       // multiplicador por clique da roda

}  // namespace

CameraJogo::CameraJogo(float largura_mundo, float altura_mundo)
    : largura_mundo_(largura_mundo), altura_mundo_(altura_mundo) {
    camera_.zoom = 1.0f;
    camera_.target = { largura_mundo / 2.0f, altura_mundo / 2.0f };
}

void CameraJogo::centralizar(Vector2 ponto_mundo) {
    camera_.target = ponto_mundo;
    limitar_ao_mundo();
}

void CameraJogo::atualizar(float dt) {
    // O offset mantém o alvo da câmera no centro da tela, inclusive quando a
    // janela é redimensionada.
    camera_.offset = { static_cast<float>(GetScreenWidth()) / 2.0f,
                       static_cast<float>(GetScreenHeight()) / 2.0f };

    Vector2 direcao{ 0.0f, 0.0f };
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direcao.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direcao.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direcao.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direcao.y += 1.0f;
    // Dividir pelo zoom mantém a velocidade constante na tela: afastado, a
    // câmera percorre mais mundo por segundo.
    camera_.target = Vector2Add(
        camera_.target, Vector2Scale(Vector2Normalize(direcao), VELOCIDADE_PAN * dt / camera_.zoom));

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        camera_.target = Vector2Subtract(camera_.target, Vector2Scale(GetMouseDelta(), 1.0f / camera_.zoom));
    }

    const float roda = GetMouseWheelMove();
    if (roda != 0.0f) {
        // Zoom em direção ao cursor: o ponto do mundo sob o mouse continua sob
        // o mouse depois do zoom, como em editores de mapa e no Google Maps.
        const Vector2 antes = mouse_no_mundo();
        camera_.zoom = std::clamp(camera_.zoom * (roda > 0 ? PASSO_ZOOM : 1.0f / PASSO_ZOOM), ZOOM_MIN, ZOOM_MAX);
        const Vector2 depois = mouse_no_mundo();
        camera_.target = Vector2Add(camera_.target, Vector2Subtract(antes, depois));
    }

    limitar_ao_mundo();
}

Vector2 CameraJogo::mouse_no_mundo() const {
    return GetScreenToWorld2D(GetMousePosition(), camera_);
}

Rectangle CameraJogo::area_visivel() const {
    const Vector2 canto = GetScreenToWorld2D({ 0.0f, 0.0f }, camera_);
    return { canto.x, canto.y,
             static_cast<float>(GetScreenWidth()) / camera_.zoom,
             static_cast<float>(GetScreenHeight()) / camera_.zoom };
}

void CameraJogo::limitar_ao_mundo() {
    camera_.target.x = std::clamp(camera_.target.x, 0.0f, largura_mundo_);
    camera_.target.y = std::clamp(camera_.target.y, 0.0f, altura_mundo_);
}
