/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_file_hpp_
#define indigo_file_hpp_

#include "../Build.hpp"
#ifdef HAVE_BOOST
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#if _MSC_VER > 2000
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::tr2::sys;
#endif
#endif

namespace indigo
{
	class INDIGO_API File
	{
	public:
		static fs::path Normalize(const fs::path &path);
		static fs::path GetRelativePath(const fs::path &path, const fs::path &relativeTo);
	};
}

#endif // indigo_file_hpp_
