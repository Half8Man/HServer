//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_DATAPROTO_HPP
#define HSERVER_DATAPROTO_HPP

enum Cmd {
	kCmdHeartBeatC2s,
	kCmdHeartBeatS2c,
	kCmdLogin,
	kCmdLoginRet,
	kCmdLogout,
	kCmdLogoutRet,
	kCmdNewUserJoin,
	kCmdError
};

struct DataHeader {
	int data_len;
	int cmd;
};

// DataPackage
struct LoginData : public DataHeader {
	LoginData() : DataHeader() {
		data_len = sizeof(LoginData);
		cmd = Cmd::kCmdLogin;
	}

	char user_name[64] = {};
	char password[64] = {};
};

struct LoginRetData : public DataHeader {
	LoginRetData() : DataHeader() {
		data_len = sizeof(LoginRetData);
		cmd = Cmd::kCmdLoginRet;
		ret = 0;
	}

	int ret;
};

#endif //HSERVER_DATAPROTO_HPP
