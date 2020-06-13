//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_CELLSERVER_H
#define HSERVER_CELLSERVER_H

#include <mutex>
#include <unordered_map>

#include <sys/epoll.h>

#include "BaseEvent.hpp"
#include "CellThread.hpp"
#include "ThreadPool.hpp"

const size_t max_event_count = 1024;
const unsigned short svr_port = 8888;

class MainServer;

class CellClient;

class CellServer : public BaseEvent {
public:
	CellServer() = delete;

	CellServer(int sock_fd, int server_id, MainServer *main_server);

	virtual ~CellServer();

	void Start();

	void Run(CellThread *cell_thread);

	void Close();

	int GetClientCount();

	void AddClient(int sock_fd);

	void DelClient(int sock_fd);

	void ClearClients();

	void AddTask(const task_t &task);

	size_t CalcMsgCount();

private:
	int server_id_; // server id
	MainServer *main_server_;
	CellThread *cell_thread_;
	epoll_event epoll_events_[max_event_count]{}; // 事件数组 // todo 宏定义
	std::unordered_map<int, CellClient *> client_map_{}; // 正式客户端map
	std::unordered_map<int, CellClient *> client_buf_map_{}; //客户端缓冲map
	std::mutex client_mutex_; // 客户端锁
};


#endif //HSERVER_CELLSERVER_H
