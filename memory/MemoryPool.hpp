//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_MEMORYPOOL_HPP
#define HSERVER_MEMORYPOOL_HPP

#include <cassert>

class MemoryPool;

struct MemoryBlock {
	MemoryPool *pool_; // 所属内存池
	MemoryBlock *next_; // 下一块block
	int ref_count_; // 引用计数
	bool is_in_pool_; // 是否在池内
};

class MemoryPool {
public:
	MemoryPool() : buf_(nullptr), header_(nullptr), size_(0), block_count_(0) {

	}

	virtual ~MemoryPool() {
		if (buf_)
			free(buf_);
	}

	// 初始化
	void InitMemory() {
		assert(!buf_);
		if (buf_)
			return;

		// 计算内存池大小
		size_t real_size = size_ + sizeof(MemoryBlock);
		size_t buf_size = real_size * block_count_;

		// 向系统申请内存
		buf_ = (char *) malloc(buf_size);

		// 初始化内存池
		header_ = (MemoryBlock *) buf_;
		header_->pool_ = this;
		header_->next_ = nullptr;
		header_->ref_count_ = 0;
		header_->is_in_pool_ = true;

		// 遍历内存块，进行初始化
		auto pre_block = header_;
		for (size_t i = 1; i < block_count_; ++i) {
			auto *cur_block = (MemoryBlock *) (buf_ + (i * real_size));
			cur_block->pool_ = this;
			cur_block->next_ = nullptr;
			cur_block->ref_count_ = 0;
			cur_block->is_in_pool_ = true;

			pre_block->next_ = cur_block;
			pre_block = cur_block;
		}
	}

	// 申请内存
	void *AllocMemory(size_t size) {
		if (!buf_)
			InitMemory();

		MemoryBlock *block;
		if (!header_) {
			// 初始化内存池失败或者内存池满了
			block = (MemoryBlock *) malloc(size + sizeof(MemoryBlock));
			block->pool_ = nullptr;
			block->next_ = nullptr;
			block->ref_count_ = 1;
			block->is_in_pool_ = false;
		} else {
			block = header_;
			header_ = header_->next_;
			assert(block->ref_count_ == 0);
			block->ref_count_ = 1;
		}

		return ((char *) block + sizeof(MemoryBlock));
	}

	// 释放内存
	void FreeMemory(void *ptr) {
		auto *block = (MemoryBlock *) ((char *) ptr - sizeof(MemoryBlock));

		if (--block->ref_count_ != 0)
			return;

		if (block->is_in_pool_) {
			block->next_ = header_;
			header_ = block;
		} else {
			free(block);
		}
	}

protected:
	char *buf_; // 内存池地址
	MemoryBlock *header_; // 内存头部单元
	size_t size_; // 内存单元大小
	size_t block_count_; // 内存单元数量
};

template<size_t size, size_t block_count>
class MemoryAlloctor : public MemoryPool {
public:
	MemoryAlloctor() {
		const size_t void_ptr_size = sizeof(void *);
		size_ = (size / void_ptr_size) * void_ptr_size + (size % void_ptr_size ? void_ptr_size : 0);
		block_count_ = block_count;
	}
};

#endif //HSERVER_MEMORYPOOL_HPP
