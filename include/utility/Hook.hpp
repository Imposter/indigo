/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_hook_hpp_
#define indigo_hook_hpp_

#include "../Build.hpp"

namespace indigo
{
class INDIGO_API HookBase
{
public:
	virtual ~HookBase() = default;

	virtual bool Install() = 0;
	virtual bool Remove() = 0;

	/**
	* \brief Static utility function to install a call hook, of which external use has been deprecated in favour of Hook::Create<CallHook>(...)
	* \param target Target address (being hooked)
	* \param function Pointer to replacement function (replacing target)
	* \param original Pointer to original function (saved for future usage)
	* \return Returns true if hooking was successful
	*/
	static bool Install(void *target, void *function, void **original = nullptr);

	/**
	* \brief Static utility function to remove a call hook, of which external use has been deprecated in favour of CallHook::Remove()
	* \param original The original function pointing to the original code
	* \return Returns true if removing the hook was successful
	*/
	static bool Remove(void **original);

	/**
	* \brief Static utility function to install an export call hook, of which external use has been deprecated in favour of Hook::Create<EATHook>(...)
	* \param moduleName The target module name
	* \param exportName The target export, ordinal (int) or named (const char *)
	* \param function Pointer to replacement function (replacing target)
	* \param original Pointer to original function (saved for future usage)
	* \return Returns true if hooking was successful
	*/
	static bool Install(const char *moduleName, const char *exportName, void *function, void **original = nullptr);

	/**
	 * \brief Static utility function to install an import hook, of which external use has been deprecated in favour of Hook::Create<IATHook>(...)
	 * \param module The instance of which the hook is intended for
	 * \param moduleName The target module name 
	 * \param importName The target import, ordinal (int) or named (const char *)
	 * \param ordinal Flag to determine if the target import is an ordinal or name
	 * \param function Pointer to replacement function (replacing target)
	 * \param original Pointer to original function (saved for future usage)
	 * \return Returns true if hooking was successful
	 */
	static bool Install(void *module, const char *moduleName, const char *importName, bool ordinal, void *function,
	                    void **original = nullptr);
};

class Hook : public HookBase
{
protected:
	void *mRedirect;
	void *mOriginal;

public:
	Hook()
		: mRedirect(nullptr), mOriginal(nullptr) {}

	template <typename _TType>
	_TType GetOriginal()
	{
		return static_cast<_TType>(mOriginal);
	}

	template <typename _TType>
	_TType GetRedirect()
	{
		return static_cast<_TType>(mRedirect);
	}
};

class IATHook : public Hook
{
	void *mModule;
	const char *mModuleName;
	const char *mImportName;
	bool mOrdinal;
	void *mTarget;
	bool mInstalled;

public:
	IATHook()
		: mModule(nullptr), mModuleName(nullptr), mImportName(nullptr), mOrdinal(false), mTarget(nullptr),
		  mInstalled(false) { }

	~IATHook()
	{
		IATHook::Remove();
	}

	bool Install() override
	{
		if (mInstalled || mModuleName == nullptr)
			return false;

		if (!Hook::Install(mModule, mModuleName, mImportName, mOrdinal, mRedirect, &mOriginal))
			return false;

		mInstalled = true;

		return true;
	}

	bool Install(void *sourceModule, const char *module, const char *importName, void *redirect)
	{
		if (mInstalled)
			return false;

		mModule = sourceModule;
		mModuleName = module;
		mImportName = importName;
		mRedirect = redirect;

		return Install();
	}

	bool Install(void *sourceModule, const char *module, int importOrdinal, void *redirect)
	{
		if (mInstalled)
			return false;

		mModule = sourceModule;
		mModuleName = module;
		mImportName = reinterpret_cast<const char *>(importOrdinal);
		mOrdinal = true;
		mRedirect = redirect;

		return Install();
	}

	bool Remove() override
	{
		if (!mInstalled)
			return false;

		if (!Hook::Install(mModule, mModuleName, mImportName, mOrdinal, mOriginal, nullptr))
			return false;

		mInstalled = false;

		return true;
	}
};

class EATHook : public Hook
{
	const char *mModuleName;
	const char *mExportName;
	bool mInstalled;

public:
	EATHook()
		: mModuleName(nullptr), mExportName(nullptr), mInstalled(false) { }

	~EATHook()
	{
		EATHook::Remove();
	}

	bool Install() override
	{
		if (mInstalled || mModuleName == nullptr)
			return false;

		if (!Hook::Install(mModuleName, mExportName, mRedirect, &mOriginal))
			return false;

		mInstalled = true;

		return true;
	}

	bool Install(const char *module, const char *exportName, void *redirect, void *original = nullptr)
	{
		if (mInstalled)
			return false;

		mModuleName = module;
		mExportName = exportName;
		mRedirect = redirect;
		mOriginal = original;

		return Install();
	}

	bool Install(const char *module, const int exportOrdinal, void *redirect, void *original = nullptr)
	{
		if (mInstalled)
			return false;

		mModuleName = module;
		mExportName = reinterpret_cast<const char *>(exportOrdinal);
		mRedirect = redirect;
		mOriginal = original;

		return Install();
	}

	bool Remove() override
	{
		if (!mInstalled)
			return false;

		if (!Hook::Remove(&mOriginal))
			return false;

		mInstalled = false;

		return true;
	}
};

class CallHook : public Hook
{
	void *mTarget;
	bool mInstalled;

public:
	CallHook()
		: mTarget(nullptr), mInstalled(false) { }

	~CallHook()
	{
		CallHook::Remove();
	}

	bool Install() override
	{
		if (mInstalled || mTarget == nullptr)
			return false;

		if (!Hook::Install(mTarget, mRedirect, &mOriginal))
			return false;

		mInstalled = true;

		return true;
	}

	bool Install(void *target, void *redirect)
	{
		if (mInstalled)
			return false;

		mTarget = target;
		mRedirect = redirect;

		return Install();
	}

	bool Remove() override
	{
		if (!mInstalled)
			return false;

		if (!Hook::Remove(&mOriginal))
			return false;

		mInstalled = false;

		return true;
	}
};
}

#endif // indigo_hook_hpp_
