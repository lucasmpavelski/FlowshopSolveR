#include <iostream>
#include <sstream>
#include <fstream>
#include <random>

#include "hppsoInputParam.hpp"

int main(int argc, char **argv)
{  	
  	std::random_device true_rng;
	std::uniform_int_distribution<unsigned int> uniform;

	std::string command;

	// Get params
	hppsoInputParam parser(argc,argv);
	

	// Check if user needs help (--help)
    if (parser.userNeedsHelp())
    	return 1;
	
	try
	{
		for(int i = 0; i < parser.getSeed(); i++) {
			//std::cout << parser.getHppsoRun(uniform(true_rng)) << std::endl;
			std::cout << parser.getHppsoRunAllTest(uniform(true_rng),true) << std::endl;
		}
		
	}
	catch(std::exception& e)
    {
		std::cout << "Exception: " << e.what() << '\n';
		return 1;
    }

  	return 0;
}