
#include "CliIRC.hpp"
#include "Game.hpp"

int			main()
{
	CliIRC	client("robot.ini");
	client.setEventPtr(NULL);
	Game::init(client);
	while (24400)
	{
		std::cout << "connecting..." << std::endl;
		client.connect();
		client.loop();
		std::cout << "disconnecting..." << std::endl;
		sleep(5);
	}		
	return (0);
}
