#ifndef __MRUBYTYPEBINDER_HPP__
#define __MRUBYTYPEBINDER_HPP__

template<typename T>
struct TypeBinder 
{
};

template<>
struct TypeBinder<int> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, int i) { return mrb_fixnum_value(i); }
	static int from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
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
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str) { return mrb_str_new(mrb, str.c_str(), str.size()); }
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) 
	{ 
		if (val.tt == MRB_TT_SYMBOL)
		{
			val = mrb_sym2str(mrb, val.value.i);
		}
		return std::string(RSTRING_PTR(val), RSTRING_LEN(val)); 
	}
};

template<class TClass>
struct TypeBinder< NativeObject<TClass> >
{
	static mrb_value to_mrb_value(mrb_state* mrb, NativeObject<TClass> obj)
	{
		mrb_value v;
		NativeObject<TClass>* objptr = new NativeObject<TClass>(obj);
		v.value.p = objptr;
		v.tt = MRB_TT_DATA;
		mrb_gc_protect(mrb, v);
		return v;
	}

	static NativeObject<TClass> from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (val.tt == MRB_TT_DATA)
		{
			NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(val);
			return *thisptr;
		}

		throw Exception("Not a data type", "");
	}
};

/* internally used datatypes */

template<>
struct TypeBinder<RClass*> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, RClass* cls) { return mrb_class_find_path(mrb, cls); }
	static RClass* from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_class(mrb, val); }
};

template<>
struct TypeBinder<RData*> 
{
	static mrb_value to_mrb_value(mrb_state* mrb, RData* data) { 
		mrb_value val = { 0 };
		val.tt = data->tt;
		val.value.p = data;
		return val;
	}
	static RData* from_mrb_value(mrb_state* mrb, mrb_value val) { return RDATA(val); }
};

template<>
struct TypeBinder<mrb_sym>
{
	static mrb_value to_mrb_value(mrb_state* mrb, mrb_sym sym) { return mrb_sym2str(mrb, sym); }
	static mrb_sym from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_intern_str(mrb, val); }
};

#endif // __MRUBYTYPEBINDER_HPP__
