//
// Created by wangj on 2020/6/11.
//

#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>

#include "CellClient.h"
#include "CellServer.h"
#include "DataProto.hpp"

CellClient::CellClient(int ep_fd, int fd, CellServer *cell_server)
		: BaseEvent(ObjType::kClient), last_time_(time(nullptr)), recv_buf_len_(0), send_buf_len_(0),
		  cell_server_(cell_server) {
	epoll_fd_ = ep_fd;
	sock_fd_ = fd;

	SetNonBlock();
	callback_ = [this] { RecvData(); };
}

CellClient::~CellClient() {
	if (sock_fd_ != kInvalidSocket) {
		close(sock_fd_);
		sock_fd_ = kInvalidSocket;
	}
}

int CellClient::SockFd() const {
	return sock_fd_;
}

int CellClient::Event() const {
	return event_;
}

time_t CellClient::LastTime() const {
	return last_time_;
}

void CellClient::SetNonBlock() const {
	int flags = Fcntl(sock_fd_, F_GETFL, 0);
	Fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK);
}

void CellClient::Reset(EventCallback callback) {
	event_ = kDefaultEvent;
	status_ = kDefaultStatus;
	last_time_ = time(nullptr);
	callback_ = std::move(callback);
}

void CellClient::AddToTree(int ep_fd, int event) {
	epoll_event epv{};
	epv.events = event_ = event;
	epv.data.ptr = this;

	if (status_ != 0) {
		printf("add error: already on tree\n");
		return;
	}

	if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_fd_, &epv) < 0) {
		printf("event add failed, ep_fd[%d], fd[%d], events[%d]\n", ep_fd, sock_fd_, event);
		return;
	}

	status_ = true;
//	printf("event add finifhed, ep_fd[%d], fd[%d], events[%d]\n", ep_fd, sock_fd_, event);
}

void CellClient::DelFromTree(int ep_fd) {
	if (status_ != 1)
		return;

	epoll_ctl(ep_fd, EPOLL_CTL_DEL, sock_fd_, nullptr);
	status_ = false;
}

void CellClient::OnNetMsg(DataHeader *header) {
	if (!header)
		return;

	++msg_count_;

	switch (header->cmd) {
		case kCmdLogin: {
			
//			auto temp = (LoginData *) header;
//			auto login_data = new LoginData;
//			memcpy(login_data, temp, temp->data_len);
//			AddTask([this, login_data]() -> void {
//				printf("user_name:%s, password:%s\n", login_data->user_name, login_data->password);
//				delete login_data;
//			});

//			auto login_data = (LoginData*)header;
//			printf("user_name:%s, password:%s\n", login_data->user_name, login_data->password);
//			LoginRetData login_ret_data = {};
//			login_ret_data.ret = 8520;
//			Send(&login_ret_data);
		}
			break;

		default:
			printf("unknow cmd\n");
			break;
	}
}

void CellClient::RecvData() {
	int ret;
	while (IsAlive()) {
		ret = Read(sock_fd_, recv_buf_ + recv_buf_len_, BUFSIZ - recv_buf_len_);
		if (ret < 0) {
			if (errno == EWOULDBLOCK) {
				DelFromTree(epoll_fd_);
				Reset([this] { SendData(); });
				AddToTree(epoll_fd_, EPOLLOUT);
				break;
			} else {
				perror("read error");
				DelFromTree(epoll_fd_);
				close(sock_fd_);
				printf("Client[%d] disconnection\n", sock_fd_);
				sock_fd_ = kInvalidSocket;
				break;
			}
		} else if (ret == 0) {
			DelFromTree(epoll_fd_);
			close(sock_fd_);
			printf("Client[%d] disconnection\n", sock_fd_);
			sock_fd_ = kInvalidSocket;
			break;
		} else {
			recv_buf_len_ += ret;

			while (recv_buf_len_ >= sizeof(DataHeader)) {
				// 转为DataHeader指针，获取当前消息长度
				auto header = (DataHeader *) recv_buf_;

				// 判断接收缓冲区的数据长度是否大于等于一条完整的消息
				if (recv_buf_len_ >= header->data_len) {
					// 未处理的消息的长度
					int len = recv_buf_len_ - header->data_len;

					OnNetMsg(header);

					// 将消息缓冲区的未处理的数据前移
					memcpy(recv_buf_, recv_buf_ + header->data_len, len);

					// 将消息缓冲区数据尾部位置前移
					recv_buf_len_ = len;

				} else {
					// 缓冲区的数据长度不足一条完整消息长度
					break;
				}
			}
		}
	}
}

void CellClient::SendData() {
	std::lock_guard<std::mutex> lock(send_buf_mutex_);

	int ret = 0;
	while (IsAlive() && (send_buf_len_ > 0)) {
		ret = Write(sock_fd_, send_buf_ + ret, send_buf_len_);
		if (ret < 0) {
			perror("write error");
			break;
		} else if (ret == 0) {
			DelFromTree(epoll_fd_);
			close(sock_fd_);
			printf("Client[%d] disconnection\n", sock_fd_);
			sock_fd_ = kInvalidSocket;
			break;
		} else {
			send_buf_len_ -= ret;

			// 全部发送完
			if (send_buf_len_ == 0) {
				DelFromTree(epoll_fd_);
				Reset([this] { RecvData(); });
				AddToTree(epoll_fd_, EPOLLIN);
				break;
			}
		}
	}

	DelFromTree(epoll_fd_);
	Reset([this] { RecvData(); });
	AddToTree(epoll_fd_, EPOLLIN);
}

void CellClient::OnEvent() {
	if (callback_)
		callback_();
}

void CellClient::AddTask(const task_t &task) {
	if (cell_server_)
		cell_server_->AddTask(task);
}

void CellClient::Send(DataHeader *header) {
	if (header) {
		std::lock_guard<std::mutex> lock(send_buf_mutex_);

		memcpy(send_buf_ + send_buf_len_, header, header->data_len);
		send_buf_len_ += header->data_len;
	}
}


size_t CellClient::MsgCount() {
	return size_t(msg_count_);
}

void CellClient::ResetMsgCount(size_t old_count) {
	msg_count_ -= old_count;
}
