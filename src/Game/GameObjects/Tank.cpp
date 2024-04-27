#include "Tank.h"

#include "../../Physics/PhysicsEngine.h"
#include "../../Renderer/Sprite.h"
#include "../../Resources/ResourceManager.h"
#include "Bullet.h"
#include "iostream"

const std::string &Tank::getTankSpriteFromType(const ETankType eType) {
    return TankTypeToSpriteString[static_cast<size_t>(eType)];
}

const std::string &Tank::getTankSpriteFromExplosion(const ETankExplosion eExplosion) {
    return TankExplosionToSpriteString[static_cast<size_t>(eExplosion)];
}

Tank::Tank(const Tank::ETankType eType, const bool bHasAI,
           const bool bShieldOnSpawn, const EOrientation eOrientation,
           const double maxVelocity, const glm::vec2 &position,
           const glm::vec2 &size, const float layer)
        : IGameObject(IGameObject::EObjectType::Tank, position, size, 0.f, layer),
          m_eOrientation(eOrientation),
          m_pCurrentBullet(std::make_shared<Bullet>(0.1, m_position + m_size / 4.f,
                                                    m_size / 2.f, m_size, layer)),
          m_pBigExplosionBottomLeft(
                  ResourceManager::getSprite(getTankSpriteFromExplosion(ETankExplosion::Bottom_left))),
          m_pBigExplosionBottomRight(
                  ResourceManager::getSprite(getTankSpriteFromExplosion(ETankExplosion::Bottom_right))),
          m_pBigExplosionTopLeft(ResourceManager::getSprite(getTankSpriteFromExplosion(ETankExplosion::Top_left))),
          m_pBigExplosionTopRight(ResourceManager::getSprite(getTankSpriteFromExplosion(ETankExplosion::Top_right))),
          m_spriteBigExplosionBottomLeft(m_pBigExplosionBottomLeft),
          m_spriteBigExplosionBottomRight(m_pBigExplosionBottomRight),
          m_spriteBigExplosionTopLeft(m_pBigExplosionTopLeft),
          m_spriteBigExplosionTopRight(m_pBigExplosionTopRight),
          m_pSprite_top(ResourceManager::getSprite(getTankSpriteFromType(eType) + "_top")),
          m_pSprite_bottom(ResourceManager::getSprite(getTankSpriteFromType(eType) + "_bottom")),
          m_pSprite_left(ResourceManager::getSprite(getTankSpriteFromType(eType) + "_left")),
          m_pSprite_right(ResourceManager::getSprite(getTankSpriteFromType(eType) + "_right")),
          m_spriteAnimator_top(m_pSprite_top),
          m_spriteAnimator_bottom(m_pSprite_bottom),
          m_spriteAnimator_left(m_pSprite_left),
          m_spriteAnimator_right(m_pSprite_right),
          m_pSprite_respawn(ResourceManager::getSprite("respawn")),
          m_spriteAnimator_respawn(m_pSprite_respawn),
          m_pSprite_shield(ResourceManager::getSprite("shield")),
          m_spriteAnimator_shield(m_pSprite_shield), m_maxVelocity(maxVelocity),
          m_isSpawning(true),
          m_hasShield(false),
          m_eTankType(eType),
          m_bShieldOnSpawn(bShieldOnSpawn) {
    setOrientation(m_eOrientation);
    checkTankType();

    m_respawnTimer.setCallback([&]() {
        m_isSpawning = false;
        if (m_pAIComponent) {
            m_velocity = m_maxVelocity;
        }
        if (m_bShieldOnSpawn) {
            m_hasShield = true;
            m_shieldTimer.start(2000);
        }
    });

    auto onCollisionCallback = [&](const IGameObject &object, const Physics::ECollisionDirection) {
        setVelocity(0);
        m_isExplosion = true;
        m_explosionTimer.start(m_spriteBigExplosionTopRight.getTotalDuration());
    };
    m_colliders.emplace_back(glm::vec2(0), m_size, onCollisionCallback);

    m_explosionTimer.setCallback([&]() {
        m_maxVelocity = 0;
        m_isExplosion = false;
        m_isActive = false;
        m_spriteBigExplosionTopRight.reset();
        m_spriteBigExplosionTopLeft.reset();
        m_spriteBigExplosionBottomRight.reset();
        m_spriteBigExplosionBottomLeft.reset();
    });

    if (m_eTankType == ETankType::EnemyRed_type4) {
        m_hasMoreHP = true;
        m_isRed = true;
    } else if (m_eTankType == ETankType::EnemyRed_type3
               or m_eTankType == ETankType::EnemyRed_type2
               or m_eTankType == ETankType::EnemyRed_type1) {
        m_isRed = true;
        m_hasMoreHP = false;
    } else if (m_eTankType == ETankType::EnemyGreen_type4) {
        m_isGreen = true;
        m_hasMoreHP = true;
    } else if (m_eTankType == ETankType::EnemyGreen_type3
               or m_eTankType == ETankType::EnemyGreen_type2
               or m_eTankType == ETankType::EnemyGreen_type1) {
        m_isGreen = true;
        m_hasMoreHP = false;
    } else if (m_eTankType == ETankType::EnemyWhite_type4) {
        m_isWhite = true;
        m_hasMoreHP = true;
    } else if (m_eTankType == ETankType::EnemyWhite_type3
               or m_eTankType == ETankType::EnemyWhite_type2
               or m_eTankType == ETankType::EnemyWhite_type1) {
        m_isWhite = true;
        m_hasMoreHP = false;
    }

    m_isActive = true;

    m_respawnTimer.start(1500);

    m_shieldTimer.setCallback([&]() { m_hasShield = false; });

    m_colliders.emplace_back(glm::vec2(0), m_size);

    m_pCurrentBullet->setOwner(this);
    Physics::PhysicsEngine::addDynamicGameObject(m_pCurrentBullet);

    if (bHasAI) {
        m_pAIComponent = std::make_unique<TankAI>(this);
    }
}

void Tank::setVelocity(const double velocity) {
    if (!m_isSpawning) {
        m_velocity = velocity;
    }
}

void Tank::render() const {
    if (m_isActive) {
        if (m_isSpawning) {
            m_pSprite_respawn->render(m_position, m_size, m_rotation, m_layer,
                                      m_spriteAnimator_respawn.getCurrentFrame());
        } else {
            switch (m_eOrientation) {
                case Tank::EOrientation::Top:
                    m_pSprite_top->render(m_position, m_size, m_rotation, m_layer,
                                          m_spriteAnimator_top.getCurrentFrame());
                    break;
                case Tank::EOrientation::Bottom:
                    m_pSprite_bottom->render(m_position, m_size, m_rotation, m_layer,
                                             m_spriteAnimator_bottom.getCurrentFrame());
                    break;
                case Tank::EOrientation::Left:
                    m_pSprite_left->render(m_position, m_size, m_rotation, m_layer,
                                           m_spriteAnimator_left.getCurrentFrame());
                    break;
                case Tank::EOrientation::Right:
                    m_pSprite_right->render(m_position, m_size, m_rotation, m_layer,
                                            m_spriteAnimator_right.getCurrentFrame());
                    break;
            }

            if (m_hasShield) {
                m_pSprite_shield->render(m_position, m_size, m_rotation, m_layer + 0.1f,
                                         m_spriteAnimator_shield.getCurrentFrame());
            }
        }
    } else if (m_isExplosion) {
        m_pBigExplosionTopRight->render(m_position + glm::vec2(8, 8), m_size, m_rotation, m_layer + 0.1f,
                                        m_spriteBigExplosionTopRight.getCurrentFrame());
        m_pBigExplosionTopLeft->render(m_position + glm::vec2(-8, 8), m_size, m_rotation, m_layer + 0.1f,
                                       m_spriteBigExplosionTopLeft.getCurrentFrame());
        m_pBigExplosionBottomRight->render(m_position + glm::vec2(8, -8), m_size, m_rotation, m_layer + 0.1f,
                                           m_spriteBigExplosionBottomRight.getCurrentFrame());
        m_pBigExplosionBottomLeft->render(m_position + glm::vec2(-8, -8), m_size, m_rotation, m_layer + 0.1f,
                                          m_spriteBigExplosionBottomLeft.getCurrentFrame());
    }

    m_pCurrentBullet->render();

}

void Tank::setOrientation(const EOrientation eOrientation) {
    m_eOrientation = eOrientation;
    switch (m_eOrientation) {
        case Tank::EOrientation::Top:
            m_direction.x = 0.f;
            m_direction.y = 1.f;
            break;
        case Tank::EOrientation::Bottom:
            m_direction.x = 0.f;
            m_direction.y = -1.f;
            break;
        case Tank::EOrientation::Left:
            m_direction.x = -1.f;
            m_direction.y = 0.f;
            break;
        case Tank::EOrientation::Right:
            m_direction.x = 1.f;
            m_direction.y = 0.f;
            break;
    }
}

void Tank::update(const double delta) {
    m_pCurrentBullet->update(delta);

    if (m_isSpawning) {
        m_spriteAnimator_respawn.update(delta);
        m_respawnTimer.update(delta);
    } else if (m_isActive) {
        if (m_pAIComponent) {
            m_pAIComponent->update(delta);
        }

        if (m_hasShield) {
            m_spriteAnimator_shield.update(delta);
            m_shieldTimer.update(delta);
        }

        if (m_velocity > 0) {
            switch (m_eOrientation) {
                case Tank::EOrientation::Top:
                    m_spriteAnimator_top.update(delta);
                    break;
                case Tank::EOrientation::Bottom:
                    m_spriteAnimator_bottom.update(delta);
                    break;
                case Tank::EOrientation::Left:
                    m_spriteAnimator_left.update(delta);
                    break;
                case Tank::EOrientation::Right:
                    m_spriteAnimator_right.update(delta);
                    break;
            }
        }
    } else if (m_isExplosion) {
        m_explosionTimer.update(delta);
    }
}

void Tank::checkTankType() {
    if (m_isRed and m_hasMoreHP) {
        loadTankType(m_eTankType);
        std::cout << "Red More HP" << std::endl;
        m_hasMoreHP = false;
    } else if (m_isRed) {
        loadTankType(m_eTankType);
        std::cout << "Red low HP" << std::endl;
        m_isRed = false;
        m_isGreen = true;
        m_hasMoreHP = true;
    } else if (m_isGreen and m_hasMoreHP) {
        std::cout << "Green More HP" << std::endl;
        loadTankType(m_eTankType);
        m_hasMoreHP = false;
    } else if (m_isGreen) {
        loadTankType(m_eTankType);
        std::cout << "Green low HP" << std::endl;
        m_isGreen = false;
        m_isWhite = true;
        m_hasMoreHP = true;
    } else if (m_isWhite and m_hasMoreHP) {
        loadTankType(m_eTankType);
        std::cout << "White More HP" << std::endl;
        m_hasMoreHP = false;
    } else if (m_isWhite) {
        loadTankType(m_eTankType);
        std::cout << "White low HP" << std::endl;
        m_isWhite = false;
    } else {
        std::cout << "Fatal hp" << std::endl;
        m_isActive = false;
        m_isExplosion = true;
        m_velocity = 0;
        m_explosionTimer.start(m_spriteBigExplosionTopRight.getTotalDuration());
    }
}

void Tank::loadTankType(Tank::ETankType &tankType) {
    m_pSprite_top = ResourceManager::getSprite(getTankSpriteFromType(tankType) + "_top");
    m_pSprite_bottom = ResourceManager::getSprite(getTankSpriteFromType(tankType) + "_bottom");
    m_pSprite_left = ResourceManager::getSprite(getTankSpriteFromType(tankType) + "_left");
    m_pSprite_right = ResourceManager::getSprite(getTankSpriteFromType(tankType) + "_right");
    m_spriteAnimator_top.reset();
    m_spriteAnimator_bottom.reset();
    m_spriteAnimator_left.reset();
    m_spriteAnimator_left.reset();
    m_spriteAnimator_top = m_pSprite_top;
    m_spriteAnimator_bottom = m_pSprite_bottom;
    m_spriteAnimator_left = m_pSprite_left;
    m_spriteAnimator_right = m_pSprite_right;
}

void Tank::explosion() {
    checkTankType();
}

void Tank::fire() {
    if (!m_isSpawning && !m_pCurrentBullet->isActive()) {
        m_pCurrentBullet->fire(m_position + m_size / 4.f + m_size * m_direction / 4.f, m_direction);
    }
}
