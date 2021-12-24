#ifndef __MRUBYTYPEBINDER_HPP__
#define __MRUBYTYPEBINDER_HPP__

template<typename T>
struct TypeBinder 
{
};

/* internally used datatypes */

template<>
struct TypeBinder<RClass*> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, RClass* cls)
	{
		return mrb_class_find_path(mrb, cls);
	}
	static RClass* from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		return mrb_class(mrb, val);
	}
};

template<>
struct TypeBinder<RData*> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, RData* data)
	{ 
		mrb_value val = {{ 0 }};
		SET_OBJ_VALUE(val, data);
		return val;
	}
	static RData* from_mrb_value(mrb_state* mrb, mrb_value val) { return RDATA(val); }
};

template<>
struct TypeBinder<RProc*>
{
	static mrb_value to_mrb_value(mrb_state* mrb, RProc* data)
	{
		mrb_value val = {{ 0 }};
		SET_OBJ_VALUE(val, data);
		return val;
	}
	static RProc* from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		return mrb_proc_ptr(val);
	}
};

template<>
struct TypeBinder<mrb_sym>
{
	static mrb_value to_mrb_value(mrb_state* mrb, mrb_sym sym) { return mrb_sym2str(mrb, sym); }
	static mrb_sym from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_intern_str(mrb, val); }
};

/* public data types */

template<>
struct TypeBinder<bool>
{
	static mrb_value to_mrb_value(mrb_state* mrb, bool b) { return mrb_bool_value(b); }
	static bool from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_bool(val); }
};

template<>
struct TypeBinder<int> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, int i) { return mrb_fixnum_value(i); }
	static int from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
};

template<>
struct TypeBinder<int64_t>
{
	static mrb_value to_mrb_value(mrb_state* mrb, int64_t i) { return mrb_fixnum_value(i); }
	static int64_t from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
};

template<>
struct TypeBinder<float>
{
	static mrb_value to_mrb_value(mrb_state* mrb, float f) { return mrb_float_value(mrb, f); }
	static float from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_float(val); }
};

template<>
struct TypeBinder<double>
{
	static mrb_value to_mrb_value(mrb_state* mrb, double f) { return mrb_float_value(mrb, f); }
	static double from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_float(val); }
};

template<>
struct TypeBinder<size_t> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, size_t i) { return mrb_fixnum_value(i); }
	static size_t from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
};

template<>
struct TypeBinder<std::string> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str)
	{
		Arena arena(mrb);
		{
			return mrb_str_new(mrb, str.c_str(), str.size());
		}
	}
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) 
	{ 
		if (mrb_type(val) == MRB_TT_SYMBOL)
		{
			val = mrb_sym2str(mrb, mrb_symbol(val));
		}
		return std::string(RSTRING_PTR(val), RSTRING_LEN(val)); 
	}
};

template<>
struct TypeBinder<const std::string>
{
	static mrb_value to_mrb_value(mrb_state* mrb, const std::string str)
	{
		Arena arena(mrb);
		{
			return mrb_str_new(mrb, str.c_str(), str.size());
		}
	}
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (mrb_type(val) == MRB_TT_SYMBOL)
		{
			val = mrb_sym2str(mrb, mrb_symbol(val));
		}
		return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
	}
};

template<>
struct TypeBinder<const std::string&>
{
	static mrb_value to_mrb_value(mrb_state* mrb, const std::string& str)
	{
		Arena arena(mrb);
		{
			return mrb_str_new(mrb, str.c_str(), str.size());
		}
	}
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (mrb_type(val) == MRB_TT_SYMBOL)
		{
			val = mrb_sym2str(mrb, mrb_symbol(val));
		}
		return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
	}
};

// TODO
// add specializations for
// Array type
// Hash type 
// We might need more than just std::map or std::vector, since objects can be any type.

template<class TClass>
struct TypeBinder< NativeObject<TClass> >
{
	static mrb_value to_mrb_value(mrb_state* mrb, NativeObject<TClass> obj)
	{
		Arena arena(mrb);
		{
			RClass* cls = mrb_class_get(mrb, obj.get_classname().c_str());
			NativeObject<TClass>* objptr = new NativeObject<TClass>(obj);
			RData* data = mrb_data_object_alloc(mrb, cls, objptr, objptr->get_type_ptr());

			return TypeBinder<RData*>::to_mrb_value(mrb, data);
		}
	}

	static NativeObject<TClass> from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (mrb_type(val) == MRB_TT_DATA)
		{
			NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(val);
			return *thisptr;
		}

		throw TypeError("Not a data type", "");
	}
};

template<typename TFunc>
class Function;
template<typename TRet, typename ... TArgs>
class Function<TRet(TArgs...)>;

template<typename TRet, typename ... TArgs>
struct TypeBinder< Function<TRet(TArgs...)> >
{
	static mrb_value to_mrb_value(mrb_state* mrb, Function<TRet(TArgs...)> func)
	{
		throw NotImplementedError("Not implemented", "");
	}

	static Function<TRet(TArgs...)> from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		return Function<TRet(TArgs...)>(mrb, TypeBinder<RProc*>::from_mrb_value(mrb, val));
	}
};

#endif // __MRUBYTYPEBINDER_HPP__
