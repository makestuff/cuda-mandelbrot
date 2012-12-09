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
#ifndef MANDEL_H
#define MANDEL_H

#ifdef __cplusplus
extern "C" {
#endif

	// Constants
	//#define SCREEN_WIDTH  2560
	//#define SCREEN_HEIGHT 1600
	#define SCREEN_WIDTH  1280
	#define SCREEN_HEIGHT 720
	//#define SCREEN_WIDTH  800
	//#define SCREEN_HEIGHT 600
	#define TILE_WIDTH    32
	#define TILE_HEIGHT   16
	#define HORIZ_TILES   (SCREEN_WIDTH / TILE_WIDTH)
	#define VERT_TILES    (SCREEN_HEIGHT / TILE_HEIGHT)
	#define ARRAY_SIZE    (SCREEN_WIDTH * SCREEN_HEIGHT)

	// Different renderers
	void cpuNaiveRender(
		unsigned char *pixels, const float xCentre, const float yCentre,
		const float size);
	void cpuOptRender(
		unsigned char *pixels, const float xCentre, const float yCentre,
		const float size);
	void cpuTiledRender(
		unsigned char *pixels, const float xCentre, const float yCentre,
		const float size);
	void gpuRender(
		unsigned char *pixels, const float xCentre, const float yCentre,
		const float size);

	// GPU alloc & copy
	unsigned char *gpuAlloc(void);
	int gpuBlit(const unsigned char *sourceArray, unsigned char *destArray);
	void gpuFree(unsigned char *gpuArray);

#ifdef __cplusplus
}
#endif

#endif
