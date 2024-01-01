#include <iostream>

#include "Common.h"
#include "Game.h"

int main(int argc, char * argv[])
{
	Game game("src/Config.txt");
	game.run();

	return 0;
}