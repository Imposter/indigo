/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Platform.hpp"
#include "Core/String.hpp"
#include "Utility/File.hpp"

#ifdef HAVE_BOOST
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#if _MSC_VER > 2000
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <filesystem>
namespace fs = std::tr2::sys;
#endif
#endif

namespace indigo
{
#if _MSC_VER < 2000
	fs::path canonical(const fs::path &path)
	{
		char newPath[MAX_PATH]{0};
		PathCanonicalizeA(newPath, path.string().c_str());
		return fs::path(newPath);
	}
#endif

	// Taken from http://stackoverflow.com/questions/1746136/how-do-i-normalize-a-pathname-using-boostfilesystem
	fs::path File::Normalize(const fs::path &path)
	{
#if _MSC_VER > 2000
		fs::path absPath = absolute(path);
#else
		fs::path absPath = complete(path);
#endif
		fs::path::iterator it = absPath.begin();
		fs::path result = *it++;

		// Get canonical version of the existing part
		for (; exists(result / fs::path(*it)) && it != absPath.end(); ++it)
			result /= *it;

		result = canonical(result);

		// For the rest remove ".." and "." in a path with no symlinks
		for (; it != absPath.end(); ++it)
		{
			// Just move back on ../
			if (*it == "..")
				result = result.parent_path();
			else if (*it != ".")
				// Just cat other path entries
				result /= *it;
		}

		// Fix path
#if defined(OS_WIN)
		std::string finalPath = String::Replace(result.string(), "/", "\\");
		finalPath = String::Replace(finalPath, "\\\\", "\\");
#if defined(HAVE_BOOST)
		if (*finalPath.end() == 0) {
			result = fs::path(std::vector<char>(final_path.begin(), final_path.end() - 1));
		}
#endif
#endif

		return result;
	}

	fs::path File::GetRelativePath(const fs::path &path, const fs::path &relativeTo)
	{
		std::string sourcePath = path.string();
		std::string relativePath = relativeTo.string();

		// Fix path
#if defined(OS_WIN)
		relativePath += "\\";

		sourcePath = String::Replace(sourcePath, "/", "\\");
		relativePath = String::Replace(relativePath, "/", "\\");

		sourcePath = String::Replace(sourcePath, "\\\\", "\\");
		relativePath = String::Replace(relativePath, "\\\\", "\\");
#endif

		std::string finalPath = String::Replace(sourcePath, relativePath, "", true);
#if defined(HAVE_BOOST)
	if (*finalPath.end() == 0) {
		return fs::path(std::vector<char>(finalPath.begin(), finalPath.end() - 1));
	} 
#endif
		return finalPath;
	}
}
