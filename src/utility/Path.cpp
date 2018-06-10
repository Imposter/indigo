/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Platform.hpp"
#include "core/String.hpp"
#include "utility/Path.hpp"

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

	std::string Path::CurrentDirectory()
	{		
#if _MSC_VER < 2000
		return fs::current_path<fs::path>().string();
#else
		return fs::current_path().string();
#endif
	}

	// Taken from http://stackoverflow.com/questions/1746136/how-do-i-normalize-a-pathname-using-boostfilesystem
	std::string Path::Normalize(const std::string &path)
	{
#if _MSC_VER > 2000
		fs::path absPath = absolute(fs::path(path));
#else
		fs::path absPath = complete(fs::path(path));
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
#endif

		return result.string();
	}

	std::string Path::RelativePath(std::string path, std::string relativeTo)
	{
		// Fix path
#if defined(OS_WIN)
		relativeTo += "\\";

		path = String::Replace(path, "/", "\\");
		relativeTo = String::Replace(relativeTo, "/", "\\");

		path = String::Replace(path, "\\\\", "\\");
		relativeTo = String::Replace(relativeTo, "\\\\", "\\");
#endif

		return String::Replace(path, relativeTo, "", true);
	}
}
