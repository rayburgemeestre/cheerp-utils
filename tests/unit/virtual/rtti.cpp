//===---------------------------------------------------------------------===//
//	Copyright 2015 Leaning Technlogies
//===---------------------------------------------------------------------===//

#include <tests.h>

static int A_foo_called = 0;
static int A_bar_called = 0;
static int B_foo_called = 0;
static int B_bar_called = 0;
static int C_foo_called = 0;
static int C_bar_called = 0;
static int D_foo_called = 0;
static int D_bar_called = 0;

class I {
public:
	virtual void foo() = 0;// pure virtual function
	virtual void bar() = 0;// pure virtual function
};

class A : public I {
public:
	virtual void foo() {
		A_foo_called++;
	};

	virtual void bar() {
		A_bar_called++;
	}
};

class B : public A {
public:
	virtual void foo() {
		B_foo_called++;
	};

	virtual void bar() {
		B_bar_called++;
	}
};

class C : public I {
public:
	virtual void foo() {
		C_foo_called++;
	};

	virtual void bar() {
		C_bar_called++;
	}
};

class D : public A, public C {
public:
	virtual void foo() {
		D_foo_called++;
	};

	virtual void bar() {
		D_bar_called++;
	}
};

void testTypeidName() {
	{
		I* a = new A();
		I* t = dynamic_cast<A*>(a);
		const char *name = typeid(t).name();
		assertEqual(name, "P1I", "typeid().name() support 1/N");
	}

	{
		int i = 0;
		const char *name = typeid(i).name();
		assertEqual(name, "i", "typeid().name() support 2/N");
	}

	{
		double *doubleptr = nullptr;
		const std::type_info &ti = typeid(doubleptr);
		assertEqual(ti.name(), "Pd", "typeid().name() support 3/N");
	}

	// Dereferencing a null pointer: okay for a non-polymorphic expression
	{
		double *doubleptr = nullptr;
		const std::type_info &ti = typeid(*doubleptr);
		assertEqual(ti.name(), "d", "typeid().name() support 4/N");
	}

	// Dereferencing a null pointer: not okay for a polymorphic lvalue
	{
		B* bad_ptr = NULL;
		const std::type_info &ti = typeid(*bad_ptr);
		assertEqual(ti.name(), "P1B", "typeid().name() support 5/N");
	}
}

void testDynamicCast()
{
	I* a = new A();
	I* b = new B();
	I* t = dynamic_cast<A*>(a);

	if ((t = dynamic_cast<A*>(a))) {
		t->foo();
	}

	if ((t = dynamic_cast<B*>(a))) {
		t->bar();
	}

	assertEqual(A_foo_called, 1, "A::foo is called");
	assertEqual(A_bar_called, 0, "A::bar is not called");
	assertEqual(B_foo_called, 0, "B::foo is not called");
	assertEqual(B_bar_called, 0, "B::bar is not called");

	if ((t = dynamic_cast<A*>(b))) {
		t->foo();
	}

	if ((t = dynamic_cast<B*>(b))) {
		t->bar();
	}

	assertEqual(A_foo_called, 1, "A::foo is called");
	assertEqual(A_bar_called, 0, "A::bar is not called");
	assertEqual(B_foo_called, 1, "B::foo is called");
	assertEqual(B_bar_called, 1, "B::bar is called");

	void *v = dynamic_cast<void *>(a);
	assertEqual(!v, false, "Cast to void support 1/N");
}

void testMultipleInheritance()
{
	A* a = new A();
	D* d = new D();
	I* t;

	t = dynamic_cast<A*>(d);
	assertEqual(!t, false, "Multiple inheritance support 1/N");

	t = dynamic_cast<C*>(d);
	assertEqual(!t, false, "Multiple inheritance support 2/N");

	D* tmp = dynamic_cast<D*>(a);
	assertEqual(!tmp, true, "Multiple inheritance support 3/N");
}

void webMain()
{
	testDynamicCast();
	testMultipleInheritance();
	testTypeidName();
}

// vim: noexpandtab
