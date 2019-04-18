// 第八节.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <condition_variable>

using namespace std;

class A
{
public:
	//把收到的消息（玩家命令）入到一个队列的线程
	void inMsgRecvQueue()	//unlock()
	{
		for (int i = 0; i < 100000; ++i)
		{
			std::unique_lock<std::mutex> sbguard1(my_mutex1);
			cout << "inMsgRecvQueue()执行，插入一个元素" << i << endl;
			msgRecvQueue.push_back(i);
			//假设这个数字i就是我收到的命令，我直接弄到消息队列里边来；

			//假如outMsgRecvQueue()正在处理一个事务，需要一段时间，而不是正卡在wait()那里等待你唤醒，那么此时这个notify_one()这个调用也许就没效果；
			//my_cond.notify_one();
			//我们尝试把wait()的线程唤醒，执行完这行，那么outMsgRecvQueue()里边的wait就会被唤醒
			//唤醒之后的事情后续研究；
			my_cond.notify_all();
			//......
			//其他处理代码；
		}
		return;
	}

	//bool outMsgLULProc(int &command)
	//{
	//  //双重锁定，双重检查
	//  if (!msgRecvQueue.empty())
	//  {
	//  std::unique_lock<std::mutex> sbguard1(my_mutex1); 

	//  if (!msgRecvQueue.empty())
	//  {
	//  //消息不为空
	//  command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在；
	//  msgRecvQWueue.pop_front(); //移除第一个元素，但不返回；
	//  return true;
	//  }
	//  }
	//  return false;
	//}

	//把数据从消息队列中取出的线程：
	void outMsgRecvQueue()
	{
		int command = 0;

		//for (int i = 0; i < 100000; ++i)
		//{
		//  bool result = outMsgLULProc(command);
		//  if (result == true)
		//  {
		//  cout << "outMsgRecvQueue()执行，取出一个元素" << command << endl;
		//  //可以考虑进行命令（数据）处理
		//  //.....
		//  }
		//  else
		//  {
		//  //sleep(100)
		//  //消息队列为空
		//  cout << "outMsgEecvQueue()执行，但目前消息队列中为空" << i << endl;
		//  }
		//}
		//cout << endl;

		while (true)
		{
			std::unique_lock<std::mutex> sbguard1(my_mutex1);

			//wait()用来等一个东西
			//如果第二个参数lambda表达式返回值是true，那wait()直接返回；
			//如果第二个参数lambda表达式返回值是false，那么wait()将解锁互斥量，并堵塞到本行，
			//那堵塞到什么时候为止呢？堵塞到其他某个线程调用notify_one()成员函数为止；
			//如果wait()没有第二个参数	my_cond.wait(sbguard1)；那么就跟第二个参数lambda表达式返回false效果一样
				//wait()将解锁互斥量，并堵塞到本行，堵塞到其他某个线程调用notify_one()成员函数为止；
				//当其他线程用notify_one()将本wait（原来是睡着/堵塞）的状态唤醒后，wait就开始恢复干活了，恢复后wait干什么活？
				//a）wait()不断的尝试重新获取互斥量锁，如果获取不到，那么流程就卡在wait这里等着获取，如果获取到了锁（等于加了锁），那么wait就继续执行b；
				//b）
				//b.1）如果wait有第二个参数(lambda)，就判断这个lambda表达式，如果lambda表达式为false，那么wait又对互斥量解锁，然后又休眠在这里等待再次被notify_one唤醒
				//b.2）如果lambda表达式为true，则wait返回，流程走下来（此时互斥锁被锁着）。
				//b.3）如果wait没有第二个参数，则wait返回，流程走下来。
				my_cond.wait(sbguard1, [this] {	//一个lambda就是一个可调用对象（函数）
				if (!msgRecvQueue.empty())
					return true;
				return false;
			});

			//流程只要能走到这里来，这个互斥锁一定是锁着的。
			//一会再写其他的...
			command = msgRecvQueue.front();	//返回第一个元素，但不检查元素是否存在；
			msgRecvQueue.pop_front();	//移除第一个元素，但不返回；
			cout << "outMsgRecvQueue()执行，取出一个元素: " << command << "threadid = " << std::this_thread::get_id() << endl;
			sbguard1.unlock();	//因为unique_lock的灵活性，所以我们可以随时的unlock解锁，以免锁住太长时间

			//执行一些其他的动作，帮助玩家抽卡，抽卡需要100毫秒的处理时间；
			//...
			//执行100毫秒
			//

		}  //end while
	}

private:
	std::list<int> msgRecvQueue;	//容器，专门用于代表玩家给咱们发送过来的命令。
	std::mutex my_mutex1;	//创建一个互斥量（一把锁头）
	std::condition_variable my_cond;	//生成一个条件变量对象
};

int main()
{
	//一：条件变量std::condition_variable、wait()、notify_one()：只能通知一个outMsgRecvQueue线程
	//线程A：等待一个条件满足
	//线程B：专门往消息队列中扔消息（数据）
	//std::condition_variable实际上是一个类，是一个和条件相关的一个类，说白了就是等待一个条件达成。
	//这个类需要和互斥量来配合工作，用的时候我们要生成这个类的对象；


	//二：上述代码深入思考


	//三：notify_all()


	A myobja;
	std::thread myOutnMsgObj(&A::outMsgRecvQueue, &myobja);	//第二个参数 引用，才能保证线程里 用的是同一个对象
	std::thread myOutnMsgObj2(&A::outMsgRecvQueue, &myobja);
	std::thread myInMsgObj(&A::inMsgRecvQueue, &myobja);

	myInMsgObj.join();
	myOutnMsgObj.join();
	myOutnMsgObj2.join();

	return 0;
}
