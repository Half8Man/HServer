//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_CELLCLIENT_H
#define HSERVER_CELLCLIENT_H

#include <mutex>
#include <atomic>

#include "BaseEvent.hpp"
#include "warp.h"
#include "ThreadPool.hpp"

class CellServer;

struct DataHeader;

class CellClient : public BaseEvent {
public:
	CellClient() = delete;

	CellClient(int ep_fd, int fd, CellServer *cell_server);

	~CellClient() override;

	int SockFd() const;

	int Event() const;

	time_t LastTime() const;

	void SetNonBlock() const;

	void Reset(EventCallback callback);

	void AddToTree(int ep_fd, int event);

	void DelFromTree(int ep_fd);

	void OnNetMsg(DataHeader *header);

	void RecvData();

	void SendData();

	void OnEvent();

	void AddTask(const task_t &task);

	void Send(DataHeader *header);

	size_t MsgCount();

	void ResetMsgCount(size_t old_count);

private:
	time_t last_time_;
	int recv_buf_len_;
	int send_buf_len_;
	char recv_buf_[BUFSIZ]{};
	char send_buf_[BUFSIZ]{};
	CellServer *cell_server_;
	std::atomic_int msg_count_{};
	std::mutex send_buf_mutex_;
};


#endif //HSERVER_CELLCLIENT_H
