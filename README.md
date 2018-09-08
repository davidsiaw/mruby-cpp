# mruby-cpp


[![CircleCI](https://circleci.com/gh/davidsiaw/mruby-cpp.svg?style=shield)](https://circleci.com/gh/davidsiaw/mruby-cpp)
[![Coverage Status](https://coveralls.io/repos/github/davidsiaw/mruby-cpp/badge.svg?branch=master)](https://coveralls.io/github/davidsiaw/mruby-cpp?branch=master)

This is a header-only wrapper over mruby. You still need to pull mruby into your project. That is not covered in this README.

# Getting Started

Add the repository directory to your compiler include directory list

```
CPPFLAGS="$CPPFLAGS -Imruby-cpp"
```

Include mruby.hpp

```c++
#include <mruby.hpp>
```

Its that simple!

# Using

### Create an instance of the mruby VM

```c++
mruby::VM vm;
```

### Run a script

```c++
vm.run("puts 'hello world!'");

# hello world!
```

### Add a function

```c++
int myfunction(int a, int b)
{
	return a+b;
}

vm.bind_method("myfunction", myfunction);
vm.run("p myfunction 5,6")

# 11
```

# Testing

You can test by running

```
make test
```

This will download mruby and compile it to test the headers.

# Advice

Have fun with mruby!
