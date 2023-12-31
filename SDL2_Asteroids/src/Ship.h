#pragma once
#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "SpaceObject.h"
#include "Asteroid.h"

using std::vector, std::pair, std::make_pair;

class Ship 
{
public:
    Ship(const glm::vec2& position, const SDL_Color& color);

    void ProcessInput();
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    
    void Clean();

   const glm::vec2& GetPosition() { return mShip.mPosition; }
    SpaceObject& GetSpaceObject() { return mShip; }

    void MoveShip(float deltaTime);
    void MoveBullet(SpaceObject& bullet, float deltaTime);

	bool IsCollidingWithAsteroid( Asteroid* asteroid );

    void HaltAllSounds();

    bool GetIsDead() const noexcept { return mIsDead; }

    void SetIsDead( bool isDead ) {mIsDead = isDead;}

	void LoadAndSetSFX();

    void CheckAsteroidsCollision();

    void UpdateBullets(float deltaTime);

private:

    glm::vec2 GetShipForwardVector() const
	{
		return { sinf( mShip.mRotation ), -cosf( mShip.mRotation ) };
	}

    void SpawnBullet();
private:
    std::vector<std::pair<float, float>> vecModelShip;
    SpaceObject mShip;    
    SDL_Color mColor;

    bool mIsDead;

    float mAccelerationFactor;
    static const float mRotationSpeed;

    std::vector<SpaceObject> mBulletsVec;
    float mBulletSpeed;

    Mix_Chunk* mHoverSound;
    int mHoverChannel;
    
    Mix_Chunk* mLaserSound;
    int mLaserChannel;
    
    Mix_Chunk* mDeadSound;
    int mDeadChannel;
    
    Mix_Chunk* mAsteroidHitSound;
    int mAsteroidHitChannel;

    // Distance from the center of the ship to the top point
    static const float topPointOffset; 
};
