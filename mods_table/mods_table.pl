#!/usr/bin/perl -w

use strict;
use utf8;

use LWP::UserAgent;
use JSON;
use CGI;

binmode(STDOUT,':utf8');

my $mozillaAgent = 'Mozilla/5.0 (Linux; Android 4.4.2; Nexus 5 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.99 Mobile Safari/537.36';
my $baseUrl = 'http://191.ru/es';
my $projectUrl = 'project2.json';

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
		return sprintf("%.2f ",$size) . "$_" if $size < 1024;
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

sub main {
	my $req = new HTTP::Request(GET => $baseUrl . '/' . $projectUrl);

	my $ua = new LWP::UserAgent;
	$ua->agent($mozillaAgent);

	my $res = $ua->request($req);
	die "Can't request data from " . $req->uri() . " : " . $res->error_as_HTML() if $res->is_error();

	my $json = $res->content();
	my $decJson = decode_json($json);
	die "Can't decode JSON " . $json if !defined($decJson);

	my $q = CGI->new;
#	print $q->header(-charset => 'utf-8');
	$q->header(-charset => 'utf-8');
	print $q->start_html(-style => {'src'=>'mods_table.css'}, -script=>{-type=>'JAVASCRIPT', -src=>'sorttable.js'}, -title => 'Список портированных модов');

	my @headings = ('Название', 'Язык', 'Статус', 'Платформа', 'Дата', 'Размер', 'Ссылки');

	print $q->start_table({-class => "sortable"}) . "\n";
	print $q->Tr($q->th(\@headings)) . "\n";

	for my $pack (@{$decJson->{packs}}) {
		my ($dates, $sizes, $sortDate, $sortSize) = filesDateSizeString($ua, $pack);
		my $prettyLang = $pack->{lang};
		$prettyLang =~ s/\s*,\s*/, /g;

		print "<tr>\n";
		print "<td>" . $pack->{title} . "</td>\n";
		print "<td>" . $prettyLang . "</td>\n";
		print "<td>" . $pack->{status} . "</td>\n";
		print "<td>" . platformString($pack) . "</td>\n";
		print "<td sorttable_customkey=\"" . $sortDate . "\">" . $dates . "</td>\n";
		print "<td sorttable_customkey=\"" . $sortSize . "\">" . $sizes . "</td>\n";
		print "<td>" . urlsString($q, $pack) . "</td>\n";
		print "</tr>\n";
	}

	print $q->end_table() . "\n";
	print $q->end_html() . "\n";
}

main;
