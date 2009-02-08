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

#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_COMMENT "#"
#define CONFIG_DELIMITER "="
#define CONFIG_WHITESPACE " \t"

#include <map>
#include <string>

class Config {
	public:
		Config();
		~Config();

		std::string getSettingValue(const std::string &setting);

	private:
		std::map<std::string, std::string> settings;
};
#endif