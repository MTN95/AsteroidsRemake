#include "Game.h"
#include "InputManager.hpp"
#include <iostream>

Game* Game::s_Instance = nullptr;

bool Game::Init(const char* title, bool fullscreen)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Failed to initialize SDL! SDL_Error: " << SDL_GetError() << '\n';
        return false;
    }

    int window_flags = 0;

    if (fullscreen) {
        window_flags = SDL_WINDOW_FULLSCREEN;
    }

    m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    if (m_Window == nullptr) {
        std::cout << "Failed to Create Window! SDL_Error: " << SDL_GetError() << '\n';
        return false;
    }

    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_Renderer == nullptr) {
        std::cout << "Failed to Create Renderer! SDL_Error: " << SDL_GetError() << '\n';
        return false;
    }

	int image_flags = IMG_INIT_PNG | IMG_INIT_JPG;
	if ( !( IMG_Init( image_flags ) & image_flags ) )
	{
		printf( "Failed to Initialize SDL_image: %s\n", IMG_GetError() );
		throw std::runtime_error( "(Game): Failed to Initialize SDL_image!\n" );
	}

    if (TTF_Init() != 0) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error : " << TTF_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_Mixer could not initialize! Mix_Error : " << Mix_GetError() << std::endl;
        return false;
    }

    RestartGame();

    m_IsRunning = true;
    return true;
}

void Game::Update()
{
    mDeltaTime = (mTimer->PeekMilliseconds() - mTicksCount) / 1000.0f;
    if (mDeltaTime > 0.05f) 
        mDeltaTime = 0.05f;
    
    mTicksCount = mTimer->PeekMilliseconds();

    mFPS = 1.0 / mDeltaTime;
    mFPSText = "FPS: " + std::to_string( mFPS );
    mFpsText->UpdateText(mFPSText);

    //  std::cout << "FPS: " << fps << '\n';
    //  std::cout << "DeltaTime: " << mDeltaTime << '\n';

	if ( !mIsDead )
	{
		mShip->Update(mDeltaTime);
    
		WrapCoordinates(mShip->GetSpaceObject());

		for (auto& a : mAsteroidsMap)
		{
			a.second.Update(mDeltaTime);
			//WrapCoordinates( a.GetSpaceObject());
		}
	}

}

void Game::Render()
{
	// Prepare scene
	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_Renderer);

	if ( !mIsDead )
	{
		mShip->Render(m_Renderer);
		for ( auto& a : mAsteroidsMap )
		{
			a.second.Render(m_Renderer);
		}
		mFpsText->RenderText( m_Renderer, { 10.f, mFpsText->GetTextSize().y } );			
	}
	else
	{
		
		mDeadText->RenderText( m_Renderer, { 300.f, 250.f} );
		mRestartText->RenderText( m_Renderer, { 150.f, 300.f } );

	}
    // Present scene
    SDL_RenderPresent(m_Renderer);
}


void Game::Clean()
{
    for ( auto& a : mAsteroidsMap )
    {
        a.second.Clean();
    }

    mShip->Clean();

    SDL_DestroyRenderer(m_Renderer);
    m_Renderer = nullptr;

    SDL_DestroyWindow(m_Window);
    m_Window = nullptr;

    TTF_Quit();
    IMG_Quit();

	Mix_CloseAudio();
	Mix_Quit();
    SDL_Quit();
}

void Game::ProcessInput()
{
    SDL_Event event;
	auto input = InputManager::get();
	input->ProcessInput( &event );

	if ( input->isKeyPressed( SDL_SCANCODE_ESCAPE ) )
	{
		Quit();
	}

	if ( mIsDead )
	{
		if ( input->isKeyPressed( SDL_SCANCODE_RETURN ) )
		{
			RestartGame();
		}
	}
    mShip->ProcessInput();
}

void Game::RunEngine()
{
    while (m_IsRunning) {
        ProcessInput();
        Update();
        Render();
		
        InputManager::get()->UpdatePrevInput();

    }
    Clean();
}

void Game::DrawLine( SDL_Renderer* renderer, const glm::vec2& point1, const glm::vec2& point2 )
{
	SDL_RenderDrawLineF( renderer, point1.x, point1.y, point2.x, point2.y );
}

void Game::DrawWireFrameModel( SDL_Renderer* renderer, const std::vector<std::pair<float, float>>& vecModelCoordinates, float x, float y, float r, float s )
{
	// Create translated model vector of coordinate pairs
	std::vector<std::pair<float, float>> vecTransformedCoordinates;
	auto verts = vecModelCoordinates.size();
	vecTransformedCoordinates.resize( verts );

	// Rotate
	for ( int i = 0; i < verts; i++ )
	{
		vecTransformedCoordinates[ i ].first = vecModelCoordinates[ i ].first * cosf( r ) - vecModelCoordinates[ i ].second * sinf( r );
		vecTransformedCoordinates[ i ].second = vecModelCoordinates[ i ].first * sinf( r ) + vecModelCoordinates[ i ].second * cosf( r );
	}

	// Scale
	for ( int i = 0; i < verts; i++ )
	{
		vecTransformedCoordinates[ i ].first *= s;
		vecTransformedCoordinates[ i ].second *= s;
	}

	// Translate
	for ( int i = 0; i < verts; i++ )
	{
		vecTransformedCoordinates[ i ].first += x;
		vecTransformedCoordinates[ i ].second += y;
	}

	// Draw Closed Polygon
	for ( int i = 0; i < verts; i++ )
	{
		int j = ( i + 1 ) % verts;

		// Use double for intermediate calculations
		auto ax = static_cast< double >( vecTransformedCoordinates[ i ].first );
		auto ay = static_cast< double >( vecTransformedCoordinates[ i ].second );
		auto bx = static_cast< double >( vecTransformedCoordinates[ j ].first );
		auto by = static_cast< double >( vecTransformedCoordinates[ j ].second );

		// Clamp values to the range of int
		auto clampToIntRange = []( double value )
		{
			constexpr double Max = std::numeric_limits<float>::max();
			constexpr double Min = std::numeric_limits<float>::min();
			return static_cast< int >( std::max( Min, std::min( Max, value ) ) );
		};

		glm::vec2 A( clampToIntRange( ax ), clampToIntRange( ay ) );
		glm::vec2 B( clampToIntRange( bx ), clampToIntRange( by ) );

		DrawLine( renderer, A, B );
	}

}

void Game::WrapCoordinates( SpaceObject& obj )
{   
    auto fSize = static_cast< float >( obj.mSize );

	if ( obj.mPosition.x > SCREEN_WIDTH + fSize )
	{
		obj.mPosition.x = -fSize;
	}
	else if ( obj.mPosition.x < -fSize )
	{
		obj.mPosition.x = SCREEN_WIDTH + fSize;
	}

	if ( obj.mPosition.y > SCREEN_HEIGHT + fSize )
	{
		obj.mPosition.y = -fSize;
	}
	else if ( obj.mPosition.y < -fSize )
	{
		obj.mPosition.y = SCREEN_HEIGHT + fSize;
	}
}

void Game::DrawCircleFill( SDL_Renderer* renderer, float centerX, float centerY, float mRadius, SDL_Color color )
{
	// Set the draw color
	SDL_SetRenderDrawColor( renderer, color.r, color.g, color.b, color.a );

	// Draw a filled circle
	for ( float w = 0; w < mRadius * 2; w++ )
	{
		for ( float h = 0; h < mRadius * 2; h++ )
		{
			float dx = mRadius - w; // horizontal offset
			float dy = mRadius - h; // vertical offset
			if ( ( dx * dx + dy * dy ) <= ( mRadius * mRadius ) )
			{
				SDL_RenderDrawPointF( renderer, centerX + dx, centerY + dy );
			}
		}
	}
}

bool Game::IsPointInCircle( float cx, float cy, float mRadius, float x, float y )
{
	return sqrt( ( x - cx ) * ( x - cx ) + ( y - cy ) * ( y - cy ) ) < mRadius;
}

void Game::AddAsteroid(const SpaceObject& obj)
{
	++mAsteroidsIndex;
	mAsteroidsMap.insert( {mAsteroidsIndex, Asteroid(obj, SDL_Color( 255, 255, 0, 255 )) });
}

void Game::RestartGame()
{
	mShip = new Ship( { 400.f, 500.f }, { 0, 255, 0, 255 } );

	mTimer = new Timer();
	mTimer->Start();

	mFpsText = std::unique_ptr<TextRenderer, TextRendererDeleter>( new TextRenderer( mFPSText.c_str(), 24, SDL_Color( 255, 0, 0, 255 ) ), TextRendererDeleter() );
	
	const char* deadText = "You Are DEAD!";
	mDeadText = std::unique_ptr<TextRenderer, TextRendererDeleter>( new TextRenderer( deadText, 26, SDL_Color( 255, 0, 0, 255 ) ), TextRendererDeleter() );
	mDeadText->CreateText();

	const char* restartText = "Press enter to Restart or escape to exit.";
	mRestartText= std::unique_ptr<TextRenderer, TextRendererDeleter>( new TextRenderer( restartText, 20, SDL_Color( 255, 0, 0, 255 ) ), TextRendererDeleter() );
	mRestartText->CreateText();

	
	mAsteroidsIndex = 0;

	AddAsteroid( SpaceObject( { 75.f,450.f }, { 8.0f, -6.0f }, 0.5f, 48 ) );
	AddAsteroid( SpaceObject( { 75.f,250.f }, { 8.0f, -6.0f }, 0.5f, 48 ) );
	AddAsteroid( SpaceObject( { 185.f,225.f }, { 8.0f, -6.0f }, 0.5f, 48 ) );
	AddAsteroid( SpaceObject( { 300.f,100.f }, { 8.0f, -6.0f }, 0.5f, 96 ) );
	AddAsteroid( SpaceObject( { 600.f,130.f }, { 8.0f, -6.0f }, 0.5f, 96 ) );
	AddAsteroid( SpaceObject( { 300.f,400.f }, { 8.0f, -6.0f }, 0.5f, 96 ) );
	AddAsteroid( SpaceObject( { 600.f,400.f }, { 8.0f, -6.0f }, 0.5f, 96 ) );

	std::string startGameSoundSrc = std::string( SOLUTION_DIR ) + "Assets/RestartGame.wav";
	mStartGameSound = Mix_LoadWAV( startGameSoundSrc.c_str() );
	if ( !mStartGameSound )
	{
		printf( "Failed to load the asteroid hit sound! , Error: %s", Mix_GetError() );
	}

	Mix_VolumeChunk( mStartGameSound, MIX_MAX_VOLUME / 3 );
	mStartGameChannel;

	Mix_PlayChannel(mStartGameChannel,mStartGameSound,0);
	mIsDead = false;
}

void Game::SetIsDead( bool isDead )
{
	mIsDead = isDead;
	mAsteroidsMap.clear();
}
