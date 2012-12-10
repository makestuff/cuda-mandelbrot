/* 
 * Copyright (C) 2012 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <sys/time.h>
#include <SDL/SDL.h>
#include "palette.h"
#include "mandel.h"

SDL_Surface *createSurface(Uint32 flags, const SDL_Surface *display) {
	const SDL_PixelFormat *fmt = display->format;
	return SDL_CreateRGBSurface(
		flags,
		display->w,
		display->h,
		fmt->BitsPerPixel,
		fmt->Rmask,
		fmt->Gmask,
		fmt->Bmask,
		fmt->Amask
	);
}

typedef void (*RenderFunc)(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size);

void doRender(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size, RenderFunc impl)
{
	//struct timeval tvStart, tvEnd;
	//long long startTime, endTime;
	//double totalTime;
	//double fps[10], total = 0;
	//const int maxIter = 10;
	//int i;
	//for ( i = 0; i < maxIter; i++ ) {
	//	gettimeofday(&tvStart, NULL);
		(*impl)(pixels, xCentre, yCentre, size);
	//	gettimeofday(&tvEnd, NULL);
	//	startTime = tvStart.tv_sec;
	//	startTime *= 1000000;
	//	startTime += tvStart.tv_usec;
	//	endTime = tvEnd.tv_sec;
	//	endTime *= 1000000;
	//	endTime += tvEnd.tv_usec;
	//	totalTime = endTime - startTime;
	//	totalTime /= 1000000; // Convert to seconds
	//	fps[i] = 1/totalTime;
	//	printf("  FPS[%d] = %f\n", i, fps[i]);
	//	total += fps[i];
	//}
	//printf("  Average FPS = %f\n", total/maxIter);
}

#define CHECK(condition, message, code)         \
	if ( condition ) {                           \
		fprintf(stderr, message, SDL_GetError()); \
		returnCode = code;                        \
		goto cleanup;                             \
	}

void waitClick(void) {
	SDL_Event event;
	for ( ; ; ) {
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
			case SDL_KEYDOWN:
				printf(
					"The %s key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym));
			case SDL_QUIT:
				return;
			}
		}
	}
}

void benchmarkRender(SDL_Surface *hello, SDL_Surface *screen) {
	unsigned char *gpuArray, *pixels = (unsigned char *)hello->pixels;
	float size = 10.0f, fpsMin = 1000.0f;
	int i;
	struct timeval tvStart, tvEnd;
	long long startTime, endTime;
	double totalTime;
	const int numFrames = 5000;
	double fps[numFrames], total = 0;
	//char fileName[17];
	printf("gpuRender:\n");
	gpuArray = gpuAlloc();
	for ( i = 0; i < numFrames; i++) {
		gettimeofday(&tvStart, NULL);
		gpuRender(gpuArray, -0.727003f, 0.19996f, size);
		gpuBlit(gpuArray, pixels);
		SDL_BlitSurface(hello, NULL, screen, NULL);
		gettimeofday(&tvEnd, NULL);
		//snprintf(fileName, 17, "frames/f%04d.bmp", i);
		//SDL_SaveBMP(hello, fileName);
		startTime = tvStart.tv_sec;
		startTime *= 1000000;
		startTime += tvStart.tv_usec;
		endTime = tvEnd.tv_sec;
		endTime *= 1000000;
		endTime += tvEnd.tv_usec;
		totalTime = endTime - startTime;
		totalTime /= 1000000; // Convert to seconds
		fps[i] = 1/totalTime;
		SDL_Flip(screen);
		size *= 0.9975;
	}
	gpuFree(gpuArray);
	for ( i = 0; i < numFrames; i++ ) {
		if ( fpsMin > fps[i] ) {
			fpsMin = fps[i];
		}
		printf("fps[%04d] = %f\n", i, fps[i]);
	}
	printf("min fps = %f\n", fpsMin);
	printf("final frame size = %f\n", size);
}

//int main(int argc, const char *args[]) {
int main(void) {
	int returnCode = 0;
	SDL_Surface *hello = NULL;
	SDL_Surface *screen = NULL;
	SDL_Color colors[256];
	int i;
	const unsigned char *p = palette;

	// Start SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	// Set up screen
	screen = SDL_SetVideoMode(
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		8,
		SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF // | SDL_ASYNCBLIT
		// | SDL_FULLSCREEN
	);
	CHECK(!screen, "Couldn't set video mode: %s\n", 1);

	// Build the palette
	for ( i = 0; i < 256; i++ ) {
		colors[i].r = *p++;
		colors[i].g = *p++;
		colors[i].b = *p++;
	}

	// Create surface to work with, and set the palette
	hello = createSurface(SDL_SWSURFACE, screen);
	SDL_SetPalette(hello, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);

	//printf("cpuNaiveRender:\n");
	//doRender(pixels, -0.5f, 0.0f, 3.5f, cpuNaiveRender);

	//printf("cpuOptRender:\n");
	//doRender(pixels, -0.5f, 0.0f, 3.5f, cpuOptRender);

	//printf("cpuTiledRender:\n");
	//doRender(pixels, -0.5f, 0.0f, 3.5f, cpuTiledRender);

	benchmarkRender(hello, screen);

	// Pause
	//waitClick();
	//SDL_Delay(10000);

cleanup:
	// Free the loaded image
	SDL_FreeSurface(hello);

	// Quit SDL
	SDL_Quit();

	return returnCode;
}
