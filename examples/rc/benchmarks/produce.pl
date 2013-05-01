#!/usr/local/bin/perl
#
# To graph profile matrix
#

###############################################################################
$PLOT_DIR = "./plots";
$CHECKERS_DIR  = "./bin";
$MODELS_LIST_FILENAME = "models_list.txt";
$NUSMV_STDOUT      = "nusmv_stdout";


$GREP     = "grep";
@CHECKERS  = ("NuSMV_SIM", 
	      "NuSMV_ZCHAFF");
$MAX_K    = 15;
$MIN_K    = 0;
$K_INCR   = 1;
$LOOPBACK = "*";
###############################################################################


$ARGS=@ARGV;

if($ARGS > 0) {
    &printUsage;
    exit;
}


$K_ITERATIONS = floor(($MAX_K - $MIN_K)/$K_INCR) + 1;

#load the converted CTL formulae file:
open(MODELS_LIST, "<${MODELS_LIST_FILENAME}");


###############################################################################
# Preparation of models to be processed:
###############################################################################

#collect all the possible couples:
while ( !eof(MODELS_LIST) ) {
    $_ = <MODELS_LIST>;
    if (/^<(.+)>:$/)
    {
	$filename=$1;
	$curpos = tell(MODELS_LIST);

	# read all properties related to this file:
	$bExit = 0;
	while ( !eof(MODELS_LIST) && ($bExit==0) ) {
	    $_ = <MODELS_LIST>;
	    if (/^(\d\d\d) +CTL *:  (.*) *$/) {
		$index = int($1);
		$CTLProps[$index] = $2;
	    }
	    else {
		#continues until it found an empty line:
		$line = $_;
		chomp($line);
		if (! $line) {
		    $bExit = 1;
		}		    
	    }
	}

	# read all corresponding properties related to the ltl file:
	seek(MODELS_LIST, $curpos, 0);
	$bExit = 0;
	while ( !eof(MODELS_LIST) && ($bExit==0) ) {
	    $_ = <MODELS_LIST>;
	    if (/^(\d\d\d) +LTL *:  (.*) *$/) {
		$index = int($1);
		$LTLProps[$index] = $2;
	    }
	    else {
		$line = $_;
		chomp($line);
		if (! $line) {
		    $bExit = 1;		    
		}
	    }
	}
    
	$ltlpn = @LTLProps;
	$ctlpn = @CTLProps;
	if ($ltlpn > $ctlpn) {
	    print("\nERROR: file name=$filename has been badly interpreted\n");
	}

	splice(@aTimings, 0); #remove old results from data matrix

###############################################################################
# NuSMV session:
###############################################################################

	#invokes the solver(s) with the processed filename:
	$checker_idx = 0;
	foreach $CHECKER (@CHECKERS) {
	    ++$checker_idx;
	    print("\n\n----------  Starting checker: $CHECKER \n\n");
	    $HCHECKER = start_checker($filename, Nil);
	    print $HCHECKER "echo ******************** Starting $filename:\n";
	    initialize_checker($HCHECKER, $filename);
	    
	    for ($i=0; $i < $ctlpn; ++$i) {
		$LTL_Property = $LTLProps[$i];
		$CTL_Property = $CTLProps[$i];
					
#		generate_benchmark_bdd_ctl($HCHECKER, $filename, 
#					   $CTL_Property);
		if ($LTL_Property) {
#		    generate_benchmark_bdd_ltl($HCHECKER, $filename, 
#					       $LTL_Property);

		    #iterates for k from MIN_K to MAX_K:
#		    for($k = $MIN_K; $k<=$MAX_K; $k += $K_INCR) { 
#			generate_benchmark_bmc_pb_solve($HCHECKER, $filename, 
#							$LTL_Property, 
#							$k, $LOOPBACK);
#		    }

		    generate_benchmark_bmc_pbs_solve($HCHECKER, $filename, 
						     $LTL_Property, 
						     $MAX_K, $LOOPBACK);

#		    generate_benchmark_bmc_pbs_gen($HCHECKER, $filename, 
#						   $LTL_Property, 
#						   $MAX_K, $LOOPBACK);
		    
		}
	    } #foreach property
	    
	    quit_checker($HCHECKER, $filename);

	    #wait for really quit:
	    # print("\n\nWaiting NuSMV really quits... ");
#	    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
#	     $atime,$mtime,$ctime,$blksize,$blocks)
#		= stat($NUSMV_STDOUT);
#	    while($size == 0) {
#		sleep(2);
#		($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
#		 $atime,$mtime,$ctime,$blksize,$blocks)
#		    = stat($NUSMV_STDOUT);
#	    }
#	    # print("done!\n\n");

###############################################################################
# END OF NuSMV SESSION
###############################################################################


###############################################################################
# DATA RETRIEVING
###############################################################################
	    
	    #retrieve timings:
	    open(HTIMING, "<${NUSMV_STDOUT}");
	    	
	    ############################################################
	    # aTimings is a set of matrixes. Each matrix is has N+1 cols:
	    #   -------------------------------------------------
	    #  | k | measure1 | measure2 |   ...    |  measureN  |
	    #   -------------------------------------------------
	    # 
	    # Each matrix is contained in an external matrix, which 
	    # contains the specific benchmark whose measures are related to, 
	    # and the number of the property 
            ############################################################
	    
	    @sub_benchmarks = ("Generation", 
			       "CNF-ization", 
			       "Solving", 
			       "Dimacs_maxvar", "Dimacs_clauses"); 
	    @sub_benchmarks_prefix = ("UTIME =", "UTIME =", "UTIME =", 
				      "size = ", "size = ");
	    @sub_benchmarks_suffix = ("secs.",  "secs.",  "secs.",  
				      "", "");
	    @sub_benchmarks_incr_mode = ("incr", "incr", "incr", 
					 "not_incr", "not_incr");


	    $sub_benchmark_idx = -1;
	    foreach $sub_benchmark_name (@sub_benchmarks) {
		++$sub_benchmark_idx;

		#set the first column:
		for ($i=0; $i < $ctlpn; ++$i) {
		    $k=$MIN_K;
		    for($r = 0; $r < $K_ITERATIONS; ++$r) {
			$aTimings[$sub_benchmark_idx][$i][$r][0] = $k;
			$k += $K_INCR;
		    }
		}

		#retrieve BMC LTL timing:
		seek(HTIMING, 0, 0);
		for ($i=0; $i < $ctlpn; ++$i) {
		    $prev_measure = 0; # to keep incremental measurements
		    for($r = 0; $r < $K_ITERATIONS; ++$r) {
			if ($LTLProps[$i]) {
			    #formula is ok
			    $pref = $sub_benchmarks_prefix[$sub_benchmark_idx];
			    $suff = $sub_benchmarks_suffix[$sub_benchmark_idx];
			    $measure = retrieve_benchmark(
				     HTIMING, 
				     $sub_benchmark_name, 
				     $pref,
				     $suff
							 );
			    # incremental measurements:
			    if ($sub_benchmarks_incr_mode[$sub_benchmark_idx] 
				== "incr") {
				$measure = $measure + $prev_measure;
				$prev_measure = $measure
			    }

			    # stores the read value:
			    $aTimings[$sub_benchmark_idx][$i][$r]
				[$checker_idx] = $measure;
			} #LTL formula was successfully converted
			else {
			    $aTimings[$sub_benchmark_idx][$i][$r]
				[$checker_idx] = -1;
			}
		    } #for r
		} #for i (property)    


	    } #for each sub-benchmark 

	    close(HTIMING);
	} #foreach checkers


###############################################################################
# INFORMATION DUMPING
###############################################################################
	
	#here all timings have been collected:
        
       $sub_benchmark_idx = -1;
       foreach $sub_benchmark_name (@sub_benchmarks) {
	   ++$sub_benchmark_idx;
	   
	   for ($i=0; $i < $ctlpn; ++$i) {
	       $index = rindex($filename, "/");
	       $str = substr($filename, $index + 1);
	       $dat_filename = 
		   "${PLOT_DIR}/${str}__${sub_benchmark_name}__prop${i}";
	       
	       @col_names = ("k", "SIM", "ZCHAFF"
			    );
	       $graph_title = 
		   "Benchmarking of ${filename}: for ${sub_benchmark_name}";

	       writePlotData( $dat_filename,$graph_title, 
			      $sub_benchmark_suffix[$sub_benchmark_idx], 
			      \@col_names, 
			      \@{$aTimings[$sub_benchmark_idx][$i]} );
           }
        }
	    
	splice(@LTLProps, 0);
	splice(@CTLProps, 0);
	
    } # file name recognized
}


close(LTL_LIST);
close(CTL_LIST);

###############################################################################
# END OF MAIN
###############################################################################



###############################################################################
# SUBROUTINES
###############################################################################

sub writePlotData #($filename, $graph_title, $Yunit, \@col_names, \@data_matrix)
{
    my($filename, $graph_title, $Yunit, $col_names_ref, $data_matrix_ref) = @_;
    my($i, $r, $rows, $cols, $temp);

    open(GNUPLOT_DATA, ">${filename}.dat");
    print GNUPLOT_DATA 
	"# GnuPlot data for benchmarking\n";
    
    #writes out columns names: 
    $cols = @$col_names_ref;
    print GNUPLOT_DATA "# ======================================================================\n";
    print GNUPLOT_DATA "# Columns names:\n";
    print GNUPLOT_DATA "# ----------------------------------------------------------------------\n";
    print GNUPLOT_DATA "# |";
    for($i=0; $i < $cols; ++$i) {
	print GNUPLOT_DATA " @$col_names_ref[$i] |";
    }
    print GNUPLOT_DATA "\n# ======================================================================\n";    
    print GNUPLOT_DATA "\n\n";	
       
    
    #writes out rows:
    $rows = @$data_matrix_ref; 
    for($r = 0; $r <= $rows; ++$r) {
	#prints out the row:
	for($i=0; $i < $cols; ++$i) {
	    print GNUPLOT_DATA "$data_matrix_ref->[$r][$i] ";
	}
	print GNUPLOT_DATA "\n";
    }

    close(GNUPLOT_DATA);
    print("GNUPLOT data written out to $filename\n");



    ######################################################################
    # Generates ps graphic file:
    #
    open(GNUPLOT_CMD, ">${filename}.cmd");
     
    print GNUPLOT_CMD "reset\n";
    print GNUPLOT_CMD "set title \"${graph_title}\"\n";
#    print GNUPLOT_CMD "set data style linespoints\n";
    print GNUPLOT_CMD "set size 1.4,1.9\n";
    print GNUPLOT_CMD "set xlabel  \"@$col_names_ref[0]\"\n";
    print GNUPLOT_CMD "set ylabel \"benchmark measure [$Yunit]\"\n";
    print GNUPLOT_CMD "set pointsize 0.2\n";
    print GNUPLOT_CMD "set terminal postscript eps enhanced color 22\n";
    print GNUPLOT_CMD "set key top left\n";
    print GNUPLOT_CMD "show title\n";
    print GNUPLOT_CMD "set output '${filename}.ps'\n";
    print GNUPLOT_CMD "set xrange [${MIN_K}:${MAX_K}]\n";
    
    print GNUPLOT_CMD "plot";
    
    for($i=1; $i < $cols; ++$i) {
	$temp = $i+1; 
	print GNUPLOT_CMD "  '${filename}.dat' using 1:${temp} title \"@$col_names_ref[$i]\" with lp lw 3";
	if (($i+1) < $cols) {
	    #not the last:
	    print GNUPLOT_CMD ", ";
	}
    }
    print GNUPLOT_CMD "\n\n";
    close(GNUPLOT_CMD);

    #invokes gnuplot:
    open(GNUPLOT, "|gnuplot ${filename}.cmd");
    close(GNUPLOT);
    print("Generated the postscript graphic in ${filename}.ps\n");
}


# params: checker_handler, modelname, ltl_formula, k, l
# rets: nothing
sub generate_benchmark_bmc_pb_solve
{
    local($handler, $modelname, $ltl_formula, $k, $l) = @_;
 
    print $handler "set bmc_length $k \n";
    print $handler "set bmc_loopback $l \n";
    print $handler "bmc_pb_solve \"${ltl_formula}\" \n";
}


# params: checker_handler, modelname, ltl_formula, k, l
# rets: nothing
sub generate_benchmark_bmc_pbs_solve
{
    local($handler, $modelname, $ltl_formula, $k, $l) = @_;
 
#    print $handler "set bmc_length $k \n";
#    print $handler "set bmc_loopback $l \n";
#    print $handler "bmc_pbs_solve \"${ltl_formula}\" \n";
    print $handler "check_ltlspec_bmc -k ${k} -l ${l} -p \"${ltl_formula}\" \n";
}


# params: checker_handler, modelname, ltl_formula, k, l
# rets: nothing
sub generate_benchmark_bmc_pbs_gen
{
    local($handler, $modelname, $ltl_formula, $k, $l) = @_;
 
    print $handler "set bmc_length $k \n";
    print $handler "set bmc_loopback $l \n";
    print $handler "bmc_pbs_gen \"${ltl_formula}\" \n";
}


# params: checker_handler, modelname, ltl_formula, k, l
# rets: nothing
sub generate_benchmark_bmc_gen
{
    local($handler, $modelname, $ltl_formula, $k, $l) = @_;
    print $handler "set bmc_length $k \n";
    print $handler "set bmc_loopback $l \n";
    print $handler "bmc_pb_gen \"${ltl_formula}\" \n";
    print $handler "bmc_pb_gen -s\n";    
}


# params: checker_handler, modelname, ctl_formula
# rets: nothing
sub generate_benchmark_bdd_ctl
{
    local($handler, $modelname, $ctl_formula) = @_;
    print $handler "echo   **   BDD CHECKING OF CTL ${ctl_formula}\n";
    print $handler "check_spec \"${ctl_formula}\" \n";
    print $handler "echo   **   CTL FORMULA CHECKED BY BDD ; echo\n";
}


# params: checker_handler, modelname, ltl_formula
# rets: nothing
sub generate_benchmark_bdd_ltl
{
    local($handler, $modelname, $ltl_formula) = @_;
    print $handler "echo   **   BDD CHECKING OF LTL ${ctl_formula}\n";
    print $handler "check_ltlspec \"${ltl_formula}\" \n";
    print $handler "echo   **   LTL FORMULA CHECKED BY BDD ; echo\n";
}


# Starts model checker, using the given model and the corresponding 
# order file if present
# params: modelname, [sourcename]
# ret: stdin handler of the solver
sub start_checker
{
    local($modelname, $sourcename) = @_;
    local($ordername, $index, $ord_opt);
        
    #search for the ordering filename:
    $ordername = $filename;
    $index = rindex($filename, ".smv");    
    if ($index >= 0) {
	$ordername = substr($modelname, 0, $index); 
    }   

    $ordername = "${ordername}.ord";
    ${ord_opt} = "-i ${ordername}";
    #checks if ordering file exists:
    unless (open(ORDERING_FILE, "<${ordername}")) {
	${ord_opt} = "";
        close(ORDERING_FILE);
    }
    
    if (${ord_opt}) {
        print "\nFound the order file name <${ordername}>\n";
    }
    else {
        print "\nThe order file has not been found\n";    
    }

    if ($sourcename != Nil) {
        open(CHECKER_HANDLER, 
	     "|${CHECKERS_DIR}/${CHECKER} -int ${ord_opt} -load ${sourcename} ${modelname}");
    }
    else {
	open(CHECKER_HANDLER, 
	     "|${CHECKERS_DIR}/${CHECKER} -int ${ord_opt} ${modelname}");
    }
    CHECKER_HANDLER;
}


# params: checker_handler, modelname
sub quit_checker
{
    local($handler, $modelname) = @_;
    print $handler "quit\n";
    close($handler);
}


# params: checker_handler, modelname
sub initialize_checker
{
    local($handler, $modelname) = @_;
#    print $handler "set verbose_level 1 \n";
#    print $handler "read_model \n";
#    print $handler "flatten_hierarchy \n";
#    print $handler "encode_variables \n";
#    print $handler "build_model\n";
#    print $handler "compute_reachable \n";
#    print $handler "bmc_setup \n";
    print $handler "go_bmc \n";

    if (${NUSMV_STDOUT}) {
      print $handler "set nusmv_stdout ${NUSMV_STDOUT}\n";
    }

    print $handler "echo Working on ${modelname}\n";

}



# params: (fileHandle, string to search)
# ret: the line string in which search string has been found, "" otherwise
sub fileSearchStr 
{
    local($FILE, $STR) = @_; 
    local($line, $ret, $res );
    $ret = "";
    while ( (!eof($FILE)) && ($ret eq '') ) {
	$line = <$FILE>; 
	chomp($line);
	if ( index($line, $STR, 0) >= 0 ) {
	    $ret = $line;
	}	
    }
    $ret;
}



sub printUsage
{
    print "You supplied ${ARGS} arguments on 8 required\n";
    print "\n";    
}


# retrieves a specific benchmark reading it from the specified file. 
# The specific benchmark name is also a parameter.  
# Parameters: ($HFILE, $bench_name, $prefix, $suffix)
# Returns the measure.  
sub retrieve_benchmark
{
    local($HFILE, $bench_name, $prefix, $suffix) = @_;  
    local($line, $end, $ret);

    $line = fileSearchStr($HFILE, ":START:benchmarking ${bench_name}");
    chomp($line);
    if (!$line) {
	print("ERROR: benchmark of ${bench_name} has not been found!\n");
	exit(1);
    }

    # read measure: 
    $line = <$HFILE>; #reads the next line
    chomp($line);
    if (!$line) {
	print("ERROR: timing of <${bench_name}> has not been found!\n");
	exit(1);
    }	    
    
    if ($suffix) {
	$end = index($line, $suffix);
	if ($end == -1) {
	    print("\nERROR: timing is invalid in benchmark <${bench_name}>\n");
	    print("read time string='${line}', did not find ${suffix}\n");
	    exit(1);
	}
	$ret = substr($line, length($prefix)+1, $end - length($line));
    }
    else {
	$ret = substr($line, length($prefix)+1);
    }

    return $ret;    
}


# retrieves a specific statistic benchmark reading it from the specified file. 
# The specific benchmark name is also a parameter.  
# Parameters: ($HFILE, $bench_name)
# Returns the time.  
sub retrieve_statistics
{
    my($HFILE, $bench_name) = @_;  
    my($line, $end, $data, $bExit);
    my(@parsed) = ();
    my(@stat_info) = ();

    $line = fileSearchStr($HFILE, ":START:statistics ${bench_name}");
    chomp($line);
    if (!$line) {
	print("ERROR: benchmark of ${bench_name} has not been found!\n");
	exit(1);
    }
    
    $bExit = 0;
    while( !eof($HFILE) && ($bExit == 0) ) {
	# read information: 
	$line = <$HFILE>; #reads the next line
	chomp($line);
	if (!$line) {
	    print("ERROR: statistics of <${bench_name}> are invalid\n");
	    exit(1);
	}	   
	if ($line ne ":END:statistics ${bench_name}") {
	    @parsed = split(/[ \t]+/, $line);
	    $data = $parsed[@parsed-1]; # the last word contains information
	    push(@stat_info, $data);
	}
	else {
	    $bExit=1;
	}
    } #while
	   
    return @stat_info;
}


sub floor
{
    local($n) = @_;
    local($ip);

    $ip = int($n);

    if (($n > 0) | ($n == $ip)) {
	return $ip;
    }
    else {
	return $ip-1;
    }
}


sub ceil
{
    local($n) = @_;
    local($ip);

    $ip = int($n);

    if ($ip == $n) {
	return $n;
    }
    else {
	return $ip+1;
    }
}

sub round
{
    local($n) = @_;
    local($ip, $fp, $incr);

    $ip = int($n);
    $fp = abs($n - $ip);
    
    if ($n>0) {
	$incr = 1;
    }
    else {
	$incr = -1;
    }

    if ($fp > 0.5) {
	return $ip+$incr;
    }
    else {
	return $ip;
    }
}



