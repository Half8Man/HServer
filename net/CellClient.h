//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_CELLCLIENT_H
#define HSERVER_CELLCLIENT_H

#include "BaseEvent.hpp"

#include "../utils/warp.h"
#include "../thread/ThreadPool.hpp"

class MainServer;

struct DataHeader;

class CellClient : public BaseEvent {
public:
	CellClient() = delete;

	CellClient(int ep_fd, int fd, MainServer *main_server);

	~CellClient() override;

	inline int SockFd() const;

	inline int Event() const;

	inline time_t LastTime() const;

	inline void SetNonBlock() const;

	void Reset(EventCallback callback);

	void AddToTree(int ep_fd, int event);

	void DelFromTree(int ep_fd);

	static void OnNetMsg(DataHeader *header);

	void RecvData();

	void SendData();

	void OnEvent();

	void AddTask(const task_t &task);

private:
	time_t last_time_;
	int recv_buf_len_;
	int send_buf_len_;
	char recv_buf_[BUFSIZ]{};
	char send_buf_[BUFSIZ]{};
	MainServer *main_server_;
};


#endif //HSERVER_CELLCLIENT_H
