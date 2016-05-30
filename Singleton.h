#pragma once

template<class T>
class Singleton {
private:
	static T* _instance = 0;
	Singleton() {};
public:

	static T& Instance() {
		if (!_instance) _instance = new T();

	}
};