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
#include "mandel.h"

// Inner loop of all the renderers. Accepts a point x0+iy0 on the complex plane
// and returns an eight-bit color index for the corresponding pixel. This is the
// critical section from a performance point of view, because the loop can
// execute up to 256 times, and this must be done for each and every pixel.
//
// The compiler generates two versions of this function, one on the host side
// and another functionally identical one on the GPU side.
//
__host__
__device__
unsigned char getPixColor(const float x0, const float y0) {
	float x = 0.0f, y = 0.0f, tmpx;
	int i = 0;
	while ( x*x + y*y < 4.0f && i < 256 ) {
		tmpx = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = tmpx;
		i++;
	}
	return (unsigned char)i;
}

// A naive CPU-based renderer, which just recalculates the point on the complex
// plane and the address in screen memory for each and every pixel.
//
void cpuNaiveRender(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size)
{
	const float delta = size/SCREEN_WIDTH;
	int px, py;
	float x0, y0;
	for ( py = 0; py < SCREEN_HEIGHT; py++ ) {
		for ( px = 0; px < SCREEN_WIDTH; px++ ) {
			x0 = xCentre + delta*(px - SCREEN_WIDTH/2);
			y0 = yCentre + delta*(py - SCREEN_HEIGHT/2);
			pixels[SCREEN_WIDTH*py + px] = getPixColor(x0, y0);
		}
	}
}

// A more efficient CPU-based renderer, which avoids recalculation by just
// adding the known delta each time. Unfortunately, because getPixColor() is
// where the CPU spends most of its time, in practice this optimization is not
// particularly great.
//
void cpuOptRender(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size)
{
	const float delta = size/SCREEN_WIDTH;
	int px, py;
	float x0, y0;
	y0 = yCentre - SCREEN_HEIGHT*delta/2;
	for ( py = 0; py < SCREEN_HEIGHT; py++ ) {
		x0 = xCentre - SCREEN_WIDTH*delta/2;
		for ( px = 0; px < SCREEN_WIDTH; px++ ) {
			*pixels++ = getPixColor(x0, y0);
			x0 += delta;
		}
		y0 += delta;
	}
}

// On the way to developing a GPU renderer, it's first necessary to think about
// how to partition the GPU resources into blocks of threads. Because it's
// easier to get this right on a CPU than a GPU, it makes sense to implement
// the partitioned algorithm on the CPU first.
//
// In this case, the screen is split up into tiles of 32 pixels by 16. Each tile
// can then be worked on by a GPU thread-block (a block can contain up to 512
// threads).
//
void cpuTiledRender(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size)
{
	const float delta = size/SCREEN_WIDTH;
	int tx, ty, px, py;
	int xPix, yPix;
	float x0, y0;
	for ( ty = 0; ty < VERT_TILES; ty++ ) {
		for ( tx = 0; tx < HORIZ_TILES; tx++ ) {
			for ( py = 0; py < TILE_HEIGHT; py++ ) {
				for ( px = 0; px < TILE_WIDTH; px++ ) {
					xPix = tx*TILE_WIDTH + px;
					yPix = ty*TILE_HEIGHT + py;
					x0 = xCentre + delta*(xPix - SCREEN_WIDTH/2);
					y0 = yCentre + delta*(yPix - SCREEN_HEIGHT/2);
					pixels[SCREEN_WIDTH*yPix + xPix] = getPixColor(x0, y0);
				}
			}
		}
	}
}

// A GPU renderer. Note the similarity to the code in the inner loop of the
// CPU tiled renderer.
//
__global__
void mandelKernel(
	const float xCentre,     // the real component of the region to render
	const float yCentre,     // the imaginary component of the region to render
	const float delta,       // the amount to step for each pixel
	unsigned char *gpuArray  // an array to store the results
) {
	const int xPix = blockDim.x * blockIdx.x + threadIdx.x;
	const int yPix = blockDim.y * blockIdx.y + threadIdx.y;
	const float x0 = xCentre + delta*(xPix - SCREEN_WIDTH/2);
	const float y0 = yCentre + delta*(yPix - SCREEN_HEIGHT/2);
	gpuArray[SCREEN_WIDTH*yPix + xPix] = getPixColor(x0, y0);
}

void gpuRender(
	unsigned char *pixels, const float xCentre, const float yCentre,
	const float size)
{
	const float delta = size/SCREEN_WIDTH;
	const dim3 blocksPerGrid(HORIZ_TILES, VERT_TILES);
	const dim3 threadsPerBlock(TILE_WIDTH, TILE_HEIGHT);
	mandelKernel<<<blocksPerGrid, threadsPerBlock>>>(xCentre, yCentre, delta, pixels);
}	

unsigned char *gpuAlloc(void) {
	unsigned char *gpuArray;
	cudaError_t err = cudaMalloc(&gpuArray, ARRAY_SIZE);
	if ( err != cudaSuccess ) return NULL;
	return gpuArray;
}

int gpuBlit(const unsigned char *sourceArray, unsigned char *destArray) {
	cudaError_t err =
		cudaMemcpy(destArray, sourceArray, ARRAY_SIZE, cudaMemcpyDeviceToHost);
	if ( err != cudaSuccess ) return 1;
	return 0;
}

void gpuFree(unsigned char *gpuArray) {
	cudaFree(gpuArray);
}
