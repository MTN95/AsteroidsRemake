#pragma once
#include <unordered_map>

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "TextRenderer.hpp"
#include "SpaceObject.h"
#include "Asteroid.h"
#include "Timer.h"
#include "Ship.h"

// The Window's width
const int SCREEN_WIDTH = 800;
// The Window's height
const int SCREEN_HEIGHT = 600;

using namespace venture;

class Game
{
public:

	/**
	* @brief Returns the game class instance
	*/
	static inline Game* GetInstance()
	{
		return s_Instance = ( s_Instance != nullptr ) ? s_Instance : new Game();
	}

	/**
	* @brief Initializes the game, and it's objects
	*/
	bool Init( const char* title, bool fullscreen );

	/**
	* @brief Quits the game, sets mIsRunnning to false
	*/
	void Quit() { mIsRunning = false; }

	/**
	* @brief Updates the game's objects, delta time is calculated inside this function.
	*/
	void Update();
	
	/**
	* @brief Renders the game's objects
	*/
	void Render();
	
	/**
	* @brief Performs a clean-up of resources
	*/
	void Clean();

	/**
	* @brief Process the game's input
	*/
	void ProcessInput();

	/**
	* @brief Starts the game's main loop
	*/
	void RunGame();
	
	/// Getters
	///--------------------------------------------------------

	/**
	 * @brief Checks if the game is currently running.
	 * @return bool Returns true if the game is running, false otherwise.
	*/
	inline bool GetIsRunning() { return mIsRunning; }

	/**
	* @brief Returns the game's renderer (SDL_Renderer)
	*/
	inline SDL_Renderer* GetRenderer() { return mRenderer; }

	/**
	* @brief Returns the game's window (SDL_Window)
	*/
	inline const SDL_Window* GetWindow()
	{
		return mWindow;
	}

	/**
	 * @brief Returns the game's delta time (float)
	 */
	float GetDeltaTime() { return mDeltaTime; }

	/**
	 * @brief Returns the game's asteroids map (unordered)
	 */
	std::unordered_map<int, Asteroid>& GetAsteroidsMap() { return mAsteroidsMap; }

	/// Utility
	///--------------------------------------------------------

	/**
	 * @brief Draws a wire frame (an outline shape) type of model with the given the coordinates.
	 * @param renderer The renderer that handles the draw call
	 * @param vecModelCoordinates The model's vertices to be drawn. 
	 * Each pair of floats in the vector represents the X and Y coordinates of a vertex in the model.
	 * @param x The x position where the model should be drawn on the screen
	 * @param y The y position where the model should be drawn on the screen
	 * @param r The Model's rotation (in RAD), has a default value of 0.
	 * @param s The Model's scale(or size), has a default value of 1.0f.
	*/
	void DrawWireFrameModel( SDL_Renderer* renderer, const std::vector<std::pair<float, float>>& vecModelCoordinates,
							 float x, float y, float r = 0.0f, float s = 1.0f );

	/**
	 * @brief Wraps coordinates to ensure continuous movement within game space.
	  * @param in_position The current position of the object in the game space.
	  * This is a 2D vector representing the X and Y coordinates.
	*/
	void WrapCoordinates( SpaceObject& obj );

	/**
	 * @brief Draws a filled circle
	 * @param renderer The renderer that handles the draw call
	 * @param centerX The circle's center x position
	 * @param centerY The circle's center y position
	 * @param radius The circle's radius
	 * @param color The circle's color
	*/
	void DrawCircleFill( SDL_Renderer* renderer, float centerX, float centerY, float radius, SDL_Color color );

	/**
	 * @brief Checks if a given point is inside a circle
	 * @param cx The circle's x position 
	 * @param cy The circle's y position 
	 * @param radius The circle's radius
	 * @param x The point's x position
	 * @param y The point's y position
	*/
	bool IsPointInCircle( float cx, float cy, float mRadius, float x, float y );

	/**
	 * @brief Adds a single asteroid to the game.
	 * Constructing new Asteroid instance with the space object obj parameter.
	 * @param obj The asteroid's space object
	*/
	void AddAsteroid( const SpaceObject& obj );
	
	/**
	 * @brief Adds random asteroids to the game.
	 * The asteroids position, velocity, rotation and size are being randomized
	*/
	void AddRandomAsteroids();

	/**
	 * @brief Resets the game to it's initial state.
	*/
	void RestartGame();

	/**
	 * @brief Adds 1 to the player's score.
	 */
	void AddScore( int score ) { mScoreCount += score; }

private:
	// Game's private constructor
	// Follows the singleton design pattern
	Game()
		: mWindow( nullptr )
		, mRenderer( nullptr )
		, mIsRunning( false )
		, mShip( nullptr )
		, mTimer( nullptr )
		, mDeltaTime( 0.0f )
		, mFPS( 0.0 )
		, mTicksCount( 0 )
		, mAsteroidsIndex( 0 )
		, mStartGameSound( nullptr )
		, mScoreCount( 0 )
		, mPlayerWon( false )
	{}

	// The Game class instance (singleton)
	static Game* s_Instance;
	// The Game's window
	SDL_Window* mWindow;
	// The Game's renderer
	SDL_Renderer* mRenderer;

	// boolean to hold the game running condition
	bool mIsRunning;
	// boolean to hold the winning condition
	bool mPlayerWon;

	// Map to hold the game's asteroids
	std::unordered_map<int, Asteroid> mAsteroidsMap;

	// The player's ship object
	Ship* mShip;

	// the Game's timer
	Timer* mTimer;

	// The Game's tick count
	uint64_t mTicksCount;
	// The Game's delta time
	float mDeltaTime;
	// Holds the score text to be rendered
	std::string mScoreStr;
	// Holds the game's fps text 
	std::string mFPSText;
	// The Game's fps
	long double mFPS;
	// Renders the fps text
	//std::unique_ptr<TextRenderer, TextRendererDeleter> mFpsText;
	// Renders the win text
	std::unique_ptr<TextRenderer, TextRendererDeleter> mWinText;
	// Renders the dead text
	std::unique_ptr<TextRenderer, TextRendererDeleter> mDeadText;
	// Renders the restart text
	std::unique_ptr<TextRenderer, TextRendererDeleter> mRestartText;
	// Renders the score text
	std::unique_ptr<TextRenderer, TextRendererDeleter> mScoreText;

	// Game's score count
	int mScoreCount;
	
	// Asteroids map index
	int mAsteroidsIndex;

	// Minimum asteroids count
	static const int MIN_ASTEROIDS_COUNT = 15;
	// Maximum asteroids count
	static const int MAX_ASTEROIDS_COUNT = 35;
	// Minimum asteroids size
	static const int MIN_SIZE = 24;
	// Minimum asteroids size
	static const int MAX_SIZE = 96;
	// Minimum asteroids x velocity
	static const float MIN_X_VELOCITY;
	// Maximum asteroids x velocity
	static const float MAX_X_VELOCITY;
	// Minimum asteroids y velocity
	static const float MIN_Y_VELOCITY;
	// Maximum asteroids y velocity
	static const float MAX_Y_VELOCITY;
	// Minimum asteroids rotation
	static const float MIN_ROT;
	// Maximum asteroids rotation
	static const float MAX_ROT;

	// Game Start Sound
	Mix_Chunk* mStartGameSound;
	// Game start sound channel
	int mStartGameChannel = 0;
};

