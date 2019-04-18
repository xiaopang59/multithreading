// 第十二节.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <future>
#include <windows.h>

using namespace std;

//#define __WINDOWSJQ_

//把类用于自动释放windows下的临界区，防止忘记LeaveCriticalSection导致死锁情况的发生，类似于c++11中的std::lock_guard<std::mutex>
class CWinLock	//叫RAII类（Resource Acquisition is initialization）中文“资源获取即初始化”
					//容器，智能指针这种类，都属于RAII类
{
public:
	CWinLock(CRITICAL_SECTION *pCritmp)	//构造函数
	{
		m_pCritical = pCritmp;
		EnterCriticalSection(m_pCritical);
	}

	~CWinLock()	//析构函数
	{
		LeaveCriticalSection(m_pCritical);
	}

private:
	CRITICAL_SECTION *m_pCritical;
};

class A
{
public:
	//把收到的消息到队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; ++i)
		{
			cout << "inMsgRecvQueue()执行，插入一个元素" << i << endl;

#ifdef __WINDOWSJQ_
			//EnterCriticalSection(&my_winsec);	//进入临界区（加锁）
			//EnterCriticalSection(&my_winsec);
			CWinLock wlock(&my_winsec);		//wlock，wlock2 都属于RAII对象。
			CWinLock wlock2(&my_winsec);	//调用多次也没问题；
			msgRecvQueue.push_back(i);
			//LeaveCriticalSection(&my_winsec);	//离开临界区（解锁）
			//LeaveCriticalSection(&my_winsec);
#else
			//my_mutex.lock();
			//my_mutex.lock();	//报异常，和windows有区别；
			std::lock_guard<std::mutex> sbguard(my_mutex);
			//std::lock_guard<std::mutex> sbguard1(my_mutex);
			msgRecvQueue.push_back(i);	//假设这个数字i就是我收到的命令，我直接弄到消息队列里边来；
			//my_mutex.unlock();
			//my_mutex.unlock();
#endif
		}
	}

	bool outMsgLULProc(int &command)
	{
#ifdef __WINDOWSJQ_
		EnterCriticalSection(&my_winsec);
		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();	//返回第一个元素但不检查元素存在与否
			msgRecvQueue.pop_front();
			LeaveCriticalSection(&my_winsec);
			return true;
		}
		LeaveCriticalSection(&my_winsec);
#else
		my_mutex.lock();
		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();	//返回第一个元素但不检查元素存在与否
			msgRecvQueue.pop_front();
			my_mutex.unlock();
			return true;
		}
		my_mutex.unlock();
#endif

		return false;
	}

	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; ++i)
		{
			bool result = outMsgLULProc(command);
			if (result == true)
			{
				cout << "outMsgRecvQueue()执行，取出一个元素" << command << endl;
				//这里可以考虑处理数据
				//.....
			}
			else
			{
				cout << "outMsgEecvQueue()执行，但目前消息队列中为空" << i << endl;
			}
		}
		cout << "end" << endl;
	}

	A()
	{
#ifdef __WINDOWSJQ_
		InitializeCriticalSection(&my_winsec);	//用临界区之前要先初始化
#endif
	}


private:
	std::list<int> msgRecvQueue;	//容器，专门用于代表玩家给咱们发送过来的命令
	std::mutex my_mutex;	//创建互斥量

#ifdef __WINDOWSJQ_
	CRITICAL_SECTION my_winsec;	//windows种的临界区，非常类似于C++11种的mutex
#endif
};

int main01()
{
	//一：windows临界区
	//二：多次进入临界区试验
	//在同一个线程（不同线程就会卡住等待）中，windows中的“相同临界区变量”代表的临界区的进入（EnterCriticalSection）可以被多次调用
		//但是你调用了几次EnterCriticalSection，你就得调用几次LeaveCriticalSection(&my_winsec);
		//而在c++11中，不允许 同一线程中lock同一个互斥量多次，否则报异常

	//三：自动析构技术
	//std::lock_guard<std::mutex>

	//四：recursive_mutex递归的独占互斥量

	A myobja;
	std::thread myOutnMsgObj(&A::outMsgRecvQueue, &myobja);	//注意这里第二个参数必须时引用，才能保证线程里用的是同一个对象
	std::thread myInMsgObj(&A::inMsgRecvQueue, &myobja);
	myInMsgObj.join();
	myOutnMsgObj.join();

	system("pause");
	return 0;
}




class AA
{
public:
	//把收到的消息到队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; ++i)
		{
			cout << "inMsgRecvQueue()执行，插入一个元素" << i << endl;

			//my_mutex.lock();
			//my_mutex.lock();	//报异常，和windows有区别；
			std::lock_guard<std::recursive_mutex> sbguard(my_mutex);
			testfunc1();	//加了三次锁，报异常；（只要lock超过一次就报异常）

			//std::lock_guard<std::mutex> sbguard1(my_mutex);
			msgRecvQueue.push_back(i);	//假设这个数字i就是我收到的命令，我直接弄到消息队列里边来；
			//my_mutex.unlock();
			//my_mutex.unlock();
		}
	}

	bool outMsgLULProc(int &command)
	{
		my_mutex.lock();
		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();	//返回第一个元素但不检查元素存在与否
			msgRecvQueue.pop_front();
			my_mutex.unlock();
			return true;
		}
		my_mutex.unlock();

		return false;
	}

	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; ++i)
		{
			bool result = outMsgLULProc(command);
			if (result == true)
			{
				cout << "outMsgRecvQueue()执行，取出一个元素" << command << endl;
				//这里可以考虑处理数据
				//.....
			}
			else
			{
				cout << "outMsgEecvQueue()执行，但目前消息队列中为空" << i << endl;
			}
		}
		cout << "end" << endl;
	}

	void testfunc1()
	{
		std::lock_guard<std::recursive_mutex> sbguard(my_mutex);
		//......干各种事情
		testfunc2();	//悲剧了，崩溃；
	}

	void testfunc2()
	{
		std::lock_guard<std::recursive_mutex> sbguard(my_mutex);
		/*std::lock_guard<std::recursive_mutex> sbguard2(my_mutex);
		std::lock_guard<std::recursive_mutex> sbguard3(my_mutex);
		std::lock_guard<std::recursive_mutex> sbguard4(my_mutex);
		std::lock_guard<std::recursive_mutex> sbguard5(my_mutex);*/


		//......干各种另外一些事情
	}


private:
	std::list<int> msgRecvQueue;	//容器，专门用于代表玩家给咱们发送过来的命令
	//std::mutex my_mutex;	//创建互斥量
	std::recursive_mutex my_mutex;	//递归独占互斥量
};

int main02()
{
	//四：recursive_mutex递归的独占互斥量
	//std::mutex：独占互斥量，自己lock时别人lock不了；
	//std::recursive_mutex：递归的独占互斥量：允许同一线程，同一个互斥量多次被，lock()，效率上比mutex更差一些；
		//recursive_mutex也有lock，也有unlock()；
			//考虑代码是否有优化空间。
		//递归次数据说有限制，第柜台太多可能报异常。

	AA myobja;
	std::thread myOutnMsgObj(&AA::outMsgRecvQueue, &myobja);	//注意这里第二个参数必须时引用，才能保证线程里用的是同一个对象
	std::thread myInMsgObj(&AA::inMsgRecvQueue, &myobja);
	myInMsgObj.join();
	myOutnMsgObj.join();

	system("pause");
	return 0;
}





class AAA
{
public:
	//把收到的消息到队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; ++i)
		{
			cout << "inMsgRecvQueue()执行，插入一个元素" << i << endl;

			std::chrono::milliseconds timeout(100);	//等待100毫秒
			//if (my_mutex.try_lock_for(timeout))	//等待100毫秒来尝试 获取锁
			if (my_mutex.try_lock_until(chrono::steady_clock::now() + timeout))
			{
				//在这100毫秒之内拿到了锁
				msgRecvQueue.push_back(i);	//假设这个数字i就是我收到的命令，我直接弄到消息队列里边来；
				my_mutex.unlock();	//用完了要解锁；
			}
			else
			{
				//这次没拿到锁头
				std::chrono::milliseconds sleeptime(100);
				std::this_thread::sleep_for(sleeptime);
			}
		}
	}

	bool outMsgLULProc(int &command)
	{
		my_mutex.lock();

		std::chrono::milliseconds sleeptime(200);
		std::this_thread::sleep_for(sleeptime);

		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();	//返回第一个元素但不检查元素存在与否
			msgRecvQueue.pop_front();
			my_mutex.unlock();
			return true;
		}
		my_mutex.unlock();

		return false;
	}

	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; ++i)
		{
			bool result = outMsgLULProc(command);
			if (result == true)
			{
				cout << "outMsgRecvQueue()执行，取出一个元素" << command << endl;
				//这里可以考虑处理数据
				//.....
			}
			else
			{
				cout << "outMsgEecvQueue()执行，但目前消息队列中为空" << i << endl;
			}
		}
		cout << "end" << endl;
	}

private:
	std::list<int> msgRecvQueue;	//容器，专门用于代表玩家给咱们发送过来的命令
	std::timed_mutex my_mutex;	//带超时功能的独占互斥量；
};

int main()
{
	//五：带超时的互斥量std::timed_mutex和std::recursive_timed_mutex
	//std::timed_mutex：是带超时功能的独占互斥量；
		//try_lock_for()：参数是一段时间，是等待一段时间。如果我拿到了锁，或者等待超过时间未拿到锁，就走下来；
		//try_lock_until()：参数是一个未来的时间点，在这个未来的时间没到的时间段内，如果我拿到了锁，那么就走下来；
																				//如果时间到了，没拿到锁，程序流程也走下来。
	//std::recursive_timed_mutex：带超时功能的递归独占互斥量（允许同一线程多次获取这个互斥量）

	AAA myobja;
	std::thread myOutnMsgObj(&AAA::outMsgRecvQueue, &myobja);	//注意这里第二个参数必须时引用，才能保证线程里用的是同一个对象
	std::thread myInMsgObj(&AAA::inMsgRecvQueue, &myobja);
	myInMsgObj.join();
	myOutnMsgObj.join();

	system("pause");
	return 0;
}