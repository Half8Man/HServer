//
// Created by wangj on 2020/6/10.
//

#ifndef HSERVER_SINGLETON_HPP
#define HSERVER_SINGLETON_HPP

template<typename T>
class Singleton {
public:
	static T &GetInstance() {
		static T instance;
		return instance;
	}

	Singleton(const Singleton &) = delete;

	Singleton &operator=(const Singleton &) = delete;

	virtual ~Singleton() {

	}

protected:
	Singleton() {

	}
};

#endif //HSERVER_SINGLETON_HPP
