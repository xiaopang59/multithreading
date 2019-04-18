// project4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <thread>

using namespace std;

// 自己创建的线程也要从一个函数（初始函数）开始运行；
void myprint()
{
	cout << "我的线程开始执行了" << endl;
	//....
	//....
	cout << "我的线程执行完毕了1" << endl;
	cout << "我的线程执行完毕了2" << endl;
	cout << "我的线程执行完毕了3" << endl;
	cout << "我的线程执行完毕了4" << endl;
	cout << "我的线程执行完毕了5" << endl;
	cout << "我的线程执行完毕了6" << endl;
	cout << "我的线程执行完毕了7" << endl;
	cout << "我的线程执行完毕了8" << endl;
	cout << "我的线程执行完毕了9" << endl;
	cout << "我的线程执行完毕了10" << endl;
}

int main()
{
	//一：范例演示线程运行的开始和结束
	//程序运行起来，生成一个进程，该进程所属的主线程开始自动运行；
	//cout << "I Love China!" << endl;
	//实际上这个是主线程在运行，主线程从main()函数返回，则整个进程执行完毕。

	//主线程从main()开始执行，那么我们自己创建的线程，也需要从一个函数开始运行（初始函数），一旦这个函数运行完毕，就代表着我们这个线程运行结束。

	//整个进程是否执行完毕的标志是 主线程是否执行完，如果主线程执行完毕了，就代表整个进程执行完毕了。
	//此时，一般情况下，如果其他子线程还没有执行完毕，那么这些子线程也会被操作系统强行终止。
	//所以，一般情况下，我们得到一个结论：如果大家想保持子线程（自己用代码创建的线程）的运行状态的话，那么大家要让主线程一直保持 运行，不要让主线程运行完毕；
	//【这条规律有例外，老师后续会结束这种例外，大家目前先这样理解和记忆】
	//a）包含 一个头文件thread要包含近来
	//b）初始函数要写。
	//c）main中要开始写代码
	//大家必须明确一点：有两个线程在跑，相当整个程序的执行有两条线在同时走，所以，可以同时干两个事，即使一条线被堵住了，另一条线还是可以通行的。这就是多线程
	//（1.1）thread：是个标准库里的类
	//（1.2）join()：加入/汇合，说白了就是阻塞，阻塞主线程，让主线程等待子线程执行完毕，然后子线程和主线程汇合，然后主线程再往下走。
	//如果主线程执行完毕了，但子线程没执行完毕，这种程序员是不合格的，写出来的程序也是不稳定的；
	//一个书写良好的程序，应该是主线程 等待子线程执行完毕后，自己才能最终退出；
	//（1.3）detach()：传统多线程程序主线程要等待子线程执行完毕，然后自己再最后退出；
	//detach：分离，也就是主线程不和子线程汇合了，你主线程执行你的，我子线程执行我的，你主线程也不必等我子线程运行结束，你可以先执行结束，这并不影响我子线程的执行
	//为什么引入detach()：我们创建了很多子线程，让主线程逐个等待子线程结束，这种变成方法不太好，所以引入了detach();
	//一旦detach()之后，与这个主线程关联的thread对象就会失去与这个主线程的关联，此时这个子线程就会驻留在后台运行（主线程跟子线程失去联系）
	//这个子线程就相当于被C++运行时库接管，当这个子线程执行完毕后，由运行时库负责清理该线程相关的资源（守护线程）。
	//detach()使线程myprint失去我们自己的控制。
	//（1.4）joinable()：判断是否可以成功使用join()或者detach()的；返回true（可以join或者detach）或者false（不能join或者detach）

	//myprint可调用对象。
	thread mytobj(myprint);
	//(1)创建了线程，线程执行起点（入口）myprint()；(2)myprint线程开始执行。
	if (mytobj.joinable())
	{
		mytobj.join();
		cout << "1:joinable() == true" << endl;
	}
	else
	{
		cout << "2:joinable() == false" << endl;
	}

	// 阻塞主线程并等待myprint子线程执行完
	//mytobj.join();
	//主线程阻塞到这里等待myprint()执行完，当子线程执行完毕，这个join()就执行完毕，主线程就继续往下走
	mytobj.detach();
	//一旦调用了detach()，就不能再用join()，否则系统会报告异常。
	//mytobj.join();
	if (mytobj.joinable())
	{
		mytobj.detach();
		cout << "1:joinable() == true" << endl;
	}
	else
	{
		cout << "2:joinable() == false" << endl;
	}

	cout << "I Love China!1" << endl;
	cout << "I Love China!2" << endl;
	cout << "I Love China!3" << endl;
	cout << "I Love China!4" << endl;
	cout << "I Love China!5" << endl;

	return 0;
}



class TA
{
public:
	int m_i;
	TA(int &i) : m_i(i)
	{
		cout << "TA()构造函数被执行" << endl;
	}
	TA(const TA &ta) :m_i(ta.m_i)
	{
		cout << "TA()拷贝构造函数被执行" << endl;
	}
	~TA()
	{
		cout << "TA()析构函数被执行" << endl;
	}
	void operator() ()
		// 不带参数
	{
		//cout << "我的operator()开始执行了" << endl;
		////...
		//cout << "我的operator()结束执行了" << endl;
		cout << "m_i1的值为:" << m_i << endl;
		// 产生不可预料的结果。
		cout << "m_i2的值为:" << m_i << endl;
		cout << "m_i3的值为:" << m_i << endl;
		cout << "m_i4的值为:" << m_i << endl;
		cout << "m_i5的值为:" << m_i << endl;
		cout << "m_i6的值为:" << m_i << endl;
	}
};

int main02()
{
	//二：其他创建线程的手法
	//（2.1）用类对象（可调用对像），以及一个问题范例
	//大家可能还有一个疑问：一旦调用了detach()，那我主线程执行结束了，我这里用的ta这个对象还在吗？（对象不在了）
	//这个对象实际上是被 复制 到线程中去；执行完主线程后，ta会被销毁，但是所复制的TA对象依旧存在。
	//所以，只要你这个TA类对象里没有引用，没有指针，那么就不会产生问题；

	int myi = 6;
	TA ta(myi);
	thread mytobj3(ta);
	//ta:可调用对象
	//mytobj3.join();
	// 等待子线程执行结束
	mytobj3.detach();

	cout << "I Love China!" << endl;

	return 0;
}




int main03()
{
	//（2.2）用lanbda表达式
	auto mylamthread = [] {
		cout << "我的线程3开始执行了" << endl;
		//....
		cout << "我的线程3执行结束了" << endl;
	};
	thread mytobj4(mylamthread);
	//mytobj4.join();
	mytobj4.detach();

	return 0;
}