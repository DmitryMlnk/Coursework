#pragma once

#include <array>
#include <glm/vec2.hpp>
#include <memory>
#include "../Renderer/Timer.h"

class IGameState;

namespace RenderEngine {
    class ShaderProgram;
}

class Game {
public:
    enum class EGameMode {
        OnePlayer, TwoPlayers
    };

    Game();
    ~Game();

    static void render();

    static void update(const double delta);

    static void setKey(const int key, const int action);

    static bool init();

    static void initStartScreen(glm::uvec2 &windowSize);

    static unsigned int getCurrentWidth();

    static unsigned int getCurrentHeight();

    static void startNewLevel(const size_t level, const EGameMode eGameMode);

    static void nextLevel(const EGameMode eGameMode);

    static void updateViewport();

    static void setStartScreen();

    static void setWindowSize(glm::uvec2 &windowSize);

private:
    enum class EGameState {
        StartScreen,
        LevelStart,
        Level,
        Pause,
        Scores,
        GameOver
    };

    static std::array<bool, 349> m_keys;

    static glm::uvec2 m_windowSize;
    static EGameState m_eCurrentGameState;

    static Timer m_changeScreenTimer;
    static std::shared_ptr<IGameState> m_pCurrentGameState;
    static std::shared_ptr<RenderEngine::ShaderProgram> m_pSpriteShaderProgram;
    static size_t m_currentLevelIndex;
};