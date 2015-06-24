// ThreadPool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "ThreadPool/ThreadPool.h"

void threadFunction(const char* param) {
	sleep(1);
	puts( param );
}

int main(int argc, char* argv[])
{
	ThreadPool<const char*> pool (threadFunction, 
		1, 
		4, //ThreadPool<const char*>::UnlimitedThreads, 
		1 //ThreadPool<const char*>::UnlimitedLifetime
		);
	pool.Launch("First Message");
	pool.Launch("Second Message");
	pool.Launch("Third Message");
	pool.Launch("Fourth Message");
	pool.Launch("Fifth Message");
	pool.Launch("Final Message");
	pool.Launch("Done");	

	std::string msg;
	std::cin >> msg;
	return 0;
}

