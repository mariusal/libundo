#!/usr/local/bin/perl

$main::srcdir = undef;

&main;

sub usage {
    print STDERR "$0:  Use '$0 <testfile>'\n";
    exit 1;
}

sub uniq {
    my ($list) = @_;
    my %hash;
    my $ret = [];

    my $item;
    foreach $item (@$list) {
	if(!defined($hash{$item})) {
	    push @$ret, $item;
	    $hash{$item} = 1;
	}
    }
 
    return $ret;
}

sub parse_tests {
    my ($testfile) = @_;
    my @ret;
    my $test;
    my %name_link;

    @ret = ();

    if(!open(TESTFILE, "$testfile")) {
	print STDERR "$0: Can't open '$testfile'\n";
	exit 1;
    }
    my @testlines = <TESTFILE>;
    close(TESTFILE);

    my $linenum = 0;
    while(@testlines) {
	my $line = shift @testlines;
	$linenum++;

	if($line =~ /^~~(.*)/) {
	    my $label;
	    my $link = [];
	    $line = $1;

	    if($line =~ /(.*):(.*)/) {
		my $label_part = $1;
		$line = $2;

		if($label_part =~ /[a-zA-Z0-9_.]+/) {
		    $label = $&;
		}
	    }

	    while($line =~ /[*a-zA-Z0-9_.\/]+/) {
		my $item = $&;
		$line = $';

		if($item =~ /^\*(.*)/) {
		    push @$link, $1;
		} else {
		    #  link to past
		    if(!defined($namelink{$item})) {
			print "No previous test '$item'\n";
			exit 1;
		    }
		    push @$link, @{$namelink{$item}};
		}
	    }

	    if(!defined($label)) {
		if(!defined($test)) {
		    print STDERR "$0: Need initial test name\n";
		    exit 1;
		}
		$label = $test->{'label'};
		
		if($label =~ /\#([0-9]+)/) {
		    $label = $` . '#' . ($1 + 1);
	        } else {
		    $label = $label . ' #2';
		}
	    }

	    $link = uniq($link);

	    if(!@$link) {
		if(!defined($test)) {
		    print STDERR "$0: Need initial link files\n";
		    exit 1;
		}
		$link = $test->{'link'};
	    }

	    $namelink{$label} = $link;
	    $test = { 'label' => $label, 'link' => $link, 'text' => '',
		      'filename' => $testfile, 'line' => $linenum + 1 };
	    push @ret, $test;
	} elsif($line =~ /^~(.*)/) {
	    print STDERR "$0: What? ('$1')\n";
	    exit 1;
	} else {
	    if(!defined($test)) {
		print STDERR "$0: Need initial ~~ line\n";
		exit 1;
	    }

	    $test->{'text'} .= $line;
	}
    }

    return @ret;
}

sub perform_test {
    my ($test) = @_;

    $| = 1;

    my $ln = '  ' . $test->{'label'};
    while(length($ln) < 38) {
	$ln .= ' ';
    }
    
    print "$ln  ";

    open(TMPC, ">/tmp/test.$$.c");
    print TMPC "# $test->{'line'} \"$test->{'filename'}\"\n";
    print TMPC $test->{'text'};
    close(TMPC);

    my $val = system("cc -g -I$main::srcdir/test -I$main::srcdir/include -I$main::srcdir/src /tmp/test.$$.c @{$test->{'link'}} -o /tmp/test.$$");
    system("rm -f /tmp/test.$$.c");

    if($val) {
	print "Failure to compile\n";
	exit(1);
    }

    $val = system("/tmp/test.$$");
    if($val) {
	system("mv -f /tmp/test.$$ ./failed");
	print "Failure\n";
	exit(1);
    }
    system("rm -f /tmp/test.$$");

    print "Ok\n";
}

sub main {
    my $testfile = shift @ARGV;
    $main::srcdir = shift @ARGV;

    usage()  if(!defined($testfile));
    
    my @test = parse_tests($testfile);
    my $test;

    foreach $test (@test) {
	perform_test($test);
    }

    exit(0);
}
