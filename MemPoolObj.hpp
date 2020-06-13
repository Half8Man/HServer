//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_MEMPOOLOBJ_HPP
#define HSERVER_MEMPOOLOBJ_HPP

#include "MemoryMgr.hpp"

class MemPoolObj {
public:
	void *operator new(size_t size) {
		return MemoryMgr::GetInstance().AllocMem(size);
	}

	void operator delete(void *ptr) {
		MemoryMgr::FreeMem(ptr);
	}

	void *operator new[](size_t size) {
		return MemoryMgr::GetInstance().AllocMem(size);
	}

	void operator delete[](void *ptr) {
		MemoryMgr::FreeMem(ptr);
	}
};

#endif //HSERVER_MEMPOOLOBJ_HPP
