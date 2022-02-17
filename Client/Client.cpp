// Client.cpp : Source file for your target.
//

#include "Peer.h"

int main(int argc, char *argv[]) {
	Peer c(321, "localhost", 46873, "localhost", 55555, "../../../../testFolder");
	c.start();
	c.console();

	return 0;
}
