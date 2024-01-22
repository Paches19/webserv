#include "../include/WebServer.hpp"

int main(int argc, char **argv) 
{
	if (argc == 1 || argc == 2) {
		try 
		{
			std::string		config;
			ConfigParser	cluster;
	
			// Configuration file as argument or default path
			config = (argc == 1 ? "config/default.conf" : argv[1]);
			cluster.initParser(config);
			cluster.print();
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return (1);
		}
    }
    else 
	{
		std::cout << "Error: wrong arguments" << std::endl;
		return (1);
	}
    return (0);
}
