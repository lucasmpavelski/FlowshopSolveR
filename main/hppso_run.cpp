#include <iostream>
#include <sstream>
#include <fstream>

#include "pso-tunner/hppsoInit.hpp"
#include "pso-tunner/hppsoInputParam.hpp"

using namespace std;

int main(int argc, char **argv)
{
	// Get params
	hppsoInputParam parser(argc,argv);

	// Check if user needs help (--help)
    if (parser.userNeedsHelp())
    	return 1;

    // Start HPPSO
//	try
//	{
		cout << "Output file: " << parser.getOutputFile() << endl;

    hppsoInit swarm(
			parser.getPopSize(),
			parser.getMaxGen(),
			parser.getMaxRep(),
			parser.getOutputFile(),
			parser.getProbModel(),
			parser.getMinProb());

      swarm.runHppsoOne(
    		parser.getInstanceFile(),
    		parser.getSeed());

//	}
//	catch(exception& e)
//    {
//		cout << "Exception: " << e.what() << '\n';
//		return 1;
//    }

  	return 0;
}
