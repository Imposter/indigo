/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_manual_reset_hpp_
#define indigo_manual_reset_hpp_

#include <mutex>
#include <condition_variable>

namespace indigo
{
	class ManualReset
	{
		std::mutex mMutex;
		std::condition_variable mConditionVariable;
		bool mSignaled;

	public:
		explicit ManualReset(bool initialState = false)
			: mSignaled(initialState) { }

		void Set()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mSignaled = true;
			mConditionVariable.notify_all();
		}

		void Reset()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mSignaled = false;
		}

		bool Wait(int timeout = 0)
		{
			std::unique_lock<std::mutex> lock(mMutex);
			while (!mSignaled)
			{
				if (timeout == 0)
					mConditionVariable.wait(lock);
				else
					return mConditionVariable.wait_for(lock, std::chrono::milliseconds(timeout)) != std::cv_status::timeout;
			}

			return true;
		}
	};
}

#endif // indigo_manual_reset_hpp_
