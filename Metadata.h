#ifndef METADATA_H
/* defines */
#define METADATA_H
/* keys */
#define ALBUM "ALBUM"
#define ALBUMARTIST "ALBUMARTIST"
#define ALBUMARTISTSORT "ALBUMARTISTSORT"
#define ARTIST "ARTIST"
#define ARTISTSORT "ARTISTSORT"
#define MUSICBRAINZ_ALBUMARTISTID "MUSICBRAINZ_ALBUMARTISTID"
#define MUSICBRAINZ_ALBUMID "MUSICBRAINZ_ALBUMID"
#define MUSICBRAINZ_ARTISTID "MUSICBRAINZ_ARTISTID"
#define MUSICBRAINZ_TRACKID "MUSICBRAINZ_TRACKID"
#define MUSICIP_PUID "MUSICIP_PUID"
#define TITLE "TITLE"
#define TRACKNUMBER "TRACKNUMBER"
#define YEAR "YEAR"

/* includes */
#include <list>
#include <string>
#include "Levenshtein.h"

/* namespaces */
using namespace std;

/* key/value for metadata */
struct Entry {
	string key;
	string value;
};

/* Metadata */
class Metadata {
	public:
		/* variables */
		int duration;
		list<Entry> entries;

		/* constructors */
		Metadata();
		Metadata(int duration);

		/* destructors */
		~Metadata();

		/* methods */
		bool equalMBID(Metadata target);
		bool equalMetadata(Metadata target);
		string getValue(const string key);
		void setValue(const string key, const string value);

	protected:
		/* variables */
};
#endif
