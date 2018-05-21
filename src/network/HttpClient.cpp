/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Network/HttpClient.hpp"
#include "Core/String.hpp"
#include "curl/curl.h"

namespace indigo
{
	int32_t HttpClient::progressCallback(double downloadTotal, double downloadNow, double uploadTotal, double uploadNow)
	{
		if (downloadTotal > 0.0)
			OnProgressChanged(mObject, mUrl, mHeaders, downloadNow, downloadTotal, downloadNow / downloadTotal * 100);

		return 0;
	}

	int32_t HttpClient::writeCallback(char *data, int32_t size, int32_t count) const
	{
		if (mObject != nullptr)
		{
			mObject->write(data, size * count);
			mObject->flush();
		}

		return size * count;
	}

	int32_t HttpClient::CurlProgressCallback(void *obj, double downloadTotal, double downloadCurrent, double uploadTotal, double uploadNow)
	{
		return reinterpret_cast<HttpClient *>(obj)->progressCallback(downloadTotal, downloadCurrent, uploadTotal, uploadNow);
	}

	int32_t HttpClient::CurlWriteCallback(char *data, int32_t size, int32_t count, void *obj)
	{
		return static_cast<HttpClient *>(obj)->writeCallback(data, size, count);
	}

	HttpClient::HttpClient(std::string userAgent)
		: mCurl(nullptr), mObject()
	{
		mCurl = curl_easy_init();
		curl_easy_setopt(mCurl, CURLOPT_USERAGENT, userAgent.c_str());
	}

	HttpClient::~HttpClient()
	{
		curl_easy_cleanup(mCurl);
	}

	bool HttpClient::DownloadData(std::ostream *object, std::string url, std::map<std::string, std::string> headers, bool sslVerify)
	{
		mMutex.lock();

		mObject = object;
		mUrl = url;
		mHeaders = headers;

		// Set headers
		curl_slist *chunk = nullptr;
		for (auto it = headers.begin(); it != headers.end(); ++it)
			chunk = curl_slist_append(chunk, String::Format("%s: %s", it->first, it->second).c_str());

		curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, chunk);
		curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, &CurlWriteCallback);
		curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, sslVerify);
		curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, sslVerify);
		curl_easy_setopt(mCurl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_PROGRESSFUNCTION, &CurlProgressCallback);
		curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(mCurl, CURLOPT_FAILONERROR, TRUE);
#ifdef INDIGO_HTTP_CLIENT_VERBOSE
	curl_easy_setopt(mCurl, CURLOPT_VERBOSE, TRUE);
#endif

		// Trigger start event
		OnStart(mObject, mUrl, mHeaders);

		// Complete request
		CURLcode code = curl_easy_perform(mCurl);
		bool success = code == CURLE_OK;

		// Trigger end event
		OnFinish(success, mObject, mUrl, mHeaders);

		// Clear download info
		mObject = nullptr;
		mUrl.clear();
		mHeaders.clear();

		mMutex.unlock();

		return success;
	}
}
