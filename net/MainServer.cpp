//
// Created by wangj on 2020/6/11.
//

#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>

#include "MainServer.h"
#include "CellServer.h"
#include "CellClient.h"

MainServer::MainServer() : BaseEvent(ObjType::kMainServer), cell_thread_(nullptr), thread_pool_(nullptr) {
}

MainServer::~MainServer() {
	if (thread_pool_) {
		delete thread_pool_;
		thread_pool_ = nullptr;
	}

	DelFromTree();
	Close();
}

void MainServer::Start(size_t cell_server_count, size_t thread_pool_size) {
	InitSocket(svr_port);

	// 创建线程池，负责处理业务逻辑
	if (!thread_pool_)
		thread_pool_ = new ThreadPool(thread_pool_size);

	// 创建cell_server，负责接收数据、发送数据
	for (size_t i = 0; i < cell_server_count; ++i) {
		auto *cell_server = new CellServer(sock_fd_, i + 1);
		cell_server_vec_.emplace_back(cell_server);
		cell_server->Start();
	}

	// 创建cell_thread，负责建立连接
	if (!cell_thread_) {
		cell_thread_ = new CellThread;
		if (cell_thread_) {
			cell_thread_->Start(
					nullptr,
					[this](CellThread *cell_thread) {
						Run(cell_thread);
					},
					nullptr
			);
		}
	}
}

void MainServer::InitSocket(unsigned short port) {
	// 创建socket
	sock_fd_ = Socket(AF_INET, SOCK_STREAM, 0);

	// 设置端口复用
	int opt = 1;
	Setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in svr_addr = {};
	bzero(&svr_addr, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	svr_addr.sin_port = htons(port);
	Bind(sock_fd_, (sockaddr *) &svr_addr, sizeof(svr_addr));
	Listen(sock_fd_, max_event_count);

	Reset([this]() { AcceptConnect(); });
	AddToTree(EPOLLIN);
}

void MainServer::Run(CellThread *cell_thread) {
	epoll_fd_ = epoll_create(max_event_count);
	if (epoll_fd_ <= 0) {
		printf("epoll_create error\n");
		return;
	}

	while (cell_thread->IsRun()) {
		// 监听
		int count = epoll_wait(epoll_fd_, epoll_events_, max_event_count, 500);
		if (count < 0) {
			printf("epoll_wait error\n");
			break;
		}

		int index;
		for (index = 0; index < count; ++index) {
			auto temp_event = epoll_events_[index];

			MainServer *server_obj = nullptr;
			auto obj = (BaseEvent *) (temp_event.data.ptr);
			if (obj->Type() == ObjType::kMainServer)
				server_obj = (MainServer *) obj;

			if (server_obj) {
				if ((temp_event.events & EPOLLIN)) {
					callback_();
				}

				if ((temp_event.events & EPOLLOUT)) {
					callback_();
				}
			}
		}
	}
}

void MainServer::Close() {
	if (cell_thread_) {
		cell_thread_->Close();
		delete cell_thread_;
		cell_thread_ = nullptr;
	}

	if (IsAlive()) {
		for (const auto &cell_server : cell_server_vec_) {
			if (cell_server) {
				cell_server->Close();
				delete cell_server;
			}
		}
		cell_server_vec_.clear();

		if (sock_fd_ != kInvalidSocket) {
			close(sock_fd_);
			sock_fd_ = kInvalidSocket;
		}

		if (epoll_fd_ != kInvalidSocket) {
			close(epoll_fd_);
			epoll_fd_ = kInvalidSocket;
		}
	}
}

void MainServer::AcceptConnect() {
	sockaddr_in client_addr{};
	socklen_t client_addr_len;
	int client_fd = Accept(sock_fd_, (sockaddr *) &client_addr, &client_addr_len);

	auto new_client = new CellClient(epoll_fd_, client_fd, this);
	AddClient2CellSvr(new_client);

	char ip[BUFSIZ]{};
	printf("new client connect, addrs:[%s:%d], fd[%d], time[%ld]\n",
	       inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)), ntohs(client_addr.sin_port),
	       client_fd, new_client->LastTime()
	);
}

void MainServer::Reset(EventCallback callback) {
	event_ = kDefaultEvent;
	status_ = kDefaultStatus;
	callback_ = std::move(callback);
}

void MainServer::AddToTree(int event) {
	epoll_event epv{};
	epv.events = event_ = event;
	epv.data.ptr = this;

	if (status_ != 0) {
		printf("add error: already on tree\n");
		return;
	}

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock_fd_, &epv) < 0) {
		printf("event add failed, fd[%d], events[%d]\n", sock_fd_, event);
		return;
	}

	status_ = true;
}

void MainServer::DelFromTree() {
	if (!status_)
		return;

	epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, sock_fd_, nullptr);
	status_ = false;
}

void MainServer::AddClient2CellSvr(CellClient *cell_client) {
	if (cell_client) {
		// 查找客户端数量最小的CellServer
		auto min_server = cell_server_vec_[0];
		for (auto cell_server : cell_server_vec_) {
			if (cell_server->GetClientCount() < min_server->GetClientCount()) {
				min_server = cell_server;
			}
		}
		min_server->AddClient(cell_client);
	}
}

void MainServer::AddTask(const task_t &task) {
	if (thread_pool_) {
		auto ret = thread_pool_->enqueue(task);
		ret.get();
	}
}
