#include "Album.h"
#include "Database.h"
#include "Debug.h"
#include "Levenshtein.h"
#include "Matcher.h"
#include "Metafile.h"
#include "Metatrack.h"
#include "WebService.h"

using namespace std;

/* constructors/destructor */
Matcher::Matcher(Database *database, WebService *webservice) : database(database), webservice(webservice) {
	puid_min_score = database->loadSetting(PUID_MIN_SCORE_KEY, PUID_MIN_SCORE_VALUE, PUID_MIN_SCORE_DESCRIPTION);
	metadata_min_score = database->loadSetting(METADATA_MIN_SCORE_KEY, METADATA_MIN_SCORE_VALUE, METADATA_MIN_SCORE_DESCRIPTION);
	album_weight = database->loadSetting(ALBUM_WEIGHT_KEY, ALBUM_WEIGHT_VALUE, ALBUM_WEIGHT_DESCRIPTION);
	artist_weight = database->loadSetting(ARTIST_WEIGHT_KEY, ARTIST_WEIGHT_VALUE, ARTIST_WEIGHT_DESCRIPTION);
	duration_limit = database->loadSetting(DURATION_LIMIT_KEY, DURATION_LIMIT_VALUE, DURATION_LIMIT_DESCRIPTION);
	duration_weight = database->loadSetting(DURATION_WEIGHT_KEY, DURATION_WEIGHT_VALUE, DURATION_WEIGHT_DESCRIPTION);
	title_weight = database->loadSetting(TITLE_WEIGHT_KEY, TITLE_WEIGHT_VALUE, TITLE_WEIGHT_DESCRIPTION);
	tracknumber_weight = database->loadSetting(TRACKNUMBER_WEIGHT_KEY, TRACKNUMBER_WEIGHT_VALUE, TRACKNUMBER_WEIGHT_DESCRIPTION);
	combine_threshold = database->loadSetting(COMBINE_THRESHOLD_KEY, COMBINE_THRESHOLD_VALUE, COMBINE_THRESHOLD_DESCRIPTION);
}

Matcher::~Matcher() {
}

/* methods */
void Matcher::match(const string &group, const vector<Metafile *> &files) {
	/* look up puids first */
	lookupPUIDs(files);
	/* then look up mbids */
	lookupMBIDs(files);
	/* compare all tracks in group with albums loaded so far */
	for (map<string, MatchGroup>::iterator mg = mgs.begin(); mg != mgs.end(); ++mg)
		compareFilesWithAlbum(mg->first, files);
	/* search using metadata */
	searchMetadata(group, files);
	/* and then match the files to albums */
	matchFilesToAlbums(files);
	/* clear data */
	clearMatchGroup();
}

/* private methods */
void Matcher::clearMatchGroup() {
	for (map<string, MatchGroup>::iterator mg = mgs.begin(); mg != mgs.end(); ++mg)
		delete mg->second.album;
	mgs.clear();
}

void Matcher::compareFilesWithAlbum(const string &mbid, const vector<Metafile *> &files) {
	if (mgs.find(mbid) == mgs.end())
		return;
	Album *album = mgs[mbid].album;
	for (vector<Metafile *>::const_iterator mf = files.begin(); mf != files.end(); ++mf) {
		for (vector<Track *>::size_type t = 0; t < album->tracks.size(); ++t) {
			if (mgs[mbid].scores[t].find(*mf) != mgs[mbid].scores[t].end())
				continue;
			Metatrack mt = album->tracks[t]->getAsMetatrack();
			Match m = compareMetafileWithMetatrack(**mf, mt);
			if (m.meta_score >= metadata_min_score)
				(*mf)->meta_lookup = false; // so good match that we won't lookup this track using metadata
			mgs[mbid].scores[t][*mf] = m;
			database->save(mt);
			saveMatchToCache((*mf)->filename, mt.track_mbid, m);
		}
	}
}

Match Matcher::compareMetafileWithMetatrack(const Metafile &metafile, const Metatrack &metatrack) {
	Match m;
	m.puid_match = false;
	m.mbid_match = false;
	m.meta_score = 0.0;
	list<string> values;
	if (metafile.album != "")
		values.push_back(metafile.album);
	if (metafile.albumartist != "")
		values.push_back(metafile.albumartist);
	if (metafile.artist != "")
		values.push_back(metafile.artist);
	if (metafile.title != "")
		values.push_back(metafile.title);
	if (metafile.tracknumber != "")
		values.push_back(metafile.tracknumber);
	string group = metafile.getGroup();
	if (group != "" && group != metafile.album)
		values.push_back(group);
	string basename = metafile.getBaseNameWithoutExtension();
	/* TODO: basename */
	if (values.size() <= 0)
		return m;
	/* find highest score */
	double scores[4][values.size()];
	int pos = 0;
	for (list<string>::iterator v = values.begin(); v != values.end(); ++v) {
		scores[0][pos] = Levenshtein::similarity(*v, metatrack.album_title);
		scores[1][pos] = Levenshtein::similarity(*v, metatrack.artist_name);
		scores[2][pos] = Levenshtein::similarity(*v, metatrack.track_title);
		scores[3][pos] = (atoi(v->c_str()) == metatrack.tracknumber) ? 1.0 : 0.0;
		++pos;
	}
	bool used_row[4];
	for (int a = 0; a < 4; ++a)
		used_row[a] = false;
	bool used_col[4];
	for (list<string>::size_type a = 0; a < values.size(); ++a)
		used_col[a] = false;
	for (int a = 0; a < 4; ++a) {
		int best_row = -1;
		list<string>::size_type best_col = -1;
		double best_score = -1.0;
		for (int r = 0; r < 4; ++r) {
			if (used_row[r])
				continue;
			for (list<string>::size_type c = 0; c < values.size(); ++c) {
				if (used_col[c])
					continue;
				if (scores[r][c] > best_score) {
					best_row = r;
					best_col = c;
					best_score = scores[r][c];
				}
			}
		}
		if (best_row >= 0) {
			scores[best_row][0] = best_score;
			used_row[best_row] = true;
			used_col[best_col] = true;
		} else {
			break;
		}
	}
	m.puid_match = (metafile.puid != "" && metafile.puid == metatrack.puid);
	m.mbid_match = (metafile.musicbrainz_trackid != "" && metafile.musicbrainz_trackid == metatrack.track_mbid);
	m.meta_score = scores[0][0] * album_weight;
	m.meta_score += scores[1][0] * artist_weight;
	m.meta_score += scores[2][0] * title_weight;
	m.meta_score += scores[3][0] * tracknumber_weight;
	int durationdiff = abs(metatrack.duration - metafile.duration);
	if (durationdiff < duration_limit)
		m.meta_score += (1.0 - durationdiff / duration_limit) * duration_weight;
	m.meta_score /= album_weight + artist_weight + title_weight + tracknumber_weight + duration_weight;
	return m;
}

bool Matcher::loadAlbum(const string &mbid) {
	if (mbid.size() != 36)
		return false;
	if (mgs.find(mbid) != mgs.end())
		return true; // already loaded
	Album *album = new Album(mbid);
	if (!database->load(album)) {
		if (webservice->lookupAlbum(album)) {
			database->save(*album);
		} else {
			/* hmm, didn't find the album? */
			delete album;
			return false;
		}
	}
	mgs[mbid].album = album;
	mgs[mbid].scores.resize((int) album->tracks.size());
	return true;
}

void Matcher::lookupMBIDs(const vector<Metafile *> &files) {
	for (vector<Metafile *>::const_iterator file = files.begin(); file != files.end(); ++file) {
		Metafile *mf = *file;
		if (!mf->mbid_lookup || mf->musicbrainz_albumid.size() != 36 || mgs.find(mf->musicbrainz_albumid) != mgs.end())
			continue;
		if (loadAlbum(mf->musicbrainz_albumid))
			mf->meta_lookup = false; // shouldn't look up using metadata
	}
}

void Matcher::lookupPUIDs(const vector<Metafile *> &files) {
	/* TODO:
	 * we'll need some sort of handling when:
	 * - no matching tracks
	 * - matching tracks, but no good mbid/metadata match */
	for (vector<Metafile *>::const_iterator file = files.begin(); file != files.end(); ++file) {
		Metafile *mf = *file;
		if (!mf->puid_lookup || mf->puid.size() != 36)
			continue;
		vector<Metatrack> tracks = webservice->searchPUID(mf->puid);
		for (vector<Metatrack>::iterator mt = tracks.begin(); mt != tracks.end(); ++mt) {
			/* puid search won't return puid, so let's set it manually */
			mt->puid = mf->puid;
			Match m = compareMetafileWithMetatrack(*mf, *mt);
			database->save(*mt);
			saveMatchToCache(mf->filename, mt->track_mbid, m);
			if (m.meta_score < puid_min_score)
				continue;
			if (!loadAlbum(mt->album_mbid))
				continue;
			int trackcount = (int) mgs[mt->album_mbid].album->tracks.size();
			if (mt->tracknumber > trackcount || mt->tracknumber <= 0) {
				/* this should never happen */
				Debug::notice("PUID search returned a tracknumber that doesn't exist on the album. This shouldn't happen, though");
				continue;
			}
			/* since we've already calculated the score, save it */
			mgs[mt->album_mbid].scores[mt->tracknumber - 1][mf] = m;
			/* if we found a match using puid we shouldn't look up using metadata */
			mf->meta_lookup = false;
		}
	}
}

void Matcher::matchFilesToAlbums(const vector<Metafile *> &files) {
	/* well, this method is a total mess :(
	 *
	 * this is what it's supposed to do:
	 * 1. find best album:
	 *    * match_score = meta_score * (3 if mbid_match, 2 if puid_match, 1 if neither)
	 *    * album_score = matches/tracks * match_score
	 * 2. make files matched unavailable for matching with next album
	 * 3. goto 1
	 *
	 * notes:
	 * - add "only save if all files in group match something" setting
	 * - add "only save complete albums" setting
	 *   * best album must be complete, it's not enough with any album being complete.
	 *     this because there often are singles with same tracks as an album
	 */
	bool only_save_if_all_match = true; // TODO: configurable
	bool only_save_complete_albums = true; // TODO: configurable
	map<Metafile *, Track *> save_files;
	int total_matched = 0;
	/* find best album */
	for (map<string, MatchGroup>::size_type mgtmp = 0; mgtmp < mgs.size(); ++mgtmp) {
		double best_album_score = 0.0;
		map<Metafile *, Track *> best_album_files;
		string best_album = "";
		map<Metafile *, Track *> used_files;
		for (map<string, MatchGroup>::iterator mg = mgs.begin(); mg != mgs.end(); ++mg) {
			map<int, bool> used_tracks;
			map<Metafile *, Track *> album_files;
			double album_score = 0.0;
			int files_matched = 0;
			int trackcount = (int) mgs[mg->first].scores.size();
			/* find best track */
			for (int tracktmp = 0; tracktmp < trackcount; ++tracktmp) {
				double best_track_score = 0.0;
				int best_track = -1;
				map<Metafile *, Match>::iterator best_file;
				for (int track = 0; track < trackcount; ++track) {
					if (used_tracks.find(track) != used_tracks.end())
						continue;
					/* find best file to track */
					for (map<Metafile *, Match>::iterator match = mgs[mg->first].scores[track].begin(); match != mgs[mg->first].scores[track].end(); ++match) {
						if (used_files.find(match->first) != used_files.end() || save_files.find(match->first) != save_files.end())
							continue;
						else if (!match->second.mbid_match && !match->second.puid_match && match->second.meta_score < metadata_min_score)
							continue;
						else if (!match->second.puid_match && match->second.meta_score < puid_min_score)
							continue;
						double track_score = match->second.meta_score * (match->second.mbid_match ? 3 : (match->second.puid_match ? 2 : 1));
						if (track_score > best_track_score) {
							best_track_score = track_score;
							best_track = track;
							best_file = match;
						}
					}
				}
				if (best_track != -1) {
					used_files[best_file->first] = mg->second.album->tracks[best_track];
					used_tracks[best_track] = true;
					album_files[best_file->first] = mg->second.album->tracks[best_track];
					album_score += best_track_score;
					++files_matched;
				}
			}
			if (only_save_complete_albums && files_matched != trackcount)
				continue;
			album_score *= (double) files_matched / (double) trackcount;
			if (album_score > best_album_score) {
				best_album_score = album_score;
				best_album_files = album_files;
				best_album = mg->first;
			}
		}
		if (best_album != "") {
			for (map<Metafile *, Track *>::iterator file = best_album_files.begin(); file != best_album_files.end(); ++file) {
				save_files[file->first] = file->second;
				++total_matched;
			}
		}
	}
	if (save_files.size() <= 0 || (only_save_if_all_match && total_matched != (int) files.size()))
		return;
	/* set new metadata */
	for (map<Metafile *, Track *>::iterator file = save_files.begin(); file != save_files.end(); ++file)
		file->first->setMetadata(file->second);
}

bool Matcher::saveMatchToCache(const string &filename, const string &track_mbid, const Match &match) const {
	/*
	if (filename == "" || track_mbid.size() != 36)
		return false;
	string e_filename = database->escapeString(filename);
	string e_track_mbid = database->escapeString(track_mbid);
	ostringstream query;
	query << "INSERT INTO match(file_id, metatrack_id, mbid_match, puid_match, meta_score) SELECT (SELECT file_id FROM file WHERE filename = '" << e_filename << "'), (SELECT metatrack_id FROM metatrack WHERE track_mbid = '" << e_track_mbid << "'), " << (match.mbid_match ? "true" : "false") << ", " << (match.puid_match ? "true" : "false") << ", " << match.meta_score << " WHERE NOT EXISTS (SELECT true FROM match WHERE file_id = (SELECT file_id FROM file WHERE filename = '" << e_filename << "') AND metatrack_id = (SELECT metatrack_id FROM metatrack WHERE track_mbid = '" << e_track_mbid << "'))";
	if (!database->query(query.str()))
		Debug::notice("Unable to save metadata match in cache, query failed. See error above");
	query.str("");
	query << "UPDATE match SET mbid_match = " << (match.mbid_match ? "true" : "false") << ", puid_match = "  << (match.puid_match ? "true" : "false") << ", meta_score = " << match.meta_score << " WHERE file_id = (SELECT file_id FROM file WHERE filename = '" << e_filename << "') AND metatrack_id = (SELECT metatrack_id FROM metatrack WHERE track_mbid = '" << e_track_mbid << "')";
	if (!database->query(query.str()))
		Debug::notice("Unable to save metadata match in cache, query failed. See error above");
	*/
	return true;
}

void Matcher::searchMetadata(const string &group, const vector<Metafile *> &files) {
	for (vector<Metafile *>::const_iterator file = files.begin(); file != files.end(); ++file) {
		Metafile *mf = *file;
		if (!mf->meta_lookup)
			continue;
		vector<Metatrack> tracks = webservice->searchMetadata(group, *mf);
		for (vector<Metatrack>::iterator mt = tracks.begin(); mt != tracks.end(); ++mt) {
			Match m = compareMetafileWithMetatrack(*mf, *mt);
			database->save(*mt);
			saveMatchToCache(mf->filename, mt->track_mbid, m);
			if (m.meta_score < metadata_min_score)
				continue;
			if (!loadAlbum(mt->album_mbid))
				continue;
			int trackcount = (int) mgs[mt->album_mbid].album->tracks.size();
			if (mt->tracknumber > trackcount || mt->tracknumber <= 0) {
				/* this should never happen */
				Debug::notice("Metadata search returned a tracknumber that doesn't exist on the album. This shouldn't happen, though");
				continue;
			}
			/* since we've already calculated the score, save it */
			mgs[mt->album_mbid].scores[mt->tracknumber - 1][mf] = m;
			compareFilesWithAlbum(mt->album_mbid, files);
		}
	}
}
