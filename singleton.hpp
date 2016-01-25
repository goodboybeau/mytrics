#ifndef _SINGLETON_HPP_
#define _SINGELTON_HPP_


template <class T>
struct type_created : std::atomic_bool
{
	type_created(bool b)
		: std::atomic_bool(b)
	{}
};

template <class T>
class Singleton
{
	static type_created<T> created;
protected:
	static std::shared_ptr<T> object;

	Singleton() = default;

	virtual ~Singleton() {}

public:

	static std::shared_ptr<T>
	Instance(const std::string& _namespace = "")
	{
		bool expected = false;
		if(Singleton<T>::created.compare_exchange_strong(expected, true))
		{
			object = std::make_shared<T>(_namespace);
		}
		return object;
	}

	static void reset()
	{
		bool expected = true;
		if(Singleton<T>::created.compare_exchange_strong(expected, false))
		{
			object.reset();
		}
	}
};

template<class T>
std::shared_ptr<T> Singleton<T>::object(nullptr);

template <class T>
struct type_created<T> Singleton<T>::created(false);


#endif /* _SINGELTON_HPP_ */
