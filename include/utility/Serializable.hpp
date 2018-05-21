/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef indigo_serializable_hpp_
#define indigo_serializable_hpp_

#include <ostream>

namespace indigo
{
	class ISerializable
	{
	protected:
		~ISerializable() {}

	public:
		virtual bool Serialize(std::ostream &outBuffer) = 0;
		virtual bool Deserialize(std::istream &inBuffer) = 0;
	};
}

#endif // indigo_serializable_hpp_
