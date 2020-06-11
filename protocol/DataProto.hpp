//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_DATAPROTO_HPP
#define HSERVER_DATAPROTO_HPP

enum Cmd {
	kCmdLogin = 1,
	kCmdLoginRet = 2
};

struct DataHeader {
	int data_len; // 消息长度
	int session; // 客户端消息session
	int cmd; // 消息号
};

struct LoginData : public DataHeader {
	LoginData() : DataHeader() {
		data_len = sizeof(LoginData);
		session = 0;
		cmd = Cmd::kCmdLogin;
	}

	char user_name[64]{};
	char password[64]{};
};

struct LoginRetData : public DataHeader {
	LoginRetData() : DataHeader() {
		data_len = sizeof(LoginRetData);
		session = 0;
		cmd = Cmd::kCmdLoginRet;
		ret = 0;
	}

	int ret;
};

#endif //HSERVER_DATAPROTO_HPP
