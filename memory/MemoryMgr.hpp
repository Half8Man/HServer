//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_MEMORYMGR_HPP
#define HSERVER_MEMORYMGR_HPP

#include <unordered_map>

#include "../utils/Singleton.hpp"
#include "MemoryPool.hpp"

#define MAX_MEMORY_SZIE 128

class MemoryMgr : public Singleton<MemoryMgr> {
	friend class Singleton<MemoryMgr>;

protected:
	MemoryMgr() {
		InitAllocArray(0, 8, &allocotr_8_);
		InitAllocArray(9, 16, &allocotr_16_);
		InitAllocArray(17, 32, &allocotr_32_);
		InitAllocArray(33, 64, &allocotr_64_);
		InitAllocArray(65, 128, &allocotr_128_);
	}

public:
	void InitAllocArray(int begin, int end, MemoryPool *pool) {
		for (int i = begin; i <= end; ++i) {
			pool_array_[i] = pool;
		}
	}

	void *AllocMem(size_t size) {
		if (size <= MAX_MEMORY_SZIE) {
			return pool_array_[size]->AllocMemory(size);
		}

		auto block = (MemoryBlock *) malloc(size + sizeof(MemoryBlock));
		block->pool_ = nullptr;
		block->next_ = nullptr;
		block->ref_count_ = 1;
		block->is_in_pool_ = false;
		return ((char *) block + sizeof(MemoryBlock));
	}

	static void FreeMem(void *ptr) {
		auto *block = (MemoryBlock *) ((char *) ptr - sizeof(MemoryBlock));

		if (block->is_in_pool_)
			block->pool_->FreeMemory(ptr);
		else if (--block->ref_count_ == 0)
			free(block);
	}

	static void AddRef(void *ptr) {
		auto *block = (MemoryBlock *) ((char *) ptr - sizeof(MemoryBlock));
		++block->ref_count_;
	}

private:
	MemoryAlloctor<8, 100> allocotr_8_;
	MemoryAlloctor<16, 100> allocotr_16_;
	MemoryAlloctor<32, 100> allocotr_32_;
	MemoryAlloctor<64, 100> allocotr_64_;
	MemoryAlloctor<128, 100> allocotr_128_;

	MemoryPool *pool_array_[MAX_MEMORY_SZIE + 1]{};
};

#endif //HSERVER_MEMORYMGR_HPP
