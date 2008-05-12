#include "WebService.h"

/* constructors */
WebService::WebService(Locutus *locutus) {
	pthread_mutex_init(&mutex, NULL);
	this->locutus = locutus;
}

/* destructors */
WebService::~WebService() {
	pthread_mutex_destroy(&mutex);
}

/* methods */
Album WebService::fetchAlbum(string mbid) {
	Album album;
	if (mbid.size() != 36)
		return album;
	/* check if it's in database and updated recently first */
	mbid = locutus->database->escapeString(mbid);
	ostringstream query;
	query << "SELECT * FROM v_album_lookup WHERE album_mbid = '" << mbid << "' AND album_updated + INTERVAL '" << album_cache_lifetime << " months' < now()";
	if (locutus->database->query(query.str()) && locutus->database->getRows() > 0) {
		/* cool, we got this album in our "cache" */
		album.artist_mbid = locutus->database->getString(0, 0);
		album.artist_name = locutus->database->getString(0, 1);
		album.artist_sortname = locutus->database->getString(0, 2);
		album.mbid = locutus->database->getString(0, 3);
		album.type = locutus->database->getString(0, 4);
		// 0, 5 is album_updated, not needed
		album.title = locutus->database->getString(0, 6);
		album.released = locutus->database->getString(0, 7);
		for (int r = 0; r < locutus->database->getRows(); ++r) {
			Metadata track;
			track.setValue(MUSICBRAINZ_TRACKID, locutus->database->getString(r, 8));
			track.setValue(TITLE, locutus->database->getString(r, 9));
			track.duration = locutus->database->getInt(r, 10);
			track.setValue(TRACKNUMBER, locutus->database->getString(r, 11));
			track.setValue(MUSICBRAINZ_ARTISTID, locutus->database->getString(r, 12));
			track.setValue(ARTIST, locutus->database->getString(r, 13));
			track.setValue(ARTISTSORT, locutus->database->getString(r, 14));
			album.tracks[locutus->database->getInt(0, 11)] = track;
		}
		locutus->database->clear();
		return album;
	}
	locutus->database->clear();
	/* if not, then check web & update database */
	string url = release_lookup_url;
	url.append(mbid);
	url.append("?type=xml&inc=tracks+puids+artist+release-events+labels+artist-rels+url-rels");
	if (fetch(url.c_str()) && root.children["metadata"].size() > 0) {
		XMLNode release = root.children["metadata"][0].children["release"][0];
		album.mbid = release.children["id"][0].value;
		string ambide = locutus->database->escapeString(album.mbid);
		album.type = release.children["type"][0].value;
		string atypee = locutus->database->escapeString(album.type);
		album.title = release.children["title"][0].value;
		string atitlee = locutus->database->escapeString(album.title);
		album.artist_mbid = release.children["artist"][0].children["id"][0].value;
		string aambide = locutus->database->escapeString(album.artist_mbid);
		album.artist_name = release.children["artist"][0].children["name"][0].value;
		string aanamee = locutus->database->escapeString(album.artist_name);
		album.artist_sortname = release.children["artist"][0].children["sort-name"][0].value;
		string aasortnamee = locutus->database->escapeString(album.artist_sortname);
		if (release.children["release-event-list"].size() > 0) {
			album.released = release.children["released-event-list"][0].children["event"][0].children["date"][0].value;
			bool ok = false;
			if (album.released.size() == 10) {
				ok = true;
				for (int a = 0; a < 10 && ok; ++a) {
					if (a == 4 || a == 6 || a == 8) {
						if (album.released[a] != '-')
							ok = false;
					} else {
						if (album.released[a] < '0' || album.released[a] > '9')
							ok = false;
					}
				}
			}
			if (!ok) {
				if (album.released.size() >= 4) {
					bool yearok = true;
					for (int a = 0; a < 4 && yearok; ++a) {
						if (album.released[a] < '0' || album.released[a] > '9')
							yearok = false;
					}
					if (yearok)
						album.released = album.released.substr(0, 4);
					else
						album.released = "";
				} else {
					album.released = "";
				}
			}
		}
		string areleasede;
		if (album.released == "") {
			areleasede = "NULL";
		} else {
			areleasede = "'";
			areleasede.append(locutus->database->escapeString(album.released));
			areleasede.append("'");
		}
		query.str("");
		bool queries_ok = true;
		if (!locutus->database->query(query.str()))
			queries_ok = false;
		locutus->database->clear();
		if (queries_ok) {
			query.str("");
			query << "INSERT INTO artist(mbid, name, sortname, loaded) SELECT '" << aambide << "', '" << aanamee << "', '" << aasortnamee << "', true WHERE NOT EXISTS (SELECT true FROM artist WHERE mbid = '" << aambide << "')";
			if (!locutus->database->query(query.str()))
				queries_ok = false;
			locutus->database->clear();
			if (queries_ok) {
				query.str("");
				query << "UPDATE artist SET name = '" << aanamee << "', sortname = '" << aasortnamee << "', loaded = true WHERE mbid = '" << aambide << "'";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
			}
		}
		if (queries_ok) {
			query.str("");
			query << "INSERT INTO album(artist_id, mbid, type, title, released, loaded) SELECT (SELECT artist_id FROM artist WHERE mbid = '" << aambide << "'), '" << ambide << "', '" << atypee << "', '" << atitlee << "', " << areleasede << ", true WHERE NOT EXISTS (SELECT true FROM album WHERE mbid = '" << ambide << "')";
			if (!locutus->database->query(query.str()))
				queries_ok = false;
			locutus->database->clear();
			if (queries_ok) {
				query.str("");
				query << "UPDATE album SET artist_id = (SELECT artist_id FROM artist WHERE mbid = '" << aambide << "'), type = '" << atypee << "', title = '" << atitlee << "', released = " << areleasede << ", loaded = true WHERE mbid = '" << aambide << "'";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
			}
		}
		for (vector<XMLNode>::size_type a = 0; a < release.children["track-list"][0].children["track"].size(); ++a) {
			Metadata track;
			track.setValue(MUSICBRAINZ_TRACKID, release.children["track-list"][0].children["track"][a].children["id"][0].value);
			string tmbide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_TRACKID));
			track.setValue(TITLE, release.children["track-list"][0].children["track"][a].children["title"][0].value);
			string ttitlee = locutus->database->escapeString(track.getValue(TITLE));
			track.duration = atoi(release.children["track-list"][0].children["track"][a].children["duration"][0].value.c_str());
			ostringstream num;
			num << a;
			track.setValue(TRACKNUMBER, num.str());
			string tambide = "";
			if (release.children["track-list"][0].children["track"][a].children["artist"].size() > 0) {
				track.setValue(MUSICBRAINZ_ARTISTID, release.children["track-list"][0].children["track"][a].children["artist"][0].children["id"][0].value);
				tambide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_ARTISTID));
				track.setValue(ARTIST, release.children["track-list"][0].children["track"][a].children["artist"][0].children["name"][0].value);
				string tartist = locutus->database->escapeString(track.getValue(ARTIST));
				track.setValue(ARTISTSORT, release.children["track-list"][0].children["track"][a].children["artist"][0].children["sort-name"][0].value);
				string tartistsort = locutus->database->escapeString(track.getValue(ARTISTSORT));
				if (queries_ok) {
					query.str("");
					query << "INSERT INTO artist(mbid, name, sortname, loaded) SELECT '" << tambide << "', '" << tartist << "', '" << tartistsort << "', true WHERE NOT EXISTS (SELECT true FROM artist WHERE mbid = '" << aambide << "')";
					if (!locutus->database->query(query.str()))
						queries_ok = false;
					locutus->database->clear();
					if (queries_ok) {
						query.str("");
						query << "UPDATE artist SET name = '" << tartist << "', sortname = '" << tartistsort << "', loaded = true WHERE mbid = '" << aambide << "'";
						if (!locutus->database->query(query.str()))
							queries_ok = false;
						locutus->database->clear();
					}
				}
			} else {
				track.setValue(MUSICBRAINZ_ARTISTID, track.getValue(MUSICBRAINZ_ARTISTID));
				tambide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_ARTISTID));
				track.setValue(ARTIST, track.getValue(ALBUMARTIST));
				track.setValue(ARTISTSORT, track.getValue(ALBUMARTISTSORT));
			}
			album.tracks[a] = track;
			if (queries_ok) {
				query.str("");
				query << "INSERT INTO track(album_id, artist_id, mbid, title, duration, tracknumber) SELECT (SELECT album_id FROM album WHERE mbid = '" << ambide << "'), (SELECT artist_id FROM artist WHERE mbid = '" << tambide << "'), '" << tmbide << "', '" << ttitlee << "', " << track.duration << ", " << a << " WHERE NOT EXISTS (SELECT true FROM track WHERE mbid = '" << tmbide << "')";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
				if (queries_ok) {
					query.str("");
					query << "UPDATE track SET album_id = (SELECT album_id FROM album WHERE mbid = '" << ambide << "'), artist_id = (SELECT artist_id FROM artist WHERE mbid = '" << tambide << "'), title = '" << ttitlee << "', duration = " << track.duration << ", tracknumber = " << a << " WHERE mbid = '" << tmbide << "'";
				       if (!locutus->database->query(query.str()))
					       queries_ok = false;
				       locutus->database->clear();
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	return album;
}

void WebService::loadSettings() {
	setting_class_id = locutus->settings->loadClassID(WEBSERVICE_CLASS, WEBSERVICE_CLASS_DESCRIPTION);
	metadata_search_url = locutus->settings->loadSetting(setting_class_id, METADATA_SEARCH_URL_KEY, METADATA_SEARCH_URL_VALUE, METADATA_SEARCH_URL_DESCRIPTION);
	release_lookup_url = locutus->settings->loadSetting(setting_class_id, RELEASE_LOOKUP_URL_KEY, RELEASE_LOOKUP_URL_VALUE, RELEASE_LOOKUP_URL_DESCRIPTION);
	album_cache_lifetime = locutus->settings->loadSetting(setting_class_id, ALBUM_CACHE_LIFETIME_KEY, ALBUM_CACHE_LIFETIME_VALUE, ALBUM_CACHE_LIFETIME_DESCRIPTION);
}

vector<Metadata> WebService::searchMetadata(string wsquery) {
	if (wsquery == "")
		return vector<Metadata>();
	string url = metadata_search_url;
	url.append("?type=xml&");
	url.append(wsquery);
	vector<Metadata> tracks;
	if (fetch(url.c_str()) && root.children["metadata"].size() > 0) {
		XMLNode tracklist = root.children["metadata"][0].children["track-list"][0];
		for (vector<XMLNode>::size_type a = 0; a < root.children["metadata"][0].children["track-list"][0].children["track"].size(); ++a) {
			XMLNode tracknode = root.children["metadata"][0].children["track-list"][0].children["track"][a];
			Metadata track;
			track.setValue(MUSICBRAINZ_TRACKID, tracknode.children["id"][0].value);
			string tmbide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_TRACKID));
			track.setValue(TITLE, tracknode.children["title"][0].value);
			string ttitlee = locutus->database->escapeString(track.getValue(TITLE));
			track.duration = atoi(tracknode.children["duration"][0].value.c_str());
			track.setValue(MUSICBRAINZ_ARTISTID, tracknode.children["artist"][0].children["id"][0].value);
			string armbide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_ARTISTID));
			track.setValue(ARTIST, tracknode.children["artist"][0].children["artist"][0].value);
			string arnamee = locutus->database->escapeString(track.getValue(ARTIST));
			track.setValue(MUSICBRAINZ_ALBUMID, tracknode.children["release-list"][0].children["release"][0].children["id"][0].value);
			string almbide = locutus->database->escapeString(track.getValue(MUSICBRAINZ_ALBUMID));
			track.setValue(ALBUM, tracknode.children["release-list"][0].children["release"][0].children["title"][0].value);
			string altitlee = locutus->database->escapeString(track.getValue(ALBUM));
			string offset = tracknode.children["release-list"][0].children["release"][0].children["track-list"][0].children["offset"][0].value;
			int tracknum = atoi(offset.c_str()) + 1;
			stringstream num;
			num << tracknum;
			track.setValue(TRACKNUMBER, num.str());
			bool queries_ok = true;
			ostringstream query;
			if (!locutus->database->query(query.str()))
				queries_ok = false;
			locutus->database->clear();
			if (queries_ok) {
				query.str("");
				query << "INSERT INTO artist(mbid, name) SELECT '" << armbide << "', '" << arnamee << "' WHERE NOT EXISTS (SELECT true FROM artist WHERE mbid = '" << armbide << "')";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
				if (queries_ok) {
					query.str("");
					query << "UPDATE artist SET name = '" << arnamee << "' WHERE mbid = '" << armbide << "'";
					if (!locutus->database->query(query.str()))
						queries_ok = false;
					locutus->database->clear();
				}
			}
			if (queries_ok) {
				query.str("");
				query << "INSERT INTO album(artist_id, mbid, title) SELECT (SELECT artist_id FROM artist WHERE mbid = '" << armbide << "'), '" << almbide << "', '" << altitlee << "' WHERE NOT EXISTS (SELECT true FROM album WHERE mbid = '" << almbide << "')";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
				if (queries_ok) {
					query.str("");
					query << "UPDATE album SET artist_id = (SELECT artist_id FROM artist WHERE mbid = '" << armbide << "'), title = '" << altitlee << "' WHERE mbid = '" << almbide << "'";
					if (!locutus->database->query(query.str()))
						queries_ok = false;
					locutus->database->clear();
				}
			}
			if (queries_ok) {
				query.str("");
				query << "INSERT INTO track(album_id, artist_id, mbid, title, duration, tracknumber) SELECT (SELECT album_id FROM album WHERE mbid = '" << almbide << "'), (SELECT artist_id FROM artist WHERE mbid = '" << armbide << "'), '" << tmbide << "', '" << ttitlee << "', " << track.duration << ", " << tracknum << " WHERE NOT EXISTS (SELECT true FROM track WHERE mbid = '" << tmbide << "')";
				if (!locutus->database->query(query.str()))
					queries_ok = false;
				locutus->database->clear();
				if (queries_ok) {
					query.str("");
					query << "UPDATE track SET album_id = (SELECT album_id FROM album WHERE mbid = '" << almbide << "'), artist_id = (SELECT artist_id FROM artist WHERE mbid = '" << armbide << "'), title = '" << ttitlee << "', duration = " << track.duration << ", tracknumber = " << tracknum << " WHERE mbid = '" << tmbide << "'";
				       if (!locutus->database->query(query.str()))
					       queries_ok = false;
				       locutus->database->clear();
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	return tracks;
}

vector<Metadata> WebService::searchPUID(string puid) {
	if (puid == "")
		return vector<Metadata>();
	/* check if it's in database and updated recently first */
	string query = "puid=";
	query.append(puid);
	return searchMetadata(query);
}

/* private methods */
void WebService::characters(const unsigned char *text, size_t len) {
	curnode->value = string((char *) text, len);
}

void WebService::close() {
	URLStream::close();
}

void WebService::endElement(const unsigned char *name) {
	if (curnode != NULL)
		curnode = curnode->parent;
}

bool WebService::fetch(const char *url) {
	pthread_mutex_lock(&mutex);
	cout << url << endl;
	status = get(url);
	if (status) {
		cout << "failed; reason=" << status << endl;
		close();
		return false;
	}
	root.parent = NULL;
	root.children.clear();
	root.key = "root";
	root.value = "";
	curnode = &root;
	if (!parse())
		cout << "not well formed..." << endl;
	close();
	printXML(&root);
	return true;
}

void WebService::printXML(XMLNode *startnode) {
	if (startnode == NULL)
		return;
	if (startnode->parent == NULL)
		cout << startnode->key << ": " << startnode->value << endl;
	else
		cout << startnode->key << ": " << startnode->value << " (parent: " << startnode->parent->key << ")" << endl;
	for (map<string, vector<XMLNode> >::iterator it = startnode->children.begin(); it != startnode->children.end(); ++it) {
		for (vector<XMLNode>::size_type a = 0; a < it->second.size(); ++a)
			printXML(&it->second[a]);
	}
}

int WebService::read(unsigned char *buffer, size_t len) {
	URLStream::read((char *) buffer, len);
	return gcount();
}

void WebService::startElement(const unsigned char *name, const unsigned char **attr) {
	XMLNode childnode;
	childnode.parent = curnode;
	childnode.key = (char *) name;
	childnode.value = "";
	curnode->children[childnode.key].push_back(childnode);
	if (curnode->children[childnode.key].size() > 1)
		uniteChildrenWithParent(curnode, childnode.key);
	curnode = &curnode->children[childnode.key][curnode->children[childnode.key].size() - 1];
	if (attr != NULL) {
		while (*attr != NULL) {
			childnode.parent = curnode;
			childnode.key = (char *) *(attr++);
			childnode.value = (char *) *(attr++);
			curnode->children[childnode.key].push_back(childnode);
			if (curnode->children[childnode.key].size() > 1)
				uniteChildrenWithParent(curnode, childnode.key);
		}
	}
}

void WebService::uniteChildrenWithParent(XMLNode *parent, string key) {
	/* when we add more elements to a vector the other elements might be recreated.
	 * this means they'll get a new memory location, and the "parent" pointer in child nodes is invalid */
	for (vector<XMLNode>::size_type a = 0; a < parent->children[key].size() - 1; ++a) {
		for (map<string, vector<XMLNode> >::iterator it = parent->children[key][a].children.begin(); it != parent->children[key][a].children.end(); ++it) {
			for (vector<XMLNode>::size_type a = 0; a < it->second.size(); ++a)
				it->second[a].parent = parent;
		}
	}
}
