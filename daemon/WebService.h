#ifndef WEBSERVICE_H
#define WEBSERVICE_H
/* settings */
#define METADATA_SEARCH_URL_KEY "metadata_search_url"
#define METADATA_SEARCH_URL_VALUE "http://musicbrainz.org/ws/1/track/"
#define METADATA_SEARCH_URL_DESCRIPTION "URL to search after metadata"
#define RELEASE_LOOKUP_URL_KEY "release_url"
#define RELEASE_LOOKUP_URL_VALUE "http://musicbrainz.org/ws/1/release/"
#define RELEASE_LOOKUP_URL_DESCRIPTION "URL to lookup a release"

#include <cc++/common.h>
#include <map>
#include <string>
#include <vector>
#include "Metatrack.h"

#include "Database.h" // XXX

class Album;
class XMLNode;

class Locutus; // XXX

class WebService : public ost::URLStream, public ost::XMLStream {
	public:
		/* constructors/destructor */
		WebService(Locutus *locutus);
		~WebService();

		/* methods */
		void loadSettings();
		bool lookupAlbum(Album *album);
		std::vector<Metatrack> *searchMetadata(const std::string &wsquery);
		std::vector<Metatrack> *searchPUID(const std::string &puid);

	private:
		/* variables */
		Locutus *locutus;
		std::vector<Metatrack> *tracks;
		URLStream::Error status;
		std::string metadata_search_url;
		std::string release_lookup_url;
		XMLNode *root;
		XMLNode *curnode;

		/* methods */
		void characters(const unsigned char *text, size_t len);
		void close();
		void endElement(const unsigned char *name);
		bool fetch(const char *url);
		void printXML(XMLNode *startnode, int indent) const;
		int read(unsigned char *buffer, size_t len);
		void startElement(const unsigned char *name, const unsigned char **attr);
};
#endif
