#mruby-cpp

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

### Create an instance of mruby

```c++
MRuby mruby;
```

### Run a script

```c++
mruby.run("puts 'hello world!'");
```

### Add a function

```c++
int myfunction(int a, int b)
{
	return a+b;
}

mruby.bind_method("myfunction", myfunction);
mruby.run("p myfunction 5,6")
```

# Testing

You can test by running

```
make test
```

This will download mruby and compile it to test the headers.

# Advice

Have fun with mruby!
