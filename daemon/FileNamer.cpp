#include "Database.h"
#include "Debug.h"
#include "FileNamer.h"
#include "Metafile.h"

using namespace std;

/* constructors/destructor */
FileNamer::FileNamer(Database *database) : database(database) {
	format_mapping["%album%"] = TYPE_ALBUM;
	format_mapping["%albumartist%"] = TYPE_ALBUMARTIST;
	format_mapping["%albumartistsort%"] = TYPE_ALBUMARTISTSORT;
	format_mapping["%artist%"] = TYPE_ARTIST;
	format_mapping["%artistsort%"] = TYPE_ARTISTSORT;
	format_mapping["%musicbrainz_albumartistid%"] = TYPE_MUSICBRAINZ_ALBUMARTISTID;
	format_mapping["%musicbrainz_albumid%"] = TYPE_MUSICBRAINZ_ALBUMID;
	format_mapping["%musicbrainz_artistid%"] = TYPE_MUSICBRAINZ_ARTISTID;
	format_mapping["%musicbrainz_trackid%"] = TYPE_MUSICBRAINZ_TRACKID;
	format_mapping["%musicip_puid%"] = TYPE_MUSICIP_PUID;
	format_mapping["%title%"] = TYPE_TITLE;
	format_mapping["%tracknumber%"] = TYPE_TRACKNUMBER;
	format_mapping["%date%"] = TYPE_DATE;
	format_mapping["%custom_artist%"] = TYPE_CUSTOM_ARTIST;
	format_mapping["%genre%"] = TYPE_GENRE;

	file_format = database->loadSettingString(FILENAME_FORMAT_KEY, FILENAME_FORMAT_VALUE, FILENAME_FORMAT_DESCRIPTION);
	illegal_characters = database->loadSettingString(FILENAME_ILLEGAL_CHARACTERS_KEY, FILENAME_ILLEGAL_CHARACTERS_VALUE, FILENAME_ILLEGAL_CHARACTERS_DESCRIPTION);
	string::size_type pos = illegal_characters.find('_', 0);
	if (pos != string::npos)
		illegal_characters.erase(pos, 1); // some idiot added "_" to illegal_characters
}

FileNamer::~FileNamer() {
}

/* methods */
const string &FileNamer::getFilename(Metafile *file) {
	if (file_format.size() <= 0) {
		filename.clear();
		Debug::warning() << "File format for output is way too short, refuse to save file" << endl;
		return filename;
	}
	string::size_type start = 0;
	filename = file_format;
	string::size_type stop = file->filename.find_last_of('.');
	if (stop != string::npos)
		filename.push_back('.'); // we need the "." before the extension (if any)
	while (stop != string::npos && ++stop < file->filename.size()) {
		if (file->filename[stop] >= 'A' && file->filename[stop] <= 'Z')
			filename.push_back(file->filename[stop] + 32);
		else
			filename.push_back(file->filename[stop]);
	}

	while (start < filename.size() && (start = filename.find('%', start)) != string::npos) {
		string::size_type stop = filename.find('%', start + 1);
		if (stop == string::npos)
			break; // no more '%'
		map<string, int>::iterator replace = format_mapping.find(filename.substr(start, stop - start + 1)); // "+ 1" to include last '%'
		if (replace != format_mapping.end()) {
			/* we should replace something */
			string tmp;
			switch (replace->second) {
				case TYPE_ALBUM:
					tmp = file->album;
					break;

				case TYPE_ALBUMARTIST:
					tmp = file->albumartist;
					break;

				case TYPE_ALBUMARTISTSORT:
					tmp = file->albumartistsort;
					break;

				case TYPE_ARTIST:
					tmp = file->artist;
					break;

				case TYPE_ARTISTSORT:
					tmp = file->artistsort;
					break;

				case TYPE_MUSICBRAINZ_ALBUMARTISTID:
					tmp = file->musicbrainz_albumartistid;
					break;

				case TYPE_MUSICBRAINZ_ALBUMID:
					tmp = file->musicbrainz_albumid;
					break;

				case TYPE_MUSICBRAINZ_ARTISTID:
					tmp = file->musicbrainz_artistid;
					break;

				case TYPE_MUSICBRAINZ_TRACKID:
					tmp = file->musicbrainz_trackid;
					break;

				case TYPE_MUSICIP_PUID:
					tmp = file->puid;
					break;

				case TYPE_TITLE:
					tmp = file->title;
					break;

				case TYPE_TRACKNUMBER:
					tmp = "";
					if (atoi(file->tracknumber.c_str()) <= 9)
						tmp.push_back('0');
					tmp.append(file->tracknumber);
					break;

				case TYPE_DATE:
					tmp = file->released;
					break;

				case TYPE_CUSTOM_ARTIST:
					if (file->musicbrainz_artistid != VARIOUS_ARTISTS_MBID)
						tmp = file->albumartistsort;
					else
						tmp = file->artistsort;
					break;

				case TYPE_GENRE:
					tmp = file->genre;

				default:
					/* didn't match anything, probably static entry */
					tmp = replace->first;
					break;
			}
			convertIllegalCharacters(&tmp);
			filename.erase(start, replace->first.size());
			filename.insert(start, tmp);
			start += tmp.size();
		}
	}
	return filename;
}

/* private methods */
void FileNamer::convertIllegalCharacters(string *text) {
	string::size_type pos = 0;
	while ((pos = text->find_first_of(illegal_characters), pos) != string::npos)
		text->replace(pos, 1, "_");
}
