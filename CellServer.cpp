//
// Created by wangj on 2020/6/11.
//

#include "CellServer.h"
#include "MainServer.h"
#include "CellClient.h"

CellServer::CellServer(int sock_fd, int server_id, MainServer *main_server)
		: BaseEvent(ObjType::kCellServer),
		  server_id_(server_id),
		  main_server_(main_server),
		  cell_thread_(nullptr) {
	sock_fd_ = sock_fd;
}

CellServer::~CellServer() {
	ClearClients();
	Close();
}

void CellServer::Start() {
	printf("收发服务%d开始启动...\n", server_id_);

	if (!cell_thread_) {
		cell_thread_ = new CellThread();
		if (cell_thread_) {
			cell_thread_->Start(
					nullptr,
					[this](CellThread *cell_thread) {
						Run(cell_thread);
					},
					[this](CellThread *cell_thread) {
						ClearClients();
					});
		}
	}

	printf("收发服务%d启动完成...\n", server_id_);
}

void CellServer::Run(CellThread *cell_thread) {
	epoll_fd_ = epoll_create(max_event_count);
	if (epoll_fd_ <= 0) {
		printf("epoll_create error\n");
		return;
	}

	while (cell_thread->IsRun()) {
		// 将缓冲区客户端移入正式客户端map
		if (!client_buf_map_.empty()) {
			std::lock_guard<std::mutex> lock(client_mutex_);
			for (const auto &iter : client_buf_map_) {
				auto sock_fd = iter.first;
				auto cell_client = iter.second;
				client_map_[sock_fd] = cell_client;
				cell_client->AddToTree(epoll_fd_, EPOLLIN);
			}
			client_buf_map_.clear();
		}

		// 如果没有客户端要处理，休眠一毫秒，跳过
		if (client_map_.empty()) {
			//TODO 休眠一毫秒
			continue;
		}

		// 监听
		int count = epoll_wait(epoll_fd_, epoll_events_, max_event_count, 0);
		if (count < 0) {
			printf("epoll_wait error\n");
			break;
		}

		int index;
		for (index = 0; index < count; ++index) {
			auto temp_event = epoll_events_[index];

			CellClient *client_obj = nullptr;

			auto obj = (BaseEvent *) (temp_event.data.ptr);
			if (obj->Type() == ObjType::kClient)
				client_obj = (CellClient *) obj;

			if (client_obj) {
				int client_fd = client_obj->SockFd();

				if ((temp_event.events & EPOLLIN) && (client_obj->Event() & EPOLLIN)) {
					client_obj->OnEvent();
				}

				if ((temp_event.events & EPOLLOUT) && (client_obj->Event() & EPOLLOUT)) {
					client_obj->OnEvent();
				}

				if (client_obj->SockFd() == kInvalidSocket)
					DelClient(client_fd);
			}
		}
	}
}

void CellServer::Close() {
	if (epoll_fd_ != kInvalidSocket)
		close(epoll_fd_);

	if (cell_thread_) {
		cell_thread_->Close();
		delete cell_thread_;
		cell_thread_ = nullptr;
	}
}

int CellServer::GetClientCount() {
	std::lock_guard<std::mutex> lock(client_mutex_);
	return client_map_.size() + client_buf_map_.size();
}

void CellServer::AddClient(int sock_fd) {
	auto new_client = new CellClient(epoll_fd_, sock_fd, this);

	std::lock_guard<std::mutex> lock(client_mutex_);
	client_buf_map_[sock_fd] = new_client;
}

void CellServer::DelClient(int sock_fd) {
	std::lock_guard<std::mutex> lock(client_mutex_);

	auto iter = client_map_.find(sock_fd);
	if (iter != client_map_.end()) {
		client_map_.erase(iter);
		iter->second->DelFromTree(epoll_fd_);
		return;
	}
}

void CellServer::ClearClients() {
	std::lock_guard<std::mutex> lock(client_mutex_);

	for (const auto &iter : client_buf_map_) {
		auto cell_client = iter.second;
		if (cell_client) {
			delete cell_client;
		}
	}
	client_buf_map_.clear();

	for (const auto &iter : client_map_) {
		auto cell_client = iter.second;
		if (cell_client) {
			cell_client->DelFromTree(epoll_fd_);
			delete cell_client;
		}
	}
	client_map_.clear();
}

void CellServer::AddTask(const task_t &task) {
	if (main_server_)
		main_server_->AddTask(task);
}

size_t CellServer::CalcMsgCount() {
	size_t total = 0;
	if (!client_map_.empty()) {
		std::lock_guard<std::mutex> lock(client_mutex_);
		for (const auto &iter : client_map_) {
			auto cell_client = iter.second;
			auto count = cell_client->MsgCount();
			total += count;
			cell_client->ResetMsgCount(count);
		}
	}

	return total;
}
