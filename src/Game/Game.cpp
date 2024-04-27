#include "Game.h"

#include "../Renderer/ShaderProgram.h"
#include "../Renderer/Sprite.h"
#include "../Renderer/Texture2D.h"
#include "../Resources/ResourceManager.h"

#include "GameObjects/Bullet.h"
#include "GameObjects/Tank.h"

#include "../Physics/PhysicsEngine.h"
#include "../Renderer/Renderer.h"
#include "GameStates/Level.h"
#include "GameStates/StartScreen.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>


std::array<bool, 349> Game::m_keys;
glm::uvec2 Game::m_windowSize;
Game::EGameState Game::m_eCurrentGameState;
std::shared_ptr<IGameState> Game::m_pCurrentGameState;
std::shared_ptr<RenderEngine::ShaderProgram> Game::m_pSpriteShaderProgram;
size_t Game::m_currentLevelIndex;
Timer Game::m_changeScreenTimer;

void Game::initStartScreen(glm::uvec2 &windowSize) {
    m_windowSize = windowSize;
}

void Game::setWindowSize(glm::uvec2 &windowSize) {
    m_windowSize = windowSize;
    updateViewport();
}

Game::Game() {
}

Game::~Game() {}

void Game::render() { m_pCurrentGameState->render(); }

void Game::updateViewport() {
    const float level_aspect_ratio = static_cast<float>(getCurrentWidth()) / getCurrentHeight();
    unsigned int viewPortWidth = m_windowSize.x;
    unsigned int viewPortHeight = m_windowSize.y;
    unsigned int viewPortLeftOffset = 0;
    unsigned int viewPortBottomOffset = 0;

    if (static_cast<float>(m_windowSize.x) / m_windowSize.y > level_aspect_ratio) {
        viewPortWidth = static_cast<unsigned int>(m_windowSize.y * level_aspect_ratio);
        viewPortLeftOffset = (m_windowSize.x - viewPortWidth) / 2;
    } else {
        viewPortHeight = static_cast<unsigned int>(m_windowSize.x / level_aspect_ratio);
        viewPortBottomOffset = (m_windowSize.y - viewPortHeight) / 2;
    }

    RenderEngine::Renderer::setViewport(viewPortWidth,
                                        viewPortHeight,
                                        viewPortLeftOffset,
                                        viewPortBottomOffset);

    glm::mat4 projectionMatrix = glm::ortho(0.f, static_cast<float>(getCurrentWidth()),
                                            0.f, static_cast<float>(getCurrentHeight()),
                                            -100.f, 100.f);
    m_pSpriteShaderProgram->setMatrix4("projectionMat", projectionMatrix);
}

void Game::startNewLevel(const size_t level, const EGameMode eGameMode) {
    m_currentLevelIndex = level;
    auto pLevel = std::make_shared<Level>(ResourceManager::getLevels()[m_currentLevelIndex], eGameMode);
    m_pCurrentGameState = pLevel;
    Physics::PhysicsEngine::setCurrentLevel(pLevel);
    updateViewport();
}

void Game::nextLevel(const EGameMode eGameMode) {
    m_changeScreenTimer.setCallback([&]{
        startNewLevel(++m_currentLevelIndex, eGameMode);

    });
    m_changeScreenTimer.start(1000);
}

void Game::update(const double delta) {
    if (m_changeScreenTimer.isActive()){
        m_changeScreenTimer.update(delta);
    }
    else {
        m_pCurrentGameState->processInput(m_keys);
        m_pCurrentGameState->update(delta);
    }
}

void Game::setKey(const int key, const int action) { m_keys[key] = action; }

void Game::setStartScreen() {
    m_changeScreenTimer.setCallback([&]{
        m_pCurrentGameState = std::make_shared<StartScreen>(ResourceManager::getStartScreen());
        m_eCurrentGameState = EGameState::StartScreen;
        m_currentLevelIndex = 0;
    });
    m_changeScreenTimer.start(1000);
}

bool Game::init() {
    ResourceManager::loadJSONResources("res/resources.json");

    m_pSpriteShaderProgram = ResourceManager::getShaderProgram("spriteShader");
    if (!m_pSpriteShaderProgram) {
        std::cerr << "Can't find shader program: " << "spriteShader" << std::endl;
        return false;
    }
    m_pSpriteShaderProgram->use();
    m_pSpriteShaderProgram->setInt("tex", 0);

    m_pCurrentGameState = std::make_shared<StartScreen>(ResourceManager::getStartScreen());
    m_eCurrentGameState = EGameState::StartScreen;
    m_currentLevelIndex = 0;
    m_keys.fill(false);
    setWindowSize(m_windowSize);

    return true;
}

unsigned int Game::getCurrentWidth() {
    return m_pCurrentGameState->getStateWidth();
}

unsigned int Game::getCurrentHeight() {
    return m_pCurrentGameState->getStateHeight();
}
