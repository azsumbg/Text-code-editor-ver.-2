#pragma once

#ifdef EDITSRV_EXPORTS
#define EDITSRV_API __declspec(dllexport)
#else
#define EDITSRV_API __declspec(dllimport)
#endif

#include <iterator>
#include <d2d1.h>
#include <random>
#include <fstream>

#pragma comment(lib, "d2d1.lib")

constexpr D2D1_RECT_F FULL_SCREEN{ 0, 0, 600.0f, 400.0f };
constexpr D2D1_RECT_F EDITOR_SCREEN{ 0, 50.0f, 600.0f, 350.0f };

constexpr float scr_width{ 600.0f };
constexpr float scr_height{ 400.0f };

constexpr int BAD_PTR{ 5001 };
constexpr int BAD_PARAM{ 5002 };
constexpr int BAD_INDEX{ 5003 };
constexpr int BAD_ITERATOR{ 5004 };
constexpr int BAD_MEMORY{ 5005 };
constexpr int BAD_ERR{ 5006 };

namespace edit
{
	class EDITSRV_API EXCEPTION
	{
	private:
		int what{ BAD_ERR };

	public:
		EXCEPTION(int what_happened);

		const wchar_t* eGet();
	};

	template<typename T> class BAG
	{
	private:
		T* mPtr{ nullptr };
		size_t max_lenght{ 1 };
		size_t next_pos{ 0 };

	public:
		BAG()
		{
			mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));

			if (!mPtr)throw EXCEPTION(BAD_MEMORY);
		}
		BAG(size_t lenght)
		{
			max_lenght = lenght;
			mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));

			if (!mPtr)throw EXCEPTION(BAD_MEMORY);
		}
		BAG(const BAG& other)
		{
			if (!other.mPtr)throw EXCEPTION(BAD_ERR);
			else
			{
				if (other.next_pos == 0)
				{
					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));
					if (!mPtr)throw EXCEPTION(BAD_MEMORY);
				}
				else
				{
					max_lenght = other.max_lenght;
					next_pos = other.next_pos;

					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));
					if (!mPtr)throw EXCEPTION(BAD_MEMORY);

					for (size_t i = 0; i < next_pos; ++i)mPtr[i] = other.mPtr[i];
				}
			}
		}
		BAG(const BAG&& other)
		{
			if (mPtr == other.mPtr)throw EXCEPTION(BAD_PARAM);

			if (!other.mPtr)throw EXCEPTION(BAD_ERR);
			else
			{
				if (other.next_pos == 0)
				{
					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));

					if (!mPtr)throw EXCEPTION(BAD_MEMORY);
				}
				else
				{
					max_lenght = other.max_lenght;
					next_pos = other.next_pos;

					mPtr = other.mPtr;
					other.mPtr = nullptr;
				}
			}
		}

		~BAG()
		{
			free(mPtr);
		}

		BAG& operator = (const BAG& other)
		{
			if (mPtr == other.mPtr)throw EXCEPTION(BAD_PARAM);

			free(mPtr);
			mPtr = nullptr;

			if (!other.mPtr)throw EXCEPTION(BAD_ERR);
			else
			{
				if (other.next_pos == 0)
				{
					max_lenght = 1;
					next_pos = 0;

					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));

					if (!mPtr)throw EXCEPTION(BAD_MEMORY);
				}
				else
				{
					max_lenght = other.max_lenght;
					next_pos = other.next_pos;

					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));
					if (!mPtr)throw EXCEPTION(BAD_MEMORY);

					for (size_t i = 0; i < next_pos; ++i)mPtr[i] = other.mPtr[i];
				}
			}
			
			return (*this);
		}
		BAG& operator = (const BAG&& other)
		{
			if (mPtr == other.mPtr)throw EXCEPTION(BAD_PARAM);

			free(mPtr);
			mPtr = nullptr;

			if (!other.mPtr)throw EXCEPTION(BAD_ERR);
			else
			{
				if (other.next_pos == 0)
				{
					max_lenght = 1;
					next_pos = 0;

					mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));
					if (!mPtr)throw EXCEPTION(BAD_MEMORY);
				}
				else
				{
					max_lenght = other.max_lenght;
					next_pos = other.next_pos;

					mPtr = other.mPtr;

					other.mPtr = nullptr;
				}
			}

			return (*this);
		}

		T& operator[](size_t index)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else if (index >= next_pos)throw EXCEPTION(BAD_INDEX);

			return mPtr[index];
		}

		void clear()
		{
			free(mPtr);
			mPtr = nullptr;

			max_lenght = 1;
			next_pos = 0;
			mPtr = reinterpret_cast<T*>(calloc(max_lenght, sizeof(T)));

			if (!mPtr)throw EXCEPTION(BAD_MEMORY);
		}
		bool empty()const
		{
			return (next_pos == 0);
		}
		size_t size()const
		{
			return next_pos;
		}
		size_t capacity()const
		{
			return max_lenght;
		}

		void push_front(T element)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else
			{
				if (next_pos + 1 <= max_lenght)
				{
					for (size_t i = next_pos; i > 0; --i)mPtr[i] = mPtr[i - 1];

					*mPtr = element;

					++next_pos;
				}
				else
				{
					++max_lenght;

					T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };
					if (!temp)throw EXCEPTION(BAD_MEMORY);
					else
					{
						mPtr = temp;
						temp = nullptr;

						for (size_t i = next_pos; i > 0; --i)mPtr[i] = mPtr[i - 1];

						*mPtr = element;

						++next_pos;
					}
				}
			}
		}
		void push_front(T* element)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else
			{
				if (next_pos + 1 <= max_lenght)
				{
					for (size_t i = next_pos; i > 0; --i)mPtr[i] = mPtr[i - 1];

					*mPtr = *element;

					++next_pos;
				}
				else
				{
					++max_lenght;

					T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };
					if (!temp)throw EXCEPTION(BAD_MEMORY);
					else
					{
						mPtr = temp;
						temp = nullptr;

						for (size_t i = next_pos; i > 0; --i)mPtr[i] = mPtr[i - 1];

						*mPtr = *element;

						++next_pos;
					}
				}
			}
		}

		void push_back(T element)
		{
			if (!mPtr)throw EXCEPTION(BAD_MEMORY);
			else
			{
				if (next_pos + 1 <= max_lenght)
				{
					mPtr[next_pos] = element;

					++next_pos;
				}
				else
				{
					++max_lenght;

					T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };
					if (!temp)throw EXCEPTION(BAD_MEMORY);
					else
					{
						mPtr = temp;
						temp = nullptr;

						mPtr[next_pos] = element;
						
						++next_pos;
					}
				}
			}
		}
		void push_back(T* element)
		{
			if (!mPtr)throw EXCEPTION(BAD_MEMORY);
			else
			{
				if (next_pos + 1 <= max_lenght)
				{
					mPtr[next_pos] = *element;

					++next_pos;
				}
				else
				{
					++max_lenght;

					T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };
					if (!temp)throw EXCEPTION(BAD_MEMORY);
					else
					{
						mPtr = temp;
						temp = nullptr;

						mPtr[next_pos] = *element;

						++next_pos;
					}
				}
			}
		}

		void insert(T element, size_t index)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else
			{
				if (index >= next_pos)push_back(element);
				else 
				{
					if (next_pos + 1 <= max_lenght)
					{
						for (size_t i = next_pos; i > index; --i)mPtr[i] = mPtr[i - 1];

						mPtr[index] = element;

						++next_pos;
					}
					else
					{
						++max_lenght;

						T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };
						
						if (!temp)throw EXCEPTION(BAD_PTR);
						else
						{
							mPtr = temp;
							temp = nullptr;
						
							for (size_t i = next_pos; i > index; --i)mPtr[i] = mPtr[i - 1];

							mPtr[index] = element;

							++next_pos;
						}
					}
				}
			}
		}
		void insert(T* element, size_t index)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else
			{
				if (index >= next_pos)push_back(*element);
				else
				{
					if (next_pos + 1 <= max_lenght)
					{
						for (size_t i = next_pos; i > index; --i)mPtr[i] = mPtr[i - 1];

						mPtr[index] = *element;

						++next_pos;
					}
					else
					{
						++max_lenght;

						T* temp{ reinterpret_cast<T*>(realloc(mPtr,max_lenght * sizeof(T))) };

						if (!temp)throw EXCEPTION(BAD_PTR);
						else
						{
							mPtr = temp;
							temp = nullptr;

							for (size_t i = next_pos; i > index; --i)mPtr[i] = mPtr[i - 1];

							mPtr[index] = *element;

							++next_pos;
						}
					}
				}
			}
		}

		void erase(size_t index)
		{
			if (!mPtr)throw EXCEPTION(BAD_PTR);
			else
			{
				if (index >= next_pos || next_pos == 0)throw EXCEPTION(BAD_PARAM);

				for (size_t i = index; i < next_pos - 1; --i)mPtr[i] = mPtr[i + 1];

				--next_pos;
			}
		}

		class iterator
		{
		private:
			T* it_ptr{ nullptr };
			T* container_base_ptr{ nullptr };
			size_t container_size{ 0 };

		public:
			friend class BAG<T>;

			using iterator_category = std::contiguous_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;

			iterator(T* init_ptr, BAG<T>& current_container)
			{
				it_ptr = init_ptr;
				container_base_ptr = current_container.mPtr;
				container_size = current_container.next_pos;
			}

			pointer operator ->()
			{
				return it_ptr;
			}
			reference operator *()
			{
				return *it_ptr;
			}

			iterator& operator++ ()
			{
				if (it_ptr >= container_base_ptr + container_size)throw EXCEPTION(BAD_ITERATOR);

				++it_ptr;

				return (*this);
			}
			iterator operator++ (int)
			{
				iterator temp{ (*this) };

				++(*this);

				return temp;
			}

			iterator& operator-- ()
			{
				if (it_ptr < container_base_ptr)throw EXCEPTION(BAD_ITERATOR);

				--it_ptr;

				return (*this);
			}
			iterator operator-- (int)
			{
				iterator temp{ (*this) };

				--(*this);

				return temp;
			}

			iterator& operator+ (int step)
			{
				if (it_ptr >= container_base_ptr + container_size)throw EXCEPTION(BAD_ITERATOR);

				it_ptr += step;

				return (*this);
			}
			iterator& operator- (int step)
			{
				if (it_ptr < container_base_ptr)throw EXCEPTION(BAD_ITERATOR);

				it_ptr -= step;

				return (*this);
			}

			friend bool operator == (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr == other.it_ptr);
			}
			friend bool operator != (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr != other.it_ptr);
			}
			friend bool operator < (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr < other.it_ptr);
			}
			friend bool operator > (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr > other.it_ptr);
			}
			friend bool operator <= (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr <= other.it_ptr);
			}
			friend bool operator >= (const iterator& current, const iterator& other) 
			{
				return(current.it_ptr >= other.it_ptr);
			}
		};

		iterator begin()
		{
			return iterator(mPtr, (*this));
		}
		iterator end()
		{
			return(mPtr + next_pos, (*this));
		}

		void erase(iterator which)
		{
			for (size_t i = 0; i < next_pos; ++i)
			{
				if (mPtr + i == which.it_ptr)
				{
					erase(i);
					return;
				}
			}

			throw EXCEPTION(BAD_ITERATOR);
		}
	};

	class EDITSRV_API RANDIT
	{
	private:
		std::mt19937* twister{ nullptr };

	public:
		RANDIT();

		~RANDIT();

		int operator()(int min, int max);
		float operator()(float min, float max);
	};

	// FUNCTIONS **************************

	int EDITSRV_API CodeChar(wchar_t symbol, int code);
	wchar_t EDITSRV_API DecodeChar(int symbol, int code);

	bool EDITSRV_API FileExist(const wchar_t* name);

	size_t EDITSRV_API SaveFile(const wchar_t* name, BAG<wchar_t>& Bag, int code, bool append = true);
	bool EDITSRV_API LoadFile(const wchar_t* name, BAG<wchar_t>& Bag);
}