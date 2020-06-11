//
// Created by wangj on 2020/6/11.
//

#ifndef HSERVER_BASEEVENT_HPP
#define HSERVER_BASEEVENT_HPP

#include <functional>
#include <unistd.h>

enum ObjType {
	kUnknow = 0,
	kMainServer = 1,
	kCellServer = 2,
	kClient = 3
};

typedef ObjType obj_type_t;
typedef int fd_t;
typedef int event_t;
typedef bool status_t;
typedef std::function<void()> EventCallback;

const int kInvalidSocket = -1;
const int kDefaultEvent = 0;
const bool kDefaultStatus = false;

class BaseEvent {
public:
	BaseEvent() : obj_type_(ObjType::kUnknow), epoll_fd_(kInvalidSocket), sock_fd_(kInvalidSocket),
	              event_(kDefaultEvent), status_(kDefaultStatus),
	              callback_(nullptr) {

	}

	explicit BaseEvent(obj_type_t obj_type) : obj_type_(obj_type), epoll_fd_(kInvalidSocket), sock_fd_(kInvalidSocket),
	                                          event_(kDefaultEvent), status_(kDefaultStatus),
	                                          callback_(nullptr) {

	}

	BaseEvent(const BaseEvent &obj) = default;

	BaseEvent &operator=(const BaseEvent &obj) = default;

	virtual ~BaseEvent() = default;

	ObjType Type() const {
		return obj_type_;
	};

	bool IsAlive() {
		return sock_fd_ != kInvalidSocket;
	}

protected:
	fd_t epoll_fd_;
	fd_t sock_fd_;
	event_t event_;
	status_t status_;
	EventCallback callback_;
	obj_type_t obj_type_;
};

#endif //HSERVER_BASEEVENT_HPP
