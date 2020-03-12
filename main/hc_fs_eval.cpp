#include <cmath>
#include <iostream>

#include "hppsoInit.hpp"

int main(int argc, char **argv)
{
  	using namespace std;

    try
    {
    	hppsoInit* swarm = new hppsoInit(30,5,2,"resultNormal.txt",false);
    	swarm->runHppsoAll(1,1);
    	delete swarm;

        swarm = new hppsoInit(30,5,2,"resultProb.txt",true);
        swarm->runHppsoAll(1,1);
        delete swarm;
    }
    catch(exception& e)
    {
		cout << "Exception: " << e.what() << '\n';
    }

    return 1;
}