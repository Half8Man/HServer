//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_OBJPOOLOBJ_HPP
#define HSERVER_OBJPOOLOBJ_HPP

#include "ObjectPool.hpp"

template<typename T, size_t obj_count>
class ObjPoolObj {
public:
	ObjPoolObj() = default;

	virtual ~ObjPoolObj() = default;

	void *operator new(size_t size) {
		return GetObjPool().AllocObj();
	}

	void operator delete(void *obj) {
		GetObjPool().FreeObj(obj);
	}

	template<typename ...Args>
	static T *CreateObject(Args ... args) {
		T *obj = new T(args...);
		return obj;
	}

	static void DestroyObject(T *obj) {
		delete obj;
	}

private:
	static ObjectPool<T, obj_count> &GetObjPool() {
		static ObjectPool<T, obj_count> pool;
		return pool;
	}
};

#endif //HSERVER_OBJPOOLOBJ_HPP
