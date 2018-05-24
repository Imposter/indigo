/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Utility/CredentialsBox.hpp"
#include "Platform.hpp"
#if !defined(OS_WIN)
#error "Unsupported platform!"
#endif
#include "core/string.hpp"

#include <Windows.h>
#include <WinCred.h>
#include <ole2.h>
#include <utility>

namespace indigo
{
	bool CredentialsBox::Show(std::string &username, std::string &password, bool &save, std::string title, std::string format, ...)
	{
		char tempString[INDIGO_CORE_STRING_BUFFERSIZE];

		va_list argumentList;
		va_start(argumentList, format);
		vsnprintf_s(tempString, sizeof tempString, format.c_str(), argumentList);
		va_end(argumentList);

		BOOL credentials_save = FALSE;

		std::wstring wUsername;
		std::wstring wPassword;
		std::wstring wTitle = String::ToWideString(std::move(title));
		std::wstring wMessage = String::ToWideString(tempString);

		CREDUI_INFOW creduiInfo;
		creduiInfo.cbSize = sizeof creduiInfo;
		creduiInfo.pszCaptionText = wTitle.c_str();
		creduiInfo.pszMessageText = wMessage.c_str();
		creduiInfo.hbmBanner = nullptr;

		wUsername.resize(CREDUI_MAX_USERNAME_LENGTH);
		wPassword.resize(CREDUI_MAX_PASSWORD_LENGTH);

		PVOID authBlob = nullptr;
		ULONG authBlobSize = 0;
		ULONG authPackage = 0;
		DWORD usernameLength = wUsername.size();
		DWORD passwordLength = wPassword.size();
		wchar_t domain[CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1];
		DWORD domainMaxLength = sizeof domain;

		DWORD creduiError = CredUIPromptForWindowsCredentialsW(&creduiInfo,
		                                                        0,
		                                                        &authPackage,
		                                                        nullptr,
		                                                        0,
		                                                        &authBlob,
		                                                        &authBlobSize,
		                                                        &credentials_save,
		                                                        CREDUIWIN_GENERIC | CREDUIWIN_CHECKBOX);

		if (creduiError == NO_ERROR)
		{
			CredUnPackAuthenticationBufferW(0,
			                                authBlob,
			                                authBlobSize,
			                                const_cast<wchar_t*>(wUsername.c_str()),
			                                &usernameLength,
			                                domain,
			                                &domainMaxLength,
			                                const_cast<wchar_t*>(wPassword.c_str()),
			                                &passwordLength);
			SecureZeroMemory(authBlob, authBlobSize);
			CoTaskMemFree(authBlob);
			authBlob = nullptr;

			wUsername.resize(usernameLength - 1);
			wPassword.resize(passwordLength - 1);

			username = String::ToString(wUsername);
			password = String::ToString(wPassword);

			SecureZeroMemory(const_cast<wchar_t*>(wUsername.c_str()), wUsername.size());
			SecureZeroMemory(const_cast<wchar_t*>(wPassword.c_str()), wPassword.size());
		}

		save = credentials_save == TRUE;

		return creduiError == NO_ERROR;
	}
}
