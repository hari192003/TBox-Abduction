#**************************************************************
#* ********************************************************** *
#* *                                                        * *
#* *          MAKEFILE FOR SPASS AND FLOTTER                * *
#* *                                                        * *
#* *                                                        * *
#* *  Copyright (C) MPI fuer Informatik                     * *
#* *                                                        * *
#* *  This program is free software; you can redistribute   * *
#* *  it and/or modify it under the terms of the FreeBSD    * *
#* *  Licence.                                              * *
#* *                                                        * *
#* *  This program is distributed in the hope that it will  * *
#* *  be useful, but WITHOUT ANY WARRANTY; without even     * *
#* *  the implied warranty of MERCHANTABILITY or FITNESS    * *
#* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * * 
#* *  for more details.                                     * *
#* *                                                        * *
#* *                                                        * *
#* $Revision: 1.23 $                                        * *
#* $State: Exp $                                            * *
#* $Date: 2016-03-23 13:34:46 $                             * *
#* $Author: weidenb $                                       * *
#* *                                                        * *
#* *             Contact:                                   * *
#* *             Christoph Weidenbach                       * *
#* *             MPI fuer Informatik                        * *
#* *             Stuhlsatzenhausweg 85                      * *
#* *             66123 Saarbruecken                         * *
#* *             Email: spass@mpi-inf.mpg.de                * *
#* *             Germany                                    * *
#* *                                                        * *
#* ********************************************************** *
#**************************************************************


# $RCSfile: makefile,v $ 

SHELL = /bin/sh

# Set the Compiler flags

ifdef SPASSDEBUG
    CFLAGS := -g -fno-inline -Wno-unused # -DHM_DEBUGPRINT=1 #(to activate gcov) -fprofile-arcs -ftest-coverage 
else
   ifdef SPASSANALYZE
      CFLAGS := -O2 -pg -Wno-unused
   else
      CFLAGS := -O2 -Winline #-fprofile-use # -fprofile-generate # # 
   endif
endif

ifdef SPASSCHECK
    CFLAGS += -DCHECK=1
endif

ifdef WIN
    CFLAGS += -DWIN=1
endif

ifdef NO_MEMORY_MANAGEMENT    
    CFLAGS += -DNO_MEMORY_MANAGEMENT=1
endif

ifdef PURIFY
    ifndef NO_MEMORY_MANAGEMENT  
      CFLAGS += -DNO_MEMORY_MANAGEMENT=1
    endif
    PURIFYPROG := purify
endif

# Enables verification of Optimzed Skolemization steps

ifdef SPASSOPTCHECK
    CFLAGS += -DOPTCHECK=1
endif

# Enables catching of some common signals (segmentation fault ...)


WARNINGS = -pedantic -Wall -Wshadow -Wpointer-arith -Wwrite-strings -std=c99 #-Wconversion  
# Turn off some warnings for scanner files
SCANNERFLAGS=-Wno-implicit -Wno-uninitialized

# Currently purify does not work with gcc 2.8.*

ifndef PURIFY
   CC		:= gcc 
else
   CC		:= gcc-2.7.2.3
endif

CFLAGS		:= $(WARNINGS) $(CFLAGS)
YACC		:= bison
LEX		:= flex
RM		:= /bin/rm -f

PROGRAM		:= SPASS
TRANSLATOR      := FLOTTER
PROCONVERTER    := prolog2dfg
CONVOTTER       := dfg2otter
CONVDIMACS	:= dfg2dimacs
CONVTPTP        := dfg2tptp
CONVASCII       := dfg2ascii
CONVDFG		:= dfg2dfg
INVCONV         := tptp2dfg
DIMCONV         := dimacs2dfg
PGEN            := pgen
RESCMP          := rescmp

BASE	    	:= misc.o memory.o strings.o stack.o list.o clock.o vector.o hashmap.o symbol.o term.o context.o unify.o subst.o st.o clause.o sharing.o cmdline.o order.o kbo.o rpos.o subsumption.o condensing.o component.o defs.o ras.o eml.o description.o
CONBASE	    	:= input.o vector.o term.o symbol.o  hashmap.o strings.o list.o memory.o misc.o stack.o
CLCONBASE 	:= clause.o clauseinput.o foldfg.o order.o kbo.o rpos.o strings.o subst.o st.o unify.o context.o sharing.o flags.o vector.o term.o symbol.o  hashmap.o list.o memory.o misc.o stack.o
PROCONBASE := array.o description.o memory.o misc.o list.o stack.o symbol.o  hashmap.o strings.o term.o order.o kbo.o rpos.o foldfg.o flags.o unify.o context.o subst.o clause.o sharing.o st.o prolog2dfg.o proscanner.o proparser.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o eml.o
PROGRAMBASE     := array.o foldfg.o cnf.o resolution.o renaming.o rules-inf.o rules-split.o rules-ur.o terminator.o rules-red.o rules-sort.o analyze.o graph.o sort.o doc-proof.o search.o flags.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o tptpparser.o tptpscanner.o iaparser.o iascanner.o proofcheck.o tableau.o closure.o partition.o table.o
CONVERTERBASE   := folspass.o
CONVBASE        := array.o description.o clause.o  sharing.o subst.o strings.o st.o context.o unify.o term.o symbol.o hashmap.o stack.o list.o memory.o misc.o order.o kbo.o rpos.o flags.o foldfg.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o cmdline.o eml.o
INVCONVBASE		:= description.o tptpparser.o tptpscanner.o misc.o memory.o strings.o list.o clause.o term.o array.o symbol.o  hashmap.o stack.o flags.o foldfg.o cmdline.o sharing.o subst.o order.o context.o unify.o st.o kbo.o rpos.o  dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o eml.o
DIMCONVBASE	:= array.o dimparser.o foldfg.o flags.o cnf.o renaming.o search.o resolution.o sort.o rules-inf.o rules-red.o rules-split.o rules-sort.o rules-ur.o closure.o table.o partition.o tptpparser.o tptpscanner.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o analyze.o graph.o
PGENBASE	:= description.o cmdline.o flags.o memory.o misc.o stack.o list.o array.o symbol.o defs.o cnf.o renaming.o clock.o rules-red.o rules-inf.o rules-sort.o rules-ur.o resolution.o condensing.o closure.o ras.o table.o partition.o rules-split.o hashmap.o strings.o term.o foldfg.o clause.o search.o analyze.o graph.o sort.o sharing.o st.o subst.o unify.o context.o order.o kbo.o rpos.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o proofcheck.o tableau.o subsumption.o component.o vector.o eml.o
RESCMPBASE	:= description.o  cmdline.o flags.o misc.o list.o memory.o term.o clause.o foldfg.o array.o symbol.o hashmap.o strings.o sharing.o st.o context.o subst.o order.o rpos.o kbo.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o unify.o stack.o subsumption.o component.o vector.o eml.o
CONVDFGBASE	:= description.o memory.o misc.o stack.o list.o strings.o array.o symbol.o hashmap.o term.o foldfg.o clause.o sharing.o st.o subst.o unify.o context.o order.o kbo.o rpos.o dfg_diagnostic.o dfg_string_table.o dfg_token.o dfgparser.o dfglexer.o flags.o approx.o cmdline.o eml.o

PROGRAMMAIN     := top.o
CONVOTTERMAIN   := dfg2otter.o
CONVDIMACSMAIN  := dfg2dimacs.o
CONVTPTPMAIN    := dfg2tptp.o
CONVASCIIMAIN   := dfg2ascii.o
CONVDFGMAIN  	:= dfg2dfg.o
INVCONVMAIN	:= tptp2dfg.o
DIMCONVMAIN     := dimacs2dfg.o
PGENMAIN        := pgen.o
RESCMPMAIN      := rescmp.o

$(PROGRAM)	: $(BASE) $(PROGRAMBASE) $(PROGRAMMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(BASE) $(PROGRAMBASE) $(PROGRAMMAIN) -lm -o $(PROGRAM)

$(TRANSLATOR)	: $(PROGRAM)
	$(RM) $(TRANSLATOR)
	ln -s $(PROGRAM) $(TRANSLATOR)

$(CONVERTER)	: $(CONBASE) $(CONVERTERBASE) $(CONVERTERMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONBASE) $(CONVERTERBASE) $(CONVERTERMAIN) -lm -o $(CONVERTER)

$(CLCONVERTER)	: $(CLCONBASE) $(CLCONVERTERBASE) $(CLCONVERTERMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CLCONBASE) $(CLCONVERTERBASE) $(CLCONVERTERMAIN) -lm -o $(CLCONVERTER)

$(PROCONVERTER)	: $(PROCONBASE)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(PROCONBASE) -lm -o $(PROCONVERTER)

$(CONVOTTER)	:  $(CONVBASE) $(CONVOTTERMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONVBASE) $(CONVOTTERMAIN) -lm -o $(CONVOTTER)

$(CONVDIMACS)	:  $(CONVBASE) $(CONVDIMACSMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONVBASE) $(CONVDIMACSMAIN) -lm -o $(CONVDIMACS)

$(CONVTPTP)	:  $(CONVBASE) $(CONVTPTPMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONVBASE) $(CONVTPTPMAIN) -lm -o $(CONVTPTP)

$(CONVASCII)	:  $(CONVBASE) $(CONVASCIIMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONVBASE) $(CONVASCIIMAIN) -lm -o $(CONVASCII)

$(CONVDFG)	: $(CONVDFGBASE) $(CONVDFGMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(CONVDFGBASE) $(CONVDFGMAIN) -lm -o $(CONVDFG)

$(INVCONV)	: $(INVCONVBASE) $(INVCONVMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(INVCONVBASE) $(INVCONVMAIN) -lm -o $(INVCONV)

$(DIMCONV)	: $(DIMCONVBASE) $(DIMCONVMAIN) $(BASE)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(BASE) $(DIMCONVBASE) $(DIMCONVMAIN) -lm -o $(DIMCONV)

$(PGEN)		: $(PGENBASE) $(PGENMAIN) 
	$(PURIFYPROG) $(CC) $(CFLAGS) $(PGENBASE) $(PGENMAIN) -lm -o $(PGEN)

$(RESCMP)	: $(RESCMPBASE) $(RESCMPMAIN)
	$(PURIFYPROG) $(CC) $(CFLAGS) $(RESCMPBASE) $(RESCMPMAIN) -lm -o $(RESCMP)

scripts		: SPASS checkstat dfg2otter.pl pcs deprose filestat tpform tpget $(PROGRAM) $(CONVOTTER) $(CONVDIMACS) $(PGEN) $(RESCMP)

.PHONY		: all depend clean archive scripts tags 

all		: $(PROGRAM) $(TRANSLATOR) $(CONVERTER) $(CLCONVERTER) $(PROCONVERTER) $(CONVOTTER) $(CONVDIMACS) $(CONVTPTP) $(CONVASCII) $(INVCONV) $(DIMCONV) $(PGEN) $(RESCMP) $(CONVDFG) scripts

depend		: iaparser.c iascanner.c proparser.c proscanner.c tptpparser.c tptpscanner.c
# Only works for gcc !!
	$(RM)  .depend
	$(CC) $(CFLAGS) -MM *.c > .depend 

tags		:
	etags *.[ch]

clean		:
	$(RM) $(BASE) $(PROGRAMBASE)  $(TRANSLATORBASE) $(TRANSLATORMAIN) $(PROGRAMMAIN) $(CONBASE) $(CONVERTERBASE) $(CONVERTERMAIN) $(CLCONBASE) $(CLCONVERTERBASE) $(CLCONVERTERMAIN) $(PROGRAM) $(TRANSLATOR) $(CONVERTER) $(CLCONVERTER)  $(PROCONBASE) $(PROCONVERTER) $(CONVOTTERMAIN) $(CONVOTTER) $(CONVDIMACSMAIN) $(CONVDIMACS) $(CONVTPTPMAIN) $(CONVTPTP) $(CONVASCIIMAIN) $(CONVASCII) $(CONVDFG) $(CONVDFGBASE) $(CONVDFGMAIN) $(INVCONV) $(INVCONVBASE) $(INVCONVMAIN) $(DIMCONV) $(DIMCONVBASE) $(DIMCONVMAIN) $(PGENBASE) $(PGENMAIN) $(RESCMPBASE) $(RESCMPMAIN) $(PGEN) $(RESCMP) iaparser.c iaparser.h iascanner.c proparser.c proparser.h proscanner.c tptpparser.c tptpparser.h tptpscanner.c

opt   :  tptpscanner.c tptpparser.c iascanner.c iaparser.c
	gcc $(WARNINGS) -O3 -fwhole-program -flto array.c misc.c memory.c strings.c stack.c list.c clock.c vector.c hashmap.c symbol.c term.c context.c unify.c subst.c st.c clause.c sharing.c cmdline.c order.c kbo.c rpos.c subsumption.c condensing.c component.c defs.c ras.c eml.c description.c foldfg.c cnf.c resolution.c renaming.c rules-inf.c rules-split.c rules-ur.c terminator.c rules-red.c rules-sort.c analyze.c graph.c sort.c doc-proof.c search.c flags.c dfg_diagnostic.c dfg_string_table.c dfg_token.c dfgparser.c dfglexer.c tptpparser.c tptpscanner.c iaparser.c iascanner.c proofcheck.c tableau.c closure.c partition.c table.c top.c -lm -o $(PROGRAM)
     

archive		: clean iascanner.l iaparser.y proscanner.l proparser.y tptpscanner.l tptpparser.y
	$(RM) spass.tgz
	tar -cf spass.tar *.c *.h *.y *.l *.inc makefile LICENCE
	gzip -9 spass.tar
	mv spass.tar.gz spass.tgz

bigarchive	: clean iascanner.l iaparser.y proscanner.l proparser.y tptpscanner.l tptpparser.y
	$(RM) spass.tgz
	tar -cf spass.tar *.c *.h *.y *.l makefile pcs dfg2otter.pl checkstat LICENCE
	gzip -9 spass.tar
	mv spass.tar.gz spass.tgz

# Prepare the sources for compilation under windows visual studio/intel compiler icc plugin
winarchive		: clean  dfgscanner.c dfgparser.c  tptpscanner.c tptpparser.c  iascanner.c iaparser.c
	mkdir -p winsource
	$(RM) winsource/*
	cp *.[ch] winsource
	$(RM) winsource/*2*.c winsource/pgen.c winsource/rescmp.c
	cp winlib/*.c winsource
	cp winlib/*.h winsource
	cp winlib/*.bat winsource
	cp winlib/*.mak winsource
	perl -pi -e 's/\#include \<inttypes\.h\>//;' winsource/clock.h  
	perl -pi -e 's/\#include \<sys\/time\.h\>//;' winsource/clock.h  
	perl -pi -e 's/\#include \<unistd\.h\>//;' winsource/*.h 
	perl -pi -e 's/(\%\d*)z/$${1}l/g;' winsource/*.c
	perl -pi -e 's/\#ifndef YY_NO_UNISTD_H/\#define YY_NO_UNISTD_H\n\#ifndef YY_NO_UNISTD_H/;' winsource/*scanner.c
	perl -pi -e 's/\|\| \(Signal == SIGALRM \&\& top_NoAlarm == 0\)//;' winsource/top.c 
	perl -pi -e 's/signal\(SIGALRM, top_SigHandler\)\;//;' winsource/top.c 	
	perl -pi -e 's/alarm\(/alarm\_SetAlarm\(/;' winsource/top.c
	perl -pi -e 's/\#include \<signal.h\>/\#include \<signal.h\>\n\#include \"alarm.h\"/;' winsource/top.c


# Generate parser files from bison files
%parser.c %parser.h : %parser.y ; $(YACC) -d -p $*_ -o $*parser.c $<

# Generate scanner files from flex files
%scanner.c : %scanner.l ; $(LEX) -Ce -P$*_ -o$@ $<

# Disable some gcc warnings for scanner files
%scanner.o : %scanner.c ; $(CC) $(CFLAGS) $(SCANNERFLAGS) -c $< -o $@

# Below this line the dependencies are included.
-include .depend

