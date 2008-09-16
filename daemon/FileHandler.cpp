#include "FileHandler.h"

/* constructors */
FileHandler::FileHandler(Locutus *locutus) {
	this->locutus = locutus;
}

/* destructors */
FileHandler::~FileHandler() {
}

/* methods */
void FileHandler::loadSettings() {
	input_dir = locutus->settings->loadSetting(MUSIC_INPUT_KEY, MUSIC_INPUT_VALUE, MUSIC_INPUT_DESCRIPTION);
	output_dir = locutus->settings->loadSetting(MUSIC_OUTPUT_KEY, MUSIC_OUTPUT_VALUE, MUSIC_OUTPUT_DESCRIPTION);
	duplicate_dir = locutus->settings->loadSetting(MUSIC_DUPLICATE_KEY, MUSIC_DUPLICATE_VALUE, MUSIC_DUPLICATE_DESCRIPTION);
	createFileFormatList(locutus->settings->loadSetting(FILENAME_FORMAT_KEY, FILENAME_FORMAT_VALUE, FILENAME_FORMAT_DESCRIPTION));
}

void FileHandler::saveFiles(const map<Metafile *, Track *> &files) {
	locutus->debug(DEBUG_INFO, "Saving files:");
	for (map<Metafile *, Track *>::const_iterator s = files.begin(); s != files.end(); ++s) {
		locutus->debug(DEBUG_INFO, s->first->filename);
		/* first save metadata */
		if (!s->first->saveMetadata(s->second)) {
			/* unable to save metadata */
			continue;
		}
		/* move file */
		moveFile(s->first);
		/* and finally update file table */
		s->first->saveToCache();
	}
}

void FileHandler::scanFiles(const string &directory) {
	dir_queue.push_back(directory);
	while (dir_queue.size() > 0 || file_queue.size() > 0) {
		/* first files */
		if (parseFile())
			continue;
		/* then directories */
		if (parseDirectory())
			continue;
	}
}

/* private methods */
void FileHandler::createFileFormatList(const string &file_format) {
	file_format_list.clear();
	string::size_type stop = 0;
	string::size_type start = 0;
	while (start < file_format.size() && (stop = file_format.find_first_of("%", start)) != string::npos) {
		file_format_list.push_back(file_format.substr(start, stop - start));
		start = stop + 1;
	}
	if (start < file_format.size())
		file_format_list.push_back(file_format.substr(start));
	if (file_format_list.size() <= 0)
		locutus->debug(DEBUG_WARNING, "Output file format is empty, won't be able to save files");
}

bool FileHandler::moveFile(Metafile *file) {
	string filename = output_dir;
	for (list<string>::iterator entry = file_format_list.begin(); entry != file_format_list.end(); ++entry) {
		if ((*entry)[0] != '%' || entry->size() <= 1) {
			/* static entry */
			filename.append(*entry);
			continue;
		}
		int limit = -1;
		string::size_type limit_stop = entry->find_first_not_of("%0123456789");
		string token;
		if (limit_stop != string::npos) {
			/* entry should be limited */
			limit = atoi(entry->substr(1, limit_stop).c_str());
			token = entry->substr(limit_stop);
		} else {
			token = entry->substr(1);
		}
		if (token == "album") {
		} else if (token == "albumartist") {
		} else if (token == "albumartistsort") {
		} else if (token == "artist") {
		} else if (token == "artistsort") {
		} else if (token == "musicbrainz_albumartistid") {
		} else if (token == "musicbrainz_albumid") {
		} else if (token == "musicbrainz_artistid") {
		} else if (token == "musicbrainz_trackid") {
		} else if (token == "musicip_puid") {
		} else if (token == "title") {
		} else if (token == "tracknumber") {
		} else if (token == "date") {
		} else if (token == "custom_artist") {
		}
	}
	return false;
}

bool FileHandler::parseDirectory() {
	if (dir_queue.size() <= 0)
		return false;
	string directory(*dir_queue.begin());
	locutus->debug(DEBUG_INFO, directory);
	dir_queue.pop_front();
	DIR *dir = opendir(directory.c_str());
	if (dir == NULL)
		return true;
	dirent *entity;
	while ((entity = readdir(dir)) != NULL) {
		string entityname = entity->d_name;
		if (entityname == "." || entityname == "..")
			continue;
		string ford = directory;
		if (ford[ford.size() - 1] != '/')
			ford.append("/");
		ford.append(entityname);
		/* why isn't always "entity->d_type == DT_DIR" when the entity is a directory? */
		DIR *tmpdir = opendir(ford.c_str());
		if (tmpdir != NULL)
			dir_queue.push_back(ford);
		else
			file_queue.push_back(ford);
		closedir(tmpdir);
	}
	closedir(dir);
	return true;
}

bool FileHandler::parseFile() {
	if (file_queue.size() <= 0)
		return false;
	string filename(*file_queue.begin());
	locutus->debug(DEBUG_INFO, filename);
	file_queue.pop_front();
	Metafile *mf = new Metafile(locutus);
	if (!mf->loadFromCache(filename)) {
		if (mf->readFromFile(filename)) {
			/* save file to cache */
			mf->saveToCache();
		} else {
			/* unable to read this file */
			delete mf;
			return false;
		}
	}
	/* TODO:
	 * should be settings which lookups we want to run */
	mf->puid_lookup = true;
	mf->mbid_lookup = true;
	mf->meta_lookup = true;
	locutus->files.push_back(mf);
	locutus->grouped_files[mf->getGroup()].push_back(mf);
	return true;
}
