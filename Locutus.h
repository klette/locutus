#ifndef LOCUTUS_H
/* defines */
#define LOCUTUS_H
#define DEBUG_ERROR 3
#define DEBUG_WARNING 2
#define DEBUG_NOTICE 1
#define DEBUG_INFO 0

/* forward declare */
class Locutus;

/* includes */
#include <iostream>
#include <map>
#include <vector>
#include "Database.h"
#include "FileMetadata.h"
#include "FileMetadataConstants.h"
#include "FileReader.h"
#include "Levenshtein.h"
#include "Metadata.h"
#include "PUIDGenerator.h"
#include "Settings.h"
#include "WebFetcher.h"
#include "WebService.h"

/* namespace */
using namespace std;

/* Locutus */
class Locutus {
	public:
		/* variables */
		Database *database;
		FileMetadataConstants *fmconst;
		Levenshtein *levenshtein;
		Settings *settings;
		WebService *webservice;
		FileReader *filereader;
		PUIDGenerator *puidgen;
		WebFetcher *webfetcher;
		vector<FileMetadata> files;
		map<string, vector<int> > grouped_files; // album/directory, files
		vector<int> gen_puid_queue; // files missing puid

		/* constructors */
		Locutus();

		/* destructors */
		~Locutus();

		/* methods */
		void debug(int level, string text);
		long run();

	private:
		/* variables */
		ofstream *debugfile;

		/* methods */
		void loadSettings();
		void scanDirectory(string directory);
};
#endif
