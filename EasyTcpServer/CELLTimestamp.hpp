#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_



#include<chrono>
using namespace std::chrono;
class CELLTimestamp
{
public:
	CELLTimestamp() {
		update();
	};
	~CELLTimestamp() {};
	//更新
	void update() {
		_begin = high_resolution_clock::now();
	};
	//获取当前秒
	double getElapsedSecond() {
		return getElapsedTimeInMicroSec() * 0.000001;
	};
	//获取毫秒
	long getElapsedTimeInMilliSec() {
		return getElapsedTimeInMicroSec() * 0.001;;
	};
	//获取微秒
	long getElapsedTimeInMicroSec() {
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	};
private:
	time_point<high_resolution_clock> _begin;
};
#endif // !_CELLTimestamp_hpp_