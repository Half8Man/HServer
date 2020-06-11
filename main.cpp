//
// Created by wangj on 2020/6/11.
//

#include "net/MainServer.h"

int main() {
	MainServer main_server;
	main_server.Start(4, 16);
	return 0;
}
