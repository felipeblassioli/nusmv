#!/usr/local/bin/perl
#
# To generate profile matrix
#
###############################################################################
$DEFAULT_LOOPBACK=0;
$MODEL_NAME = "model_path.smv";
$CHECKER = "./NuSMV_no_path_opt";
$PROFILER = "gprof";
$PROFILER_OPTS = "--no-graph --display-unused-functions";
$DEST_DIR = "./results_no_path_opt";
###############################################################################


$ARGS=@ARGV;

if($ARGS < 4 || $ARGS>7) {
    print "You supplied ${ARGS} arguments on 4 required.\n";
    print "Usage: gen.perl <K_MIN> <K_MAX> <PBS_MIN> <PBS_MAX> [<K_INCR> <PB_INCR> <LOOP>]\n";
    exit;
}

#args not checked!
$MIN_K   = $ARGV[0];
$MAX_K   = $ARGV[1];
$MIN_PBS = $ARGV[2];
$MAX_PBS = $ARGV[3];

$K_INCR = 1;
$PB_INCR= 1;
$LOOP=$DEFAULT_LOOPBACK;

if($ARGS >= 5 ) {
    $K_INCR = $ARGV[4];
    if($ARGS >= 6 ) {
	$PB_INCR= $ARGV[5];
	if($ARGS >= 7) {
	    $LOOP   = $ARGV[6];
	}
    }
}

for($k=$MIN_K; $k <= $MAX_K; $k+=$K_INCR) {
    for ($pbs=$MIN_PBS; $pbs <= $MAX_PBS; $pbs+=$PB_INCR) {
	print("\n\n******** Generating profile for k=$k, pbs=$pbs ********\n");
	open(NUSMV, "|${CHECKER} -int");
	
	#exec nusmv to generate profiling:
	print NUSMV  "read_model -i ${MODEL_NAME}\n";
	print NUSMV  "go\n";
	print NUSMV  "bmc_setup\n";
	print NUSMV  "set bmc_length ${k}\n";
	print NUSMV  "set bmc_loopback ${LOOP}\n";
	
	for($p=0; $p < $pbs; ++$p) {
	    print NUSMV "bmc_pb_gen -n 0\n";
	}
	
	print NUSMV  "quit\n";	
	close(NUSMV);

	#generates profile:
	open(PROFILER,
	     "|${PROFILER} ${PROFILER_OPTS} ${CHECKER} >${DEST_DIR}/profile_${k}_${pbs}.txt");
	close(PROFILER);
    }  
}





