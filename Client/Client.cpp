// Client.cpp : Source file for your target.
//

#include "Peer.h"

int main(int argc, char *argv[]) {
	Peer c(153, "192.168.1.231", 48563, "192.168.1.231", 55555, "../../../../testFolder2");
	c.start();
	c.console();

	return 0;
}
