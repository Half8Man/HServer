//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_MAINSERVER_H
#define HSERVER_MAINSERVER_H

#include <vector>

#include <sys/epoll.h>

#include "BaseEvent.hpp"

#include "../thread/CellThread.hpp"
#include "../thread/ThreadPool.hpp"

class CellServer;

class CellClient;

class MainServer : public BaseEvent {
public:
	MainServer();

	~MainServer() override;

	void Start(size_t cell_server_count, size_t thread_pool_size);

	void InitSocket(unsigned short port);

	void Run(CellThread *cell_thread);

	void Close();

	void AcceptConnect();

	void Reset(EventCallback callback);

	void AddToTree(int event);

	void DelFromTree();

	void AddClient2CellSvr(CellClient *cell_client);

	void AddTask(const task_t &task);

protected:
	CellThread *cell_thread_;
	ThreadPool *thread_pool_;
	std::vector<CellServer *> cell_server_vec_{};
	epoll_event epoll_events_[1024]{}; // todo 宏定义
};


#endif //HSERVER_MAINSERVER_H
