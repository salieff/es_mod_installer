#!/usr/bin/perl -w

use strict;
use utf8;

use LWP::UserAgent;
use JSON;
use CGI;
use POSIX;

binmode(STDOUT,':utf8');

my $mozillaAgent = 'Mozilla/5.0 (Linux; Android 4.4.2; Nexus 5 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.99 Mobile Safari/537.36';
my $baseUrl = 'http://191.ru/es';
my $projectUrl = 'project2.json';
my $allLikesUrl = 'http://es.191.ru/cgi-bin/ratingsystem/rating_web.py?operation=queryallmarks&mac=00:00:00:00:00:00&udid=HTMLModsTableGenerator';

sub fileDateSize {
	my $ua = shift;
	my $file = shift;

	my $req = new HTTP::Request(HEAD => $baseUrl . '/' . $file);
	my $res = $ua->request($req);
	die "Can't request data from " . $req->uri() . " : " . $res->error_as_HTML() if $res->is_error();

	return ($res->last_modified(), $res->content_length());
}

sub filesDateSizeByPlatform {
	my $ua = shift;
	my $pack = shift;
	my $platform = shift;

	my $newestDate = undef;
	my $fullSize = 0;
	for my $file (@{$pack->{'files_' . lc $platform}}) {
		my ($tmStamp, $size) = fileDateSize($ua, $file);
		$newestDate = $tmStamp if (!defined($newestDate) || $newestDate < $tmStamp);
		$fullSize += $size;
	}

	return ($newestDate, $fullSize);
}

sub filesDateSize {
	my $ua = shift;
	my $pack = shift;

	my @dates = ();
	my @sizes = ();
	for my $platform (@{$pack->{platforms}}) {
		my ($date, $size) = filesDateSizeByPlatform($ua, $pack, $platform);
		push(@dates, $date);
		push(@sizes, $size);
	}

	return (\@dates, \@sizes);
}

sub prettyBytes {
	my $size = shift;
	foreach ('b', 'kb', 'Mb', 'Gb', 'Tb', 'Pb', 'Eb', 'Zb', 'Yb') {
		return sprintf("%.1f ",$size) . "$_" if $size < 1024;
		$size /= 1024;
	}
}

sub filesDateSizeString {
	my $ua = shift;
	my $pack = shift;

	my ($dates, $sizes) = filesDateSize($ua, $pack);

	my $retDtStr = undef;
	my $newestDate = undef;

	for my $date (@{$dates}) {
		my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime($date);
		my $dtStr  = sprintf("%02d", $mday) . '.';
		$dtStr    .= sprintf("%02d", $mon + 1) . '.';
		$dtStr    .= sprintf("%04d", $year + 1900);

		if (!defined($retDtStr)) {
			$retDtStr = "";
		}
		else {
			$retDtStr .= " / ";
		}

		$retDtStr .= $dtStr;

		if (!defined($newestDate) || $newestDate < $date) {
			$newestDate = $date;
		}
	}

	my $retSzStr = undef;
	my $biggestSize = undef;

	for my $size (@{$sizes}) {
		if (!defined($retSzStr)) {
			$retSzStr = "";
		}
		else {
			$retSzStr .= " / ";
		}

		$retSzStr .= prettyBytes($size);

		if (!defined($biggestSize) || $biggestSize < $size) {
			$biggestSize = $size;
		}
	}

	return ($retDtStr, $retSzStr, $newestDate, $biggestSize);
}

sub prettyPlatform {
	my $platform = shift;

	if (lc $platform eq 'android') {
		return 'Android';
	}

	if (lc $platform eq 'ios') {
		return 'iOS';
	}

	return $platform;
}

sub platformString {
	my $pack = shift;

	my $retString = undef;
	for my $platform (@{$pack->{platforms}}) {
		if (defined($retString)) {
			$retString .= ' / ';
		}
		else {
			$retString = '';
		}

		$retString .= prettyPlatform($platform);
	}

	return $retString;
}

sub urlsString {
	my $cgi = shift;
	my $pack = shift;

	my $retString = undef;
	for my $platform (@{$pack->{platforms}}) {
		if (defined($retString)) {
			$retString .= ' / ';
		}
		else {
			$retString = '';
		}

		$retString .= $cgi->a({href=>$pack->{'infouri_' . lc $platform}, target=>"_blank"}, prettyPlatform($platform));
	}

	return $retString;
}

sub statusSortKey {
	my $pack = shift;

	if ($pack->{status} eq "окончен") {
		return 1;
	}

	if ($pack->{status} eq "в разработке") {
		return 2;
	}

	if ($pack->{status} eq "заморожен") {
		return 3;
	}

	if ($pack->{status} eq "демо") {
		return 4;
	}

	if ($pack->{status} eq "надстройка") {
		return 5;
	}

	if ($pack->{status} eq "обучаловка") {
		return 6;
	}

	return 999;
}

sub loadModsList {
	my $ua = shift;

	my $req = new HTTP::Request(GET => $baseUrl . '/' . $projectUrl);
	my $res = $ua->request($req);
	die "Can't request data from " . $req->uri() . " : " . $res->error_as_HTML() if $res->is_error();

	my $json = $res->content();
	my $decJson = decode_json($json);
	die "Can't decode JSON " . $json if !defined($decJson);

	return $decJson;
}

sub loadAllLikes {
	my $ua = shift;

	my $req = new HTTP::Request(GET => $allLikesUrl);
	my $res = $ua->request($req);
	die "Can't request data from " . $req->uri() . " : " . $res->error_as_HTML() if $res->is_error();

	my $json = $res->content();
	my $decJson = decode_json($json);
	die "Can't decode JSON " . $json if !defined($decJson);
	die "Can't load likes list: " . $decJson->{result} if ($decJson->{result} ne "ok");

	return $decJson;
}

sub scoreAsString {
	my $score = shift;

	if ($score < 1) {
		return '';
	}

	if ($score >= 1 && $score < 1.17) {
		return '1';
	}

	if ($score >= 1.17 && $score < 1.5) {
		return '1+';
	}

	if ($score >= 1.5 && $score < 1.83) {
		return '2-';
	}

	if ($score >= 1.83 && $score < 2.17) {
		return '2';
	}

	if ($score >= 2.17 && $score < 2.5) {
		return '2+';
	}

	if ($score >= 2.5 && $score < 2.83) {
		return '3-';
	}

	if ($score >= 2.83 && $score < 3.17) {
		return '3';
	}

	if ($score >= 3.17 && $score < 3.5) {
		return '3+';
	}

	if ($score >= 3.5 && $score < 3.83) {
		return '4-';
	}

	if ($score >= 3.83 && $score < 4.17) {
		return '4';
	}

	if ($score >= 4.17 && $score < 4.5) {
		return '4+';
	}

	if ($score >= 4.5 && $score < 4.83) {
		return '5-';
	}

	if ($score >= 4.83 && $score < 5.17) {
		return '5';
	}

	if ($score >= 5.17) {
		return '5+';
	}

	return 'WTF?';
}

sub imageForScore {
	my $score = shift;

	if ($score < 1) {
		return "nolikes.png";
	}

	if ($score >= 3) {
		return "like.png";
	}

	return "dislike.png"
}

sub scoreForMod {
	my $likes = shift;
	my $pack = shift;
	my $score = 0;

	for my $mark (@{$likes->{marks}}) {
		if ($mark->{id} == $pack->{idmod}) {

			if ($mark->{up} <= 0 && $mark->{down} <= 0) {
				$score = 0;
				last;
			}

			if ($mark->{up} <= 0) {
				$score = 1;
				last;
			}

			$score = 1.0 + $mark->{up} * 4.5 / ($mark->{up} + $mark->{down});
			last;
		}
	}

	return (scoreAsString($score), $score, imageForScore($score));
}

sub main {
	my $ua = new LWP::UserAgent;
	$ua->env_proxy();
	$ua->agent($mozillaAgent);

	my $modsList = loadModsList($ua);
	my $likesList = loadAllLikes($ua);

	my $q = CGI->new;
#	print $q->header(-charset => 'utf-8');
	$q->header(-charset => 'utf-8');
	print $q->start_html(-style => {'src'=>'mods_table.css'}, -script=>{-type=>'JAVASCRIPT', -src=>'sorttable.js'}, -title => 'Список портированных модов');

	my @headings = ('Название', 'Язык', 'Статус', 'Описание', 'Дата', 'Размер', 'Оценка');

	print $q->start_table({-class => "sortable"}) . "\n";
	print $q->Tr($q->th(\@headings)) . "\n";

	for my $pack (@{$modsList->{packs}}) {
		my ($dates, $sizes, $sortDate, $sortSize) = filesDateSizeString($ua, $pack);
		my ($score, $sortScore, $scoreImage) = scoreForMod($likesList, $pack);
		my $statusSK = statusSortKey($pack);
		my $prettyLang = $pack->{lang};
		$prettyLang =~ s/\s*,\s*/, /g;

		print "<tr>\n";
		print "<td>" . $pack->{title} . "</td>\n";
		print "<td style=\"white-space:nowrap;\">" . $prettyLang . "</td>\n";
		print "<td style=\"white-space:nowrap;\" sorttable_customkey=\"" . $statusSK . "\">" . $pack->{status} . "</td>\n";
		print "<td style=\"white-space:nowrap;\">" . urlsString($q, $pack) . "</td>\n";
		print "<td style=\"white-space:nowrap;\" sorttable_customkey=\"" . $sortDate . "\">" . $dates . "</td>\n";
		print "<td style=\"white-space:nowrap;\" sorttable_customkey=\"" . $sortSize . "\">" . $sizes . "</td>\n";
		print "<td style=\"white-space:nowrap;\" sorttable_customkey=\"" . $sortScore . "\">" . "<img src=\"" . $scoreImage . "\">" . "&nbsp;<span class=\"redscore\">" . $score . "</span></td>\n";
		print "</tr>\n";
	}

	print $q->end_table() . "\n";
	print "<br/><br/>Обновлено " . strftime("%F %T", localtime($^T)) . "\n";
	print $q->end_html() . "\n";
}

main;
