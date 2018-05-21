/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_web_client_hpp_
#define indigo_web_client_hpp_

#include "../Build.hpp"
#include "../Core/Event.hpp"
#include "../Core/String.hpp"
#include <map>
#include <ostream>

#ifndef INDIGO_CURL_DEFS
#define INDIGO_CURL_DEFS
typedef void CURL;
#endif // INDIGO_CURL_DEFS

namespace indigo
{
	class WebRequest;
	class WebClient;

	class INDIGO_API WebResponse
	{
		friend class WebRequest;

		WebRequest *mWebRequest;
		CURL *mCurl;
		int mCurlCode;
		std::ostream *mStream;

		int32_t progressCallback(double downloadTotal, double downloadNow, double uploadTotal, double uploadNow);
		int32_t writeCallback(char *data, int32_t size, int32_t count) const;

		static int32_t CurlProgressCallback(void *obj, double downloadTotal, double downloadCurrent, double uploadTotal, double uploadNow);
		static int32_t CurlWriteCallback(char *data, int32_t size, int32_t count, void *obj);

		void doRequest();

	public:
		WebResponse(WebRequest *webRequest);
		~WebResponse();

		bool HasError() const;
		const std::ostream *GetStream() const;

		void Close();

		Event<> OnStart;
		Event<bool> OnFinish;
		Event<double, double, double> OnProgressChanged;
	};

	class INDIGO_API WebRequest
	{
		friend class WebClient;
		friend class WebResponse;

		WebClient *mWebClient;
		std::string mUrl;
		std::map<std::string, std::string> mHeaders;
		bool mSslVerify;
		WebResponse *mResponse;

	public:
		WebRequest(WebClient *webClient);
		~WebRequest();

		const std::string &GetUrl() const;
		void SetUrl(std::string url);

		const std::map<std::string, std::string> &GetHeaders() const;
		void SetHeader(std::string key, std::string value);

		WebResponse *GetResponse(std::ostream *stream, std::function<void()> onStart = std::function<void()>([]() { }),
		                         std::function<void(bool)> onFinish = std::function<void(bool)>([](bool) { }),
		                         std::function<void(double, double, double)> onProgress = std::function<void(double, double, double)>([](double, double, double) { }));
		WebResponse *GetResponse(std::function<void()> onStart = std::function<void()>([]() { }),
		                         std::function<void(bool)> onFinish = std::function<void(bool)>([](bool) { }),
		                         std::function<void(double, double, double)> onProgress = std::function<void(double, double, double)>([](double, double, double) { }));

		void Close();
	};

	// NOTE: This library is untested and may contain errors or memory leaks. Use with caution!
	class INDIGO_API WebClient
	{
		friend class WebRequest;

		std::string mUserAgent;
		std::vector<WebRequest *> mRequests;
		std::mutex mMutex;

		void closeRequest(const WebRequest *request);

	public:
		WebClient(std::string userAgent = "indigo::WebClient");
		~WebClient();

		const std::string &GetUserAgent() const;
		void SetUserAgent(std::string userAgent);

		WebRequest *CreateRequest(std::string url, std::map<std::string, std::string> headers = {}, bool sslVerify = false);
	};
}

#endif // indigo_web_client_hpp_
