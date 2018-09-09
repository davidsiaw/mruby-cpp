#ifndef __MRUBYNATIVEOBJECT_HPP__
#define __MRUBYNATIVEOBJECT_HPP__

#include <mruby.h>

#include <string>
#include <memory>

template<class TClass>
class NativeObject
{
	std::string classname;
	mrb_data_type datatype;
	std::shared_ptr<TClass> instance;

	static void destructor(mrb_state* mrb, void* ptr)
	{
		delete (NativeObject<TClass>*)ptr;
	}

public:
	NativeObject(const std::string& classname, std::shared_ptr<TClass> instance) :
		classname(classname),
		instance(instance)
	{
		datatype.dfree = destructor;
		datatype.struct_name = classname.c_str();
	}

	~NativeObject()
	{
		//printf("NativeObject<%s> %p deleted\n", typeid(TClass).name(), this);
	}

	mrb_data_type* get_type_ptr()
	{
		return &datatype;
	}

	TClass* get_instance() const
	{
		return instance.get();
	}

	std::shared_ptr<TClass> get_shared_instance() const
	{
		return instance;
	}

	std::string get_classname() const
	{
		return classname;
	}

	TClass* operator->()
	{
		return instance.operator->();
	}

	TClass &operator*()
	{
		return instance.operator*();
	}
};

#endif // __MRUBYNATIVEOBJECT_HPP__
