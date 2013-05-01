#!/usr/local/bin/perl
#
# To extract all SPECS from files in a directory
#
###############################################################################
$DEFAULT_LOOPBACK=0;
$CHECKER = "NuSMV";
$DEST_DIR = "./results_no_path_opt";
###############################################################################


$ARGS=@ARGV;

if($ARGS != 1) {
    print "You supplied ${ARGS} arguments on 1 required.\n";
    print "Usage: extract.pl <search_root_dir>\n";
    exit;
}

#args not checked!
$ROOT_DIR = $ARGV[0];
$FIND_OUT = "find.out";
$NUSMV_OUT = "nusmv.out";

#finds all smv files:
open(FIND, "|find ${ROOT_DIR} -name '*.smv' > ${FIND_OUT}");
close(FIND);

#creates the list of input files
open(SMV_LIST, "<${FIND_OUT}");
@fileList = <SMV_LIST>;
close(SMV_LIST);

open(CTL_LIST, ">ctl_list.txt");
open(LTL_LIST, ">ltl_list.txt");

#extract all SPECS from any file in the list:
foreach $file (@fileList) {
    $file_name = $file;
    chomp $file_name;
    print "\n************* Parsing ${file_name} **************\n";
    
    open(NUSMV, "|${CHECKER} -int");
    print NUSMV  "read_model -i ${file_name}\n";
    print NUSMV  "flatten_hierarchy\n";
    print NUSMV  "set nusmv_stdout ${NUSMV_OUT}\n";
    print NUSMV  "show_property\n";
    print NUSMV  "quit\n";
    close(NUSMV);

    open(PROP_LIST, "<${NUSMV_OUT}");
    @dirty_list = <PROP_LIST>;
    close(PROP_LIST);

    #clean result
    @dirty_list = grep(!/^\*\*\*\* PROPERTY LIST /, @dirty_list);
    @loc_prop_list = grep(!/^-/, @dirty_list);
    
    $list_length = @loc_prop_list; 
    splice(@CTL_formulae, 0);
    splice(@LTL_formulae, 0);
    for ($i=0; $i<${list_length}; ++$i) {
        if (@loc_prop_list[$i] =~ /\s+\[CTL/) {
	    push(@CTL_formulae, @loc_prop_list[$i-1]);
	}
	if (@loc_prop_list[$i] =~ /\s+\[LTL/) {
	    push(@LTL_formulae, @loc_prop_list[$i-1]);
	}
    }

    $temp = @CTL_formulae;
    if ($temp>0) {
	print CTL_LIST "\n<$file_name>:\n";
	print CTL_LIST @CTL_formulae;
    }
    
    $temp = @LTL_formulae;
    if ($temp>0) {
	print LTL_LIST "\n<$file_name>:\n";
	print LTL_LIST @LTL_formulae;
    }
}

close(CTL_LIST);
close(LTL_LIST);


