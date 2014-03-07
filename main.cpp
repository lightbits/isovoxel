#include <iostream>
using namespace std;

class Foo
{
public:
	Foo() { }
	Foo(int X) : x(X) { }
	Foo(const Foo &copy)
	{
		*this = copy;
	}

	Foo operator=(const Foo &rhs) // crashes due to recursion
	{
		x = rhs.x;
		return *this;
	}

	void bar() { cout << x << endl; }
private:
	int x;
};

int main()
{
	Foo foo1(5);
	Foo foo2(foo1);

	foo1.bar();
	foo2.bar();

	return 0;
}