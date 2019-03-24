#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define MOUSE_SENSITIVITY  0.1f
#define FOV           M_PI/2.5f
//#define MAX_FPS       30

int main(int argc, char** argv){
	volatile int gameRunning = 1;
	SDL_Event event;

	SDL_Window*   window;
	SDL_Renderer* renderer;
	SDL_Texture*  frameBuffer;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		return 3;
	};

	window = SDL_CreateWindow("Software Rendering", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 	// x, y
			WINDOW_WIDTH, WINDOW_HEIGHT, 						// Width, Height
			0);													// Flags

	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_PRESENTVSYNC);
	
	SDL_SetRelativeMouseMode(SDL_TRUE);

	frameBuffer = SDL_CreateTexture(renderer, 
			SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
			WINDOW_WIDTH, WINDOW_HEIGHT);

	Uint32 pixels[WINDOW_WIDTH * WINDOW_HEIGHT];

	/* Create all game variables */

	int mapWidth = 16,
		mapHeight = 16;
	char* gameMap;
	gameMap = "################"
		      "#..............#"
		      "#.##........##.#"
		      "#.#..........#.#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#..............#"
		      "#.#..........#.#"
		      "#.##........##.#"
		      "#..............#"
		      "################";
	float renderDistance = 16.0f;

	float playerAngle = 0.0f; // player angle
	float playerX = 10.0f,
		  playerY = 10.0f;
	float playerSpeed = 3.0f;

	/* Counting FPS */
	int fps_ = 0;
	int fps = 0;
	float elapsedTime = 0;
	int currentTime = 0;
	int lastOutput  = 0;
	while (gameRunning) {
		/* == Main Game Loop ==
		 *
		 * Pipeline is in the order of:
		 *
		 * Input:
		 *   Events,
		 *   Keyboard
		 *
		 * Rendering:
		 *   Drawing Pixels,
		 *   Rendering
		 *
		 * Counting:
		 *   FPS
		 */

		/* = Input = 
		 *
		 * Here all inputs for the game are 
		 * read and processed
		 */

		/* Events */
		float deltaX = 0.0f;
		float deltaY = 0.0f;
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					gameRunning = 0;
					break;
				case SDL_MOUSEMOTION:
					deltaX += event.motion.xrel;
					deltaY += event.motion.yrel;
					playerAngle += deltaX * elapsedTime * MOUSE_SENSITIVITY;
					//printf("%f\n", deltaX);
					break;
			}
		}
		
		/* Keyboard */
		const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

		if (keyboardState[SDL_SCANCODE_ESCAPE]) gameRunning = 0;

		/* Movement */

		float playerSinTime = playerSpeed * sin(playerAngle) * elapsedTime;
		float playerCosTime = playerSpeed * cos(playerAngle) * elapsedTime;

		if (keyboardState[SDL_SCANCODE_W]){ 
			playerX += playerSinTime;
			playerY += playerCosTime;
			if (gameMap[(int)playerY * mapWidth + (int)playerX] == '#'){
				playerX -= playerSinTime;
				playerY -= playerCosTime;
			}

		}
		if (keyboardState[SDL_SCANCODE_S]){ 
			playerX -= playerSinTime;
			playerY -= playerCosTime;
			if (gameMap[(int)playerY * mapWidth + (int)playerX] == '#'){
				playerX += playerSinTime;
				playerY += playerCosTime;
			}
		}
		if (keyboardState[SDL_SCANCODE_A]){ 
			playerX -= playerCosTime;
			playerY += playerSinTime;
			if (gameMap[(int)playerY * mapWidth + (int)playerX] == '#'){
				playerX += playerCosTime;
				playerY -= playerSinTime;
			}
		}
		if (keyboardState[SDL_SCANCODE_D]){ 
			playerX += playerCosTime;
			playerY -= playerSinTime;
			if (gameMap[(int)playerY * mapWidth + (int)playerX] == '#'){
				playerX -= playerCosTime;
				playerY += playerSinTime;
			}
		}

		/* = Rendering =
		 * 
		 * Here we draw, then render all of
		 * the games pixels.
		 */



		/* Draw Pixels */
		for (int x=0; x<WINDOW_WIDTH; x++) {
			float rayAngle = (playerAngle - FOV / 2.0f) + ((float)x / (float)WINDOW_WIDTH) * FOV;

			float distanceToWall = 0.0f;
			int hitWall = 0;

			float eyeX = sin(rayAngle);
			float eyeY = cos(rayAngle);

			while (!hitWall && distanceToWall < renderDistance) {

				distanceToWall += 0.01f;

				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight){
					hitWall = 1;
					distanceToWall = renderDistance;
				}
				else {
					if(gameMap[testY * mapWidth + testX] == '#') {
						hitWall = 1;
					}
				}
			}

			// Calculate distance to ceiling / floor
			int ceiling = (float)(WINDOW_HEIGHT / 2.0f) - WINDOW_HEIGHT / ((float)distanceToWall);
			int floor = WINDOW_HEIGHT - ceiling;

			Uint32 shade = 0xFF000000;
			shade += 0x010101 * (int)(0xFF / (distanceToWall + 1.7f));

			for (int y=0; y < WINDOW_HEIGHT; y++) {
				if (y <= ceiling){
					pixels[x + y * WINDOW_WIDTH] = 0xFF000000;
				}
				else if (y > ceiling && y <= floor) {
					pixels[x + y * WINDOW_WIDTH] = shade;
				}
				else {
					float b = (((float)y - WINDOW_HEIGHT / 2.0f) / 
							((float)WINDOW_HEIGHT / 2.0f));
					pixels[x + y * WINDOW_WIDTH] = 0xFF000000 + 0x010101 * (int)(0x60 * b);
					
				}
			}
		}

		/* Rendering */
		SDL_UpdateTexture(frameBuffer, NULL, pixels, sizeof(Uint32)*WINDOW_WIDTH);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, frameBuffer, NULL, NULL);
		SDL_RenderPresent(renderer);

		/* FPS Managment */
		elapsedTime = (float)(SDL_GetTicks() - currentTime) / 1000.0f;
		currentTime = SDL_GetTicks();
		fps_++;

		if (currentTime - lastOutput >= 1000)
		{
		    lastOutput = currentTime;
		    fps = fps_; // the variable 'fps' is displayed 
		    fps_ = 0;

			char windowTitle[32] = "Software Rendering | FPS: ";
			char buffer[20];
			SDL_itoa(fps, buffer, 10);
			strcat(windowTitle, buffer);
			SDL_SetWindowTitle(window, windowTitle);
		}
	}

	SDL_Quit();
}
