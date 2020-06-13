//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_CELLSEMAPHORE_HPP
#define HSERVER_CELLSEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

class CellSemaphore {
public:
	void Wait() {
		std::unique_lock<std::mutex> lock(mutex_);

		if (--wait_count_ < 0) {
			// 阻塞等待
			cv_.wait(
					lock,
					[this]() -> bool {
						return wakeup_count_ > 0;
					});
			wakeup_count_--;
		}
	}

	void WakeUp() {
		std::lock_guard<std::mutex> lock(mutex_);

		if (++wait_count_ <= 0) {
			wakeup_count_++;
			cv_.notify_one();
		}
	}

private:
	std::mutex mutex_;
	std::condition_variable cv_;
	int wait_count_ = 0;
	int wakeup_count_ = 0;
};

#endif //HSERVER_CELLSEMAPHORE_HPP
