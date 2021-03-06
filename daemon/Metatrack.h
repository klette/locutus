// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef METATRACK_H
#define METATRACK_H

#include <string>

class Metatrack {
public:
	int duration;
	int tracknumber;
	std::string album_mbid;
	std::string album_title;
	std::string artist_mbid;
	std::string artist_name;
	std::string track_mbid;
	std::string track_title;

	Metatrack(int duration = 0, int tracknumber = 0, std::string album_mbid = "", std::string album_title = "", std::string artist_mbid = "", std::string artist_name = "", std::string track_mbid = "", std::string track_title = "");
};
#endif
