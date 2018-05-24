/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_http_client_hpp_
#define indigo_http_client_hpp_

#ifndef INDIGO_CURL_DEFS
#define INDIGO_CURL_DEFS
typedef void CURL;
#endif // INDIGO_CURL_DEFS

#include "../Build.hpp"
#include "../Core/Event.hpp"
#include <map>
#include <ostream>

namespace indigo
{
	class INDIGO_API HttpClient
	{
		CURL *mCurl;
		std::mutex mMutex;

		std::ostream *mObject;
		std::string mUrl;
		std::map<std::string, std::string> mHeaders;

		int32_t progressCallback(double downloadTotal, double downloadNow, double uploadTotal, double uploadNow);
		int32_t writeCallback(char *data, int32_t size, int32_t count) const;

		static int32_t CurlProgressCallback(void *obj, double downloadTotal, double downloadCurrent, double uploadTotal, double uploadNow);
		static int32_t CurlWriteCallback(char *data, int32_t size, int32_t count, void *obj);

	public:
		HttpClient(const std::string &userAgent = "indigo::HttpClient");
		~HttpClient();

		bool DownloadData(std::ostream *object, const std::string &url, std::map<std::string, std::string> headers = {}, bool sslVerify = false);

		Event<std::ostream *, std::string, std::map<std::string, std::string>> OnStart;
		Event<bool, std::ostream *, std::string, std::map<std::string, std::string>> OnFinish;
		Event<std::ostream *, std::string, std::map<std::string, std::string>, double, double, double> OnProgressChanged;
	};
}

#endif // indigo_http_client_hpp_
