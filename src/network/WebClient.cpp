/*
 *  This file is part of the Indigo library.
*
 *  This program is licensed under the GNU General
 *  Public License. To view the full license, check
 *  LICENSE in the project root.
*/

#include "Network/WebClient.hpp"
#include "curl/curl.h"
#include <sstream>

namespace indigo
{
	int32_t WebResponse::progressCallback(double downloadTotal, double downloadNow, double uploadTotal, double uploadNow)
	{
		if (downloadTotal > 0.0)
			OnProgressChanged(downloadNow, downloadTotal, downloadNow / downloadTotal * 100);

		return 0;
	}

	int32_t WebResponse::writeCallback(char *data, int32_t size, int32_t count) const
	{
		if (mStream != nullptr)
		{
			mStream->write(data, size * count);
			mStream->flush();
		}

		return size * count;
	}

	int32_t WebResponse::CurlProgressCallback(void *obj, double downloadTotal, double downloadCurrent, double uploadTotal, double uploadNow)
	{
		return reinterpret_cast<WebResponse *>(obj)->progressCallback(downloadTotal, downloadCurrent, uploadTotal, uploadNow);
	}
	
	int32_t WebResponse::CurlWriteCallback(char *data, int32_t size, int32_t count, void *obj)
	{
		return static_cast<WebResponse *>(obj)->writeCallback(data, size, count);
	}

	void WebResponse::doRequest()
	{
		mCurl = curl_easy_init();
		curl_easy_setopt(mCurl, CURLOPT_USERAGENT, mWebRequest->mWebClient->GetUserAgent().c_str());

		// Set headers
		curl_slist *chunk = nullptr;
		for (auto it = mWebRequest->mHeaders.begin(); it != mWebRequest->mHeaders.end(); ++it)
			chunk = curl_slist_append(chunk, String::Format("%s: %s", it->first, it->second).c_str());

		curl_easy_setopt(mCurl, CURLOPT_URL, mWebRequest->mUrl.c_str());
		curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, chunk);
		curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, &CurlWriteCallback);
		curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYHOST, mWebRequest->mSslVerify);
		curl_easy_setopt(mCurl, CURLOPT_SSL_VERIFYPEER, mWebRequest->mSslVerify);
		curl_easy_setopt(mCurl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(mCurl, CURLOPT_PROGRESSFUNCTION, &CurlProgressCallback);
		curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(mCurl, CURLOPT_FAILONERROR, TRUE);
#ifdef INDIGO_WEB_CLIENT_VERBOSE
	curl_easy_setopt(mCurl, CURLOPT_VERBOSE, TRUE);
#endif

		// Check if stream exists, and create it
		if (mStream == nullptr)
			mStream = new std::stringstream();

		mCurlCode = curl_easy_perform(mCurl);
	}

	WebResponse::WebResponse(WebRequest *webRequest)
		: mWebRequest(webRequest), mCurl(nullptr), mCurlCode(CURLE_OK), mStream(nullptr) {}

	WebResponse::~WebResponse()
	{
		Close();
	}

	bool WebResponse::HasError() const
	{
		return mCurlCode != CURLE_OK;
	}

	const std::ostream *WebResponse::GetStream() const
	{
		return mStream;
	}

	void WebResponse::Close()
	{
		if (mCurl != nullptr)
			curl_easy_cleanup(mCurl);

		if (mStream != nullptr)
		{
			delete mStream;
			mStream = nullptr;
		}
	}

	WebRequest::WebRequest(WebClient *webClient)
		: mWebClient(webClient), mSslVerify(false), mResponse(nullptr) {}

	WebRequest::~WebRequest()
	{
		Close();
	}

	const std::string &WebRequest::GetUrl() const
	{
		return mUrl;
	}

	void WebRequest::SetUrl(std::string url)
	{
		mUrl = url;
	}

	const std::map<std::string, std::string> &WebRequest::GetHeaders() const
	{
		return mHeaders;
	}

	void WebRequest::SetHeader(std::string key, std::string value)
	{
		mHeaders[key] = value;
	}

	WebResponse *WebRequest::GetResponse(std::ostream *stream, std::function<void()> onStart, std::function<void(bool)> onFinish, std::function<void(double, double, double)> onProgress)
	{
		Close();

		mResponse = new WebResponse(this);
		mResponse->mStream = stream;
		mResponse->OnStart.Add([&]()
		{
			onStart();
			return true;
		});
		mResponse->OnFinish.Add([&](bool success)
		{
			onFinish(success);
			return true;
		});
		mResponse->OnProgressChanged.Add([&](double total, double now, double percent)
		{
			onProgress(total, now, percent);
			return true;
		});
		mResponse->doRequest();

		return mResponse;
	}

	WebResponse *WebRequest::GetResponse(std::function<void()> onStart, std::function<void(bool)> onFinish, std::function<void(double, double, double)> onProgress)
	{
		Close();

		mResponse = new WebResponse(this);
		mResponse->OnStart.Add([&]()
		{
			onStart();
			return true;
		});
		mResponse->OnFinish.Add([&](bool success)
		{
			onFinish(success);
			return true;
		});
		mResponse->OnProgressChanged.Add([&](double total, double now, double percent)
		{
			onProgress(total, now, percent);
			return true;
		});
		mResponse->doRequest();

		return mResponse;
	}

	void WebRequest::Close()
	{
		if (mResponse != nullptr)
		{
			delete mResponse;
			mResponse = nullptr;
		}

		mWebClient->closeRequest(this);
	}

	void WebClient::closeRequest(const WebRequest *request)
	{
		mMutex.lock();
		for (auto it = mRequests.begin(); it != mRequests.end();)
		{
			if (*it == request)
				it = mRequests.erase(it);
			else
				++it;
		}
		mMutex.unlock();
		delete request;
	}

	WebClient::WebClient(std::string userAgent)
		: mUserAgent(userAgent) {}

	WebClient::~WebClient()
	{
		for (auto request : mRequests)
			delete request;

		mRequests.clear();
	}

	const std::string &WebClient::GetUserAgent() const
	{
		return mUserAgent;
	}

	void WebClient::SetUserAgent(std::string user_agent)
	{
		mUserAgent = user_agent;
	}

	WebRequest *WebClient::CreateRequest(std::string url, std::map<std::string, std::string> headers, bool sslVerify)
	{
		WebRequest *web_request = new WebRequest(this);
		web_request->mUrl = url;
		web_request->mHeaders = headers;
		web_request->mSslVerify = sslVerify;

		mMutex.lock();
		mRequests.push_back(web_request);
		mMutex.unlock();

		return web_request;
	}
}
