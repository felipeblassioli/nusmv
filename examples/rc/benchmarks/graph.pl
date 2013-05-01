#!/usr/local/bin/perl
#
# To graph profile matrix
#

###############################################################################
$DEST_DIR = "./results";
$PLOT_DIR = "./plots";
$GREP     = "grep";
###############################################################################


$ARGS=@ARGV;

if($ARGS < 8) {
    &printUsage;
    exit;
}

$OPT_VIEW=-1;
%hash_opt = (
	 '-local_time' => 0,
	 '-local_perc' => 1,
	 '-tot_time'   => 2,
	 '-tot_perc'   => 3,
	 '-calls'      => 4,
	 '-avg_time'   => 5
	);

if ( exists($hash_opt{$ARGV[0]}) ) {
    $OPT_VIEW = $hash_opt{$ARGV[0]};
}
	       

if ($OPT_VIEW<0) {
    print("Error: $ARGV[0] is a <what> option invalid value\n");  
    &printUsage;
    exit;
}

#args not checked!
$MIN_K   = $ARGV[1];
$MAX_K   = $ARGV[2];
$K_INCR = $ARGV[3];
$MIN_PBS = $ARGV[4];
$MAX_PBS = $ARGV[5];
$PB_INCR= $ARGV[6];    
    

@SYMBOLS=@ARGV;
splice(@SYMBOLS, 0, 7);

for($k=$MIN_K; $k <= $MAX_K; $k+=$K_INCR) {
    for ($pbs=$MIN_PBS; $pbs <= $MAX_PBS; $pbs+=$PB_INCR) {
	$filename = "$DEST_DIR/profile_${k}_${pbs}.txt";
	open(PROFILE,"<${filename}")
	    or die "Error: file not found: ${filename}\n";
	
	for($sym=0; $sym < @SYMBOLS; ++$sym) {
	    seek(PROFILE, 0, 0); #start from beginning for each symbol: 
	    $line=&fileSearchStr(PROFILE, $SYMBOLS[$sym]);

	    if (! $line) {
		print("Error: symbol <$SYMBOLS[$sym]> was not found in ${filename}.\n");
		exit;
	    }
	    #recover symbol data:
	    @data = split(/\s+/, $line);
	    
	    #save all data:
	    $info[$k][$pbs][$sym]=[@data];
	}

	close(PROFILE);
    } #for pbs 
} #for k

#generates the gnuplot data:
for($sym=0; $sym < @SYMBOLS; ++$sym) {
    $data_fname="${PLOT_DIR}/gnuplot_${SYMBOLS[$sym]}.dat";
    open(GNUPLOT_DATA, ">$data_fname");
    print GNUPLOT_DATA "# GnuPlot data for $ARGV[0] statistics on ${SYMBOLS[$sym]}\n";
    print GNUPLOT_DATA "#k pbs=${MIN_PBS} ... pbs=${MAX_PBS}\n\n";
    for($k=$MIN_K; $k <= $MAX_K; $k+=$K_INCR) {
	print GNUPLOT_DATA "$k ";
	for ($pbs=$MIN_PBS; $pbs <= $MAX_PBS; $pbs+=$PB_INCR) {
	    print GNUPLOT_DATA "${info[$k][$pbs][$sym][$OPT_VIEW]} "; 
	    } #for pbs
	print GNUPLOT_DATA "\n";
    } #for k
    close(GNUPLOT_DATA);
} #for sym


#generates gnuplot cmd script:
$cmd_fname="${PLOT_DIR}/gnuplot.cmd";
open(GNUPLOT_CMD, ">$cmd_fname");

for($sym=0; $sym < @SYMBOLS; ++$sym) {
    $data_fname="${PLOT_DIR}/gnuplot_${SYMBOLS[$sym]}.dat";
    $ps_fname ="${PLOT_DIR}/gnuplot_${SYMBOLS[$sym]}$ARGV[0].ps";

    print GNUPLOT_CMD "reset\n";
    print GNUPLOT_CMD "set title \"${SYMBOLS[$sym]}\"\n";
#    print GNUPLOT_CMD "set data style linespoints\n";
    print GNUPLOT_CMD "set size 1.4,1.9\n";
    print GNUPLOT_CMD "set xlabel  \"k (problem length)\"\n";
    print GNUPLOT_CMD "set ylabel \"function $ARGV[0]\"\n";
    print GNUPLOT_CMD "set pointsize 0.2\n";
    print GNUPLOT_CMD "set terminal postscript eps enhanced color 22\n";
    print GNUPLOT_CMD "set key top left\n";
    print GNUPLOT_CMD "show title\n";
    print GNUPLOT_CMD "set output '${ps_fname}'\n";
    print GNUPLOT_CMD "set xrange [${MIN_K}:${MAX_K}]\n";
    
    $col=2;
    print GNUPLOT_CMD "plot";
    for ($pbs=$MIN_PBS; $pbs <= $MAX_PBS; $pbs+=$PB_INCR) {
	print GNUPLOT_CMD "  '${data_fname}' using 1:${col} title \"pbs=${pbs}\" with lp lw 3";
	if (($pbs+$PB_INCR) <= $MAX_PBS) {
	    #not the last:
	    print GNUPLOT_CMD ", ";
	}
	++$col;
    }
    print GNUPLOT_CMD "\n\n";
} #for sym

#invokes gnuplot:


close(GNUPLOT_CMD);


# end !
###############################################################################




 
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
    print "Usage: graph.perl <what> <K_MIN> <K_MAX> <K_INCR> <PBS_MIN> <PBS_MAX> <PB_INCR> <SYMBOL> [<SYMBOL> ...]\n";
    print " Valid values for <what> are: \n";
    print "-local_time: time spent in the function without its children\n";
    print "-local_perc: percentage of local_time on program total time\n";
    print "-tot_time: time spent in the function with its children\n";
    print "-tot_perc: percentage of tot_time on program total time\n";
    print "-calls: number of calls for each function\n";
    print "-avg_time: tot_time / calls\n";
    print "-error: given max and min times for each function, calculates the average error\n";
    print "\n";    
}




