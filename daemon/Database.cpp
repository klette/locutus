#include "Database.h"

using namespace std;

/* constructors/destructor */
Database::Database() {
}

Database::~Database() {
}

/* methods */
bool Database::loadAlbum(Album *album) {
	return false;
}

bool Database::loadMetafile(Metafile *metafile) {
	return false;
}

bool Database::loadSettingBool(const string &key, bool default_value, const string &description) {
	return default_value;
}

double Database::loadSettingDouble(const string &key, double default_value, const string &description) {
	return default_value;
}

int Database::loadSettingInt(const string &key, int default_value, const string &description) {
	return default_value;
}

string Database::loadSettingString(const string &key, const string &default_value, const string &description) {
	return default_value;
}

bool Database::saveAlbum(const Album &album) {
	return false;
}

bool Database::saveArtist(const Artist &artist) {
	return false;
}

bool Database::saveMatch(const Match &match) {
	return false;
}

bool Database::saveMetafile(const Metafile &metafile, const string &old_filename) {
	return false;
}

bool Database::saveMetatrack(const Metatrack &metatrack) {
	return false;
}

bool Database::saveTrack(const Track &track) {
	return false;
}
