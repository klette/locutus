#!/usr/bin/perl
use strict;
use warnings;

use CGI qw(:standard);
use Data::Dumper;

use lib '../include';
use Locutus;

my $page = 'file';
my %vars = ();

my $dbh = Locutus::db_connect();

my $fiid = int(param('fiid'));

$vars{file} = $dbh->selectrow_hashref('SELECT * FROM v_web_info_file WHERE file_id = ' . $fiid);
$vars{matches} = $dbh->selectall_arrayref('SELECT * FROM v_web_list_matches WHERE file_file_id = ' . $fiid . ' ORDER BY mbid_match DESC, puid_match DESC, meta_score DESC', {Slice => {}});

foreach my $match (@{$vars{matches}}) {
	$match->{color} = Locutus::score_to_color($match->{meta_score});
	$match->{meta_score} = sprintf("%.1f%%", $match->{meta_score} * 100);
}

#print Dumper(\%vars);

Locutus::process_template($page, \%vars);
