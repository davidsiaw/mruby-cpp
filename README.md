# mruby-cpp

This is a header-only wrapper over mruby. You still need to pull mruby into your project. That is not covered in this README.

## This repo has been moved to gitlab.astrobunny.net

Contributors please change your upstream to

```
git remote set-url upstream https://gitlab.astrobunny.net/astrobunny/mruby-cpp
```

Likewise if you checked this code out from github before, please change the repo you pull from using this command:

```
git remote set-url origin https://gitlab.astrobunny.net/astrobunny/mruby-cpp
```

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
