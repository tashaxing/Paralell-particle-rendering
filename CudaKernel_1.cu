#pragma once
// includes, C string library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
//-----------------------------------
//kernel function to update the vertex buffer
//-----------------------------------
__global__ void kernel(float4 *pos,unsigned int width,unsigned int height,float time)
{
	unsigned int x=blockIdx.x*blockDim.x+threadIdx.x;
    unsigned int y=blockIdx.y*blockDim.y+threadIdx.y;
	//calculate uv coordinates
	float u=x;
	float v=y;
	//calculate simple sine wave pattern
	float freq=0.06f;  //the smaller the less wave
	float w=sinf(u*freq+time)*cosf(v*freq+time)*2.0f;   
    //write output vertex
	u=u-(float)blockDim.x*gridDim.x/2;
	v=v-(float)blockDim.y*gridDim.y/2;
	pos[y*width+x]=make_float4(u/4,w,v/4,__int_as_float(0xff1e90ff));  //color code=lake blue ,****MY FAVOURITE NOKIA COLOR****
}

extern "C"
void D3DKernel(float4 *pos,unsigned int width,unsigned int height,float time )
{
	cudaDeviceProp prop;
	int num=0; //cudaDevice count
	cudaError_t cudaStatus=cudaGetDeviceCount(&num);
	for(int i=0;i<num;i++)
		cudaGetDeviceProperties(&prop,i);
	cudaSetDevice(0); //set the Tesla C1060
    dim3 block(8, 8, 1);
    dim3 grid((width+block.x-1)/ block.x, (height+block.y-1) / block.y, 1);

    kernel<<<grid, block>>>(pos, width, height, time);

}