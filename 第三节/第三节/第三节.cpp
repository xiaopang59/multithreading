// 第三节.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <thread>

using namespace std;

//void myprint(const int &i, char *pmybuf)
//void myprint(const int i, char *pmybuf)
void myprint01(const int i, const string &pmybuf)
{
	cout << i << endl;
	//分析认为，i并不是mvar的引用，实际是值传递，那么我们认为，即便主线程detach了子线程，那么子线程中用i值应该是安全的
	// //cout << pmybuf << endl;
	//指针在detach子线程时，绝对会有问题
	cout << pmybuf.c_str() << endl;
	return;
}

int main01()
{
	//一：传递临时对象作为线程参数
	//（1.1）要避免的陷阱（解释1）
	//（1.2）要避免的陷阱（解释2）
	//事实1：只要用临时构造的A类对象作为参数传递给线程，那么就一定能够在主线程执行完毕前把线程函数的第二个参数构建出来，从而确保及时detach()子线程也能够安全运行
	//（1.3）总结
	//(a)若传递int这种简单类型参数，建议都是值传递，不要用引用。防止节外生枝。
	//(b)如果是传递类对象，避免隐式类型转换。全部都在创建线程这一行就构建出临时对象来，然后在函数参数里用引用来接；否则系统还会多构造一次对象，浪费；
	//终极结论：
	//(c)建议不使用detach()，只是用join()：这样就不存在局部变量失效导致线程对内存的非法引用问题；

	//二、临时对象作为线程参数继续讲，老师常用测试大发；
	//（2.1）线程id概念：id是个数字，每个线程（不管是主线程还是子线程）实际上都对应着一个数字，而且每个数字对应的这个数字都不同。
	//也就是，不同的线程，它的线程id（数字）必然是不同；
	//线程id可以用C++标准库里的函数来获取。std::this_thread::get_id()来获取；

	//三：传递类对象、智能指针作为线程参数
	//std::ref 函数

	//四：用成员函数指针做线程函数operator()


	int mvar = 1;
	int &mvary = mvar;
	char mybuf[] = "this is a test!";
	//thread mytobj(myprint, mvar, mybuf);
	//但是mybuf到底是什么时候转换成string。
	//事实上存在，mybuf都被回收了（main函数执行完了），系统才用mybuf去转string的可能性；
	thread mytobj(myprint01, mvar, string(mybuf));
	//我们这里直接将mybuf转换成string对象，这是 一个可以保证在线程中肯定有效的。
	//myobj.join();
	mytobj.detach();
	//子线程和主线程分别进行
	cout << "I Love China!" << endl;

	return 0;
}



class A
{
public:
	//int m_i;
	mutable int m_i;
	//类型转换构造函数，可以把一个int转换成一个类A对象。
	A(int a) :m_i(a)
	{
		//cout << "[A::A(int a)构造函数执行]" << this  << endl;
		cout << "[A::A(int a)构造函数执行]" << this << " thread = " << std::this_thread::get_id() << endl;
	}
	A(const A &a) :m_i(a.m_i)
	{
		//cout << "[A::A(int a)拷贝构造函数执行]" << this << endl;
		cout << "[A::A(int a)拷贝构造函数执行]" << this << " thread = " << std::this_thread::get_id() << endl;
	}
	~A()
	{
		cout << "[A::A()析构函数执行]" << this << endl;
		cout << "[A::A()析构函数执行]" << this << " thread = " << std::this_thread::get_id() << endl;
	}

	void thread_work(int num)
		//来个参数
	{
		cout << "【子线程thread_work执行】" << this << " thread = " << std::this_thread::get_id() << endl;
	}

	void operator() (int num)
	{
		cout << "【子线程()执行】" << this << " thread = " << std::this_thread::get_id() << endl;
	}
};

void myprint02(const int i, const A &pmybuf)
{
	cout << &pmybuf << endl;
	//这里打印的是pmybuf对象的地址
	return;
}

int main02()
{
	int mvar = 1;
	int mysecondpar = 12;
	//thread mytobj(myprint, mvar, mysecondpar);
	// 我们是希望mysecondpar转换成A类型对象传递给myprint的第二个参数
	thread mytobj(myprint02, mvar, A(mysecondpar));
	// 在创建线程的同事构造临时对象的方法传递参数是可行的；
	//mytobj.join();
	mytobj.detach();
	//子线程和主线程分别执行。
	cout << "I Love China!" << endl;

	return 0;
}



void myprint(const A &pmybuf)
{
	cout << "子线程myprint2的参数地址是: " << &pmybuf << " thread = " << std::this_thread::get_id() << endl;
	//致命的问题居然是在子线程中构造的A类对象
	//用了临时对象后，所有的A类对象都在main()函数中就已经构建完毕了
	return;
}

int main03()
{
	cout << "主线程id是: " << std::this_thread::get_id() << endl;
	int mvar = 1;
	//std::thread mytobj(myprtint, mvar);
	std::thread mytobj(myprint, A(mvar));
	//mytobj.join();
	mytobj.detach();
	//子线程和主线程分别执行。
	//cout << "I Love China!" << endl;

	return 0;
}




void myprint03(const A &pmybuf)
{
	pmybuf.m_i = 199;
	//我们修改该值不会影响到main函数
	cout << "子线程myprint2的参数地址是: " << &pmybuf << " thread = " << std::this_thread::get_id() << endl;
	return;
}

void myprint04(A &pmybuf)
{
	pmybuf.m_i = 199;
	//我们修改该值不会影响到main函数
	cout << "子线程myprint2的参数地址是: " << &pmybuf << " thread = " << std::this_thread::get_id() << endl;
	return;
}

int main04()
{
	A myobj(10);
	//生成一个类对象；
	//std::thread mytobj(myprint02, myobj);
	//mytobj将类对象作为线程参数
	std::thread mytobj(myprint04, std::ref(myobj));

	mytobj.join();

	return 0;
}



void myprint05(unique_ptr<int> &pzn)
{
	return;
}

int main05()
{
	unique_ptr<int> myp(new int(100));
	std::thread mytobj(myprint05, std::move(myp));

	mytobj.join();
	//只能用join，不能用detach
	//mytobj.detach();
	//子线程和主线程分别执行。

	return 0;
}



int main06()
{
	A myobj(10);
	//生成一个类对象
	std::thread mytobj(&A::thread_work, myobj, 15);
	//join和detach都可以 
	//std::thread mytobj(&A::thread_work, std::ref(myobj), 15);
	//只能用join
	//std::thread mytobj(&A::thread_work, &myobj, 15);
	//&myobj == std;:ref(myobj)

	mytobj.join();

	return 0;
}



int main()
{
	A myobj(10);
	//生成一个类对象
	//std::thread mytobj(myobj, 15);
	std::thread mytobj(std::ref(myobj), 15);
	//不调用拷贝构造函数了，那么后续如果调用mytobj.detach()就不安全了；

	mytobj.join();

	return 0;
}
