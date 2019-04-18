// 第十一节.cpp : 定义控制台应用程序的入口点。
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

using namespace std;

std::atomic<int> g_mycout = 0;	//这是个原子整型类型变量；可以向使用整型变量一样使用
void mythread1()
{
	for (int i = 0; i < 10000000; i++)
	{
		g_mycout++;	//对应的操作就是原子操作，不会被打断；
		g_mycout += 1;
		//g_mycout = g_mycout + 1;	//结果不对
	}
	return;
}


int main01()
{
	//一：原子操作std::atomic续谈
	//一般atomic原子操作，针对++，--，+=，&=，|=，^=是支持的。其他的可能不支持。

	std::thread mytobj1(mythread1);
	std::thread mytobj2(mythread1);
	mytobj1.join();
	mytobj2.join();
	cout << "两个线程都执行完毕，最终的g_mycout的结果是" << g_mycout << endl;

	return 0;
}




int mythread2()
{
	cout << "mythread start" << " threadid = " << std::this_thread::get_id() << endl;
	return 1;
}

int main02()
{
	//二：std::async深入谈
	//（2.1）std::async参数详述，async用来创建一个异步任务
	cout << "main start" << " threadid = " << std::this_thread::get_id() << endl;
	//std::future<int> result = std::async(std::launch::deferred, mythread2);	//deferred延迟调用，并且不创建新线程，延迟到future对象调用.get()或者.wait()的时候才执行mythread()
	//std::future<int> result = std::async(std::launch::async, mythread2);
	//std::future<int> result = std::async(std::launch::async | std::launch::deferred, mythread2);
	std::future<int> result = std::async(mythread2);
	cout << result.get() << endl;

	//老师讲解过：参数 std::launch::deferred【延迟调用】，以及std::launch::async【强制创建一个线程】
	//std::thread() 如果系统资源紧张，那么可能创建线程就会失败，那么执行std::thread()时整个程序可能会崩溃。
	//std::async()我们一般不叫创建线程（解释async能够创建线程），我们一般叫它创建 一个异步任务。
	//std::async和std::thread最明显的不同，就是async有时候并不创建新线程。
	//a）如果你用std::launch::deferred来调用async会怎么样？
	//std::launch::deferred延迟调用，并且不创建新线程，延迟到future对象调用.get()或者.wait()的时候才执行mythread()，如果没有调用get或者wait，那么这个mythread就不会执行。
	//b）std::launch::async：强制这个异步任务在新线程上执行，这意味着，系统必须要给我创建出新线程来运行mythread()；
	//c）std::launch::async | std::launch::deferred
	//这里这个|：意味着调用async的行为可能是 “创建新线程并立即执行” 或者 
											//“没有创建新线程并且延迟到调用 result.get()才开始执行任务入口函数，	两者居其一”
	
	//d）我们不带额外参数；只给一个async函数一个 入口函数名；
	//第九节课老师产生了一点小错误，其实，默认值应该是std::launch::async | std::launch::deferred；和c）效果完全一致。
		//换句话说：系统会自行决定是异步（创建新线程）还是同步（不创建新线程）方式运行。

	//自行决定是啥意思？系统如何决定是 异步（创建新线程）还是同步（不创建新线程）方式运行
	//（2.2）std::async和std::thread的区别
	//std::thread创建线程，如果系统资源紧张，创建线程失败，那么整个程序就会报异常崩溃（有脾气）
	//int mythread(){return 1;}
	//std::thread mytobj(mythread);
	//mytobj.join();
	//std::thread创建线程的方式，如果线程返回值，你想拿到这个值也不容易；
	//std::async创建异步任务。可能创建也可能不创建线程。并且async调用方法很容易拿到线程入口函数的返回值；
	//由于系统资源限制：
	//（1）如果用std::thread创建的线程太多，则可能创建失败，系统报告异常，崩溃。
	//（2）如果用std::async，一般就不会报异常不会崩溃，因为，如果系统资源紧张导致无法创建新线程的时候，
		//std::async这种不加额外参数的调用 就不会创建新线程。而是后续谁调用了result.get()来请求结果，
			//那么这个异步任务mythread就运行在执行这条get()所在的线程上。
	//如果你强制std::async一定 要创建新线程，那么就必须使用 std::launch::async。承受的代价就是系统资源紧张时，程序崩溃。
	//（3）经验：一个程序，线程数量不宜超过100-200，时间片。

	return 0;
}




int mythread3()
{
	cout << "mythread start" << " threadid = " << std::this_thread::get_id() << endl;

	std::chrono::milliseconds dura(5000);	//1秒 = 1000毫秒，所以5000毫秒 = 5秒
	std::this_thread::sleep_for(dura);		//休息一定的时长

	return 1;
}

int main()
{
	//（2.3）std::async不确定性问题的解决
	//不加额外参数的std::async调用，让系统自行决定是否创建新线程。
	//问题焦点在于 std::future<int> result = std::async(mythread)；写法
	//这个异步任务到底有没有被推迟执行，（std::launch::async还是std::launch::deferred）
	//std::future对象的wait_for函数，第10节讲过。

	cout << "main start" << " threadid = " << std::this_thread::get_id() << endl;
	std::future<int> result = std::async(mythread3);	//想判断async到底有没有创建新线程立即执行还是延迟（没创建新线程）执行。

	std::future_status status = result.wait_for(std::chrono::seconds(0));	
	if (status == std::future_status::deferred)
	{
		//线程被延迟执行了（系统资源紧张了，它给我采用std::launch::deferred策略了）
		cout << result.get() << endl;	//这个时候才去调用了mythread()；
	}
	else
	{
		//任务没有被推迟，已经开始运行了呗，线程被创建了；
		if (status == std::future_status::ready)
		{
			//线程成功返回
			cout << "线程成功执行完毕并返回！" << endl;
			cout << result.get() << endl;
		}
		else if (status == std::future_status::timeout)
		{
			//超时线程还没执行完
			cout << "超时线程还没执行完!" << endl;
			cout << result.get() << endl;
		}
	}

	return 0;
}