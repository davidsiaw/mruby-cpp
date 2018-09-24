#ifndef __MRUBYNATIVEARRAY_HPP__
#define __MRUBYNATIVEARRAY_HPP__

class NativeArray;

class NativeArrayInstance
{
    friend class NativeArray;
public:
	NativeArrayInstance()
		: mrb(), ary(mrb_nil_value())
	{ }

	NativeArrayInstance(std::shared_ptr<mrb_state> mrb, mrb_value ary)
		: mrb(mrb), ary(ary)
	{ }

	~NativeArrayInstance()
	{ }

	size_t length() const
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		return RARRAY_LEN(ary);
	}

	size_t size() const
	{
		return length();
	}

	template<typename T>
	T get(size_t i)
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_ary_ref(mrb.get(), ary, i));
	}

	template<typename T>
	void set(size_t i, T val)
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		mrb_ary_set(mrb.get(), ary, i, TypeBinder<T>::to_mrb_value(mrb.get(), val));
	}

	template<typename T>
	T shift()
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_ary_shift(mrb.get(), ary));
	}

	template<typename T>
	void unshift(T val)
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		mrb_ary_unshift(mrb.get(), ary, TypeBinder<T>::to_mrb_value(mrb.get(), val));
	}

	template<typename T>
	void push(T val)
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		mrb_ary_push(mrb.get(), ary, TypeBinder<T>::to_mrb_value(mrb.get(), val));
	}

	template<typename T>
	T pop()
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_ary_pop(mrb.get(), ary));
	}

	void clear()
	{
		if (!mrb)
		{
			throw RuntimeError("Manipulating unbound array");
		}
		mrb_ary_clear(mrb.get(), ary);
	}

	void create(std::shared_ptr<mrb_state> mrb)
	{
		this->mrb = mrb;
		ary = mrb_ary_new(mrb.get());
	}

private:
	std::shared_ptr<mrb_state> mrb;
	mrb_value ary;
};

class NativeArray
{
	friend class TypeBinder<NativeArray>;
public:
	NativeArray()
		: inst(new NativeArrayInstance())
	{ }

	NativeArray(std::shared_ptr<mrb_state> mrb, mrb_value ary)
		: inst(new NativeArrayInstance(mrb, ary))
	{ }

	~NativeArray()
	{ }

	NativeArrayInstance* operator->()
	{
		return inst.operator->();
	}

private:
	void create(mrb_state* mrb)
	{
		create(std::shared_ptr<mrb_state>(mrb));
	}

	void create(std::shared_ptr<mrb_state> mrb)
	{
		inst->create(mrb);
	}

	bool instantiated() const
	{
		return bool(inst->mrb);
	}

    mrb_value instance() const
    {
        return inst->ary;
    }

	std::shared_ptr<NativeArrayInstance> inst;
};

#endif // __MRUBYNATIVEARRAY_HPP__
