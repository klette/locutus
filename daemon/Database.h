#ifndef DATABASE_H
#define DATABASE_H

#include <string>

class Album;
class Artist;
class Match;
class Metafile;
class Metatrack;
class Track;

class Database {
	public:
		Database();
		virtual ~Database();

		virtual bool loadAlbum(Album *album);
		virtual bool loadMetafile(Metafile *metafile);
		virtual bool loadSettingBool(const std::string &key, bool default_value, const std::string &description);
		virtual double loadSettingDouble(const std::string &key, double default_value, const std::string &description);
		virtual int loadSettingInt(const std::string &key, int default_value, const std::string &description);
		virtual std::string loadSettingString(const std::string &key, const std::string &default_value, const std::string &description);
		virtual bool saveAlbum(const Album &album);
		virtual bool saveArtist(const Artist &artist);
		virtual bool saveMatch(const Match &match);
		virtual bool saveMetafile(const Metafile &metafile, const std::string &old_filename = "");
		virtual bool saveMetatrack(const Metatrack &metatrack);
		virtual bool saveTrack(const Track &track);
};
#endif
