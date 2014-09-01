#include <iostream>
#include <stdio.h>
#include "DBMgr.h"
#include <string>

int main(int argc, char **argv)
{
	auto db = DBMgr::getInstance();
	db->addFile(std::string(argv[1]));
	db->loadAll();

	return 0;
}
