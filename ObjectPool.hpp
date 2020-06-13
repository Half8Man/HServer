//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_OBJECTPOOL_HPP
#define HSERVER_OBJECTPOOL_HPP

#include <mutex>
#include <cassert>

struct NodeHeader {
	NodeHeader *next_;
	int ref_count_;
	bool is_in_pool_;
};

template<typename T, size_t obj_count>
class ObjectPool {
public:
	ObjectPool() : header_(nullptr), buf_(nullptr) {
		InitPool();
	}

	~ObjectPool() {
		delete[] buf_;
	}

	void *AllocObj() {
		std::lock_guard<std::mutex> lock(mtx_);

		NodeHeader *node;
		if (!header_) {
			size_t real_size = sizeof(T) + sizeof(NodeHeader);
			node = (NodeHeader *) new char[real_size];
			node->next_ = nullptr;
			node->ref_count_ = 1;
			node->is_in_pool_ = false;
		} else {
			node = header_;
			header_ = header_->next_;
			assert(node->ref_count_ == 0);
			node->ref_count_ = 1;
		}

		return ((char *) node + sizeof(NodeHeader));
	}

	void FreeObj(void *obj) {
		auto *node = (NodeHeader *) ((char *) obj - sizeof(NodeHeader));
		assert(node->ref_count_ == 1);
		if (node->is_in_pool_) {
			std::lock_guard<std::mutex> lock(mtx_);
			if (--node->ref_count_ != 0) {
				return;
			}

			node->next_ = header_;
			header_ = node;
		} else {
			if (--node->ref_count_ != 0) {
				return;
			}
			delete[] node;
		}
	}

private:
	// 初始化对象池
	void InitPool() {
		assert(!buf_);
		if (buf_)
			return;

		// 计算对象池大小
		size_t real_size = sizeof(T) + sizeof(NodeHeader);
		size_t pool_size = real_size * obj_count;

		// 申请对象池内存
		buf_ = new char[pool_size];

		// 初始化内存
		header_ = (NodeHeader *) buf_;
		header_->next_ = nullptr;
		header_->ref_count_ = 0;
		header_->is_in_pool_ = true;

		// 遍历内存块，进行初始化
		NodeHeader *pre_node = header_;
		for (size_t i = 1; i < obj_count; ++i) {
			auto *cur_node = (NodeHeader *) (buf_ + (i * real_size));
			cur_node->next_ = nullptr;
			cur_node->ref_count_ = 0;
			cur_node->is_in_pool_ = true;

			pre_node->next_ = cur_node;
			pre_node = cur_node;
		}
	}

private:
	NodeHeader *header_;
	char *buf_;
	std::mutex mtx_;
};

#endif //HSERVER_OBJECTPOOL_HPP
