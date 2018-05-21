/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_build_h_
#define indigo_build_h_

#ifndef INDIGO_STATIC
#ifdef INDIGO_EXPORT
#define INDIGO_API __declspec(dllexport)
#else
#define INDIGO_API __declspec(dllimport)
#endif
#else
#define INDIGO_API
#endif

#endif // indigo_build_h_