//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_CELLTHREAD_HPP
#define HSERVER_CELLTHREAD_HPP

#include <thread>
#include <functional>

#include "CellSemaphore.hpp"

class CellThread {
private:
	typedef std::function<void(CellThread *)> EventCall;

public:
	void Start(const EventCall &on_create = nullptr,
	           const EventCall &on_run = nullptr,
	           const EventCall &on_destory = nullptr) {
		std::lock_guard<std::mutex> lock(mutex_);

		if (!is_run_) {
			if (on_create)
				on_create_ = on_create;

			if (on_run)
				on_run_ = on_run;

			if (on_destory_)
				on_destory_ = on_destory;

			is_run_ = true;
			std::thread work_thread(std::mem_fn(&CellThread::Run), this);
			work_thread.detach();
		}
	}

	void Close() {
		std::lock_guard<std::mutex> lock(mutex_);

		if (is_run_) {
			is_run_ = false;
			sem_.Wait();
		}
	}

	void Exit() {
		std::lock_guard<std::mutex> lock(mutex_);

		if (is_run_)
			is_run_ = false;
	}

	bool IsRun() const {
		return is_run_;
	}

protected:
	void Run() {
		if (on_create_)
			on_create_(this);

		if (on_run_)
			on_run_(this);

		if (on_destory_)
			on_destory_(this);

		sem_.WakeUp();
	}

private:
	EventCall on_create_;
	EventCall on_run_;
	EventCall on_destory_;

	CellSemaphore sem_;
	std::mutex mutex_;
	bool is_run_ = false;
};

#endif //HSERVER_CELLTHREAD_HPP
