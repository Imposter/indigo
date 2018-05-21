/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_event_hpp_
#define indigo_event_hpp_

// Required libraries
#include <mutex>
#include <vector>
#include <functional>

namespace indigo
{
	template <typename... _TArgs>
	class Event
	{
		std::mutex mEventMutex;
		std::vector<std::function<bool(_TArgs ...)>> mCallbacks;

	public:
		Event() { }
		Event(const Event &event) { }

		template <typename _TFunction>
		Event &Add(const _TFunction &function)
		{
			mEventMutex.lock();
			mCallbacks.push_back(function);
			mEventMutex.unlock();

			return *this;
		}

		template <typename _TFunction>
		Event &Remove(const std::function<_TFunction> &function)
		{
			mEventMutex.lock();
			for (auto it = mCallbacks.begin(); it != mCallbacks.end();)
			{
				if (it->template target<_TFunction>() == function.template target<_TFunction>())
				{
					mCallbacks.erase(it);
					break;
				}
				++it;
			}
			mEventMutex.unlock();

			return *this;
		}

		bool Trigger(_TArgs ... arguments)
		{
			bool success = true;
			std::vector<std::function<bool(_TArgs ...)>> callbacks(mCallbacks.size());

			mEventMutex.lock();
			std::copy(mCallbacks.begin(), mCallbacks.end(), callbacks.begin());
			mEventMutex.unlock();

			for (auto iterator = callbacks.begin(); iterator != callbacks.end(); ++iterator)
				if (!(*iterator)(arguments...))
					success = false;

			return success;
		}

		bool operator()(_TArgs ... arguments)
		{
			return Trigger(arguments...);
		}
	};
}

#endif // indigo_event_hpp_
