#ifndef __MRUBY_HPP__
#define __MRUBY_HPP__

#include <memory>
#include <functional>
#include <sstream>
#include <type_traits>
#include <fstream>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/variable.h>
#include <mruby/string.h>

#include <mrubydefines.hpp>

MRUBY_NAMESPACE_BEGIN

#include <mrubyexception.hpp>
#include <mrubynativeobject.hpp>
#include <mrubytypebinder.hpp>
#include <mrubyfunctional.hpp>
#include <mrubymodule.hpp>
#include <mrubyclass.hpp>
#include <mrubyvm.hpp>

MRUBY_NAMESPACE_END

#endif // __MRUBY_HPP__
