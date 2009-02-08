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

#ifndef LOCUTUS_H
#define LOCUTUS_H
/* settings */
#define MUSIC_OUTPUT_KEY "output_directory"
#define MUSIC_OUTPUT_VALUE "/media/music/sorted/"
#define MUSIC_OUTPUT_DESCRIPTION "Output directory"
#define MUSIC_INPUT_KEY "input_directory"
#define MUSIC_INPUT_VALUE "/media/music/unsorted/"
#define MUSIC_INPUT_DESCRIPTION "Input directory"
#define DRY_RUN_KEY "dry_run"
#define DRY_RUN_VALUE true
#define DRY_RUN_DESCRIPTION "Only read files and look them up, don't save and move files. Currently genre won't be looked up during a dry run."
#define LOOKUP_GENRE_KEY "lookup_genre"
#define LOOKUP_GENRE_VALUE true
#define LOOKUP_GENRE_DESCRIPTION "Fetch genre (or tag) from Audioscrobbler before saving a file. If no genre is found then genre is set to an empty string. If this option is set to false, the genre field is left unmodified."

extern "C" {
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
};
#include <list>
#include <map>
#include <string>
#include <vector>

class Audioscrobbler;
class Database;
class FileNamer;
class Matcher;
class Metafile;
class MusicBrainz;
//class PUIDGenerator;

class Locutus {
	public:
		explicit Locutus(Database *database);
		~Locutus();

		static void trim(std::string *text);

		long run();

	private:
		Audioscrobbler *audioscrobbler;
		Database *database;
		FileNamer *filenamer;
		Matcher *matcher;
		//PUIDGenerator *puidgen;
		MusicBrainz *musicbrainz;
		bool dry_run;
		bool lookup_genre;
		std::list<std::string> dir_queue;
		std::list<std::string> file_queue;
		std::map<std::string, std::vector<Metafile *> > grouped_files;
		std::string input_dir;
		std::string output_dir;

		void clearFiles();
		std::string findDuplicateFilename(Metafile *file);
		bool moveFile(Metafile *file, const std::string &filename);
		bool parseDirectory();
		bool parseFile();
		void saveFile(Metafile *file);
		void scanFiles(const std::string &directory);
};
#endif
