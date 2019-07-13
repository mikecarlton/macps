/*
 * Copyright (c) 1988, The Regents of the University of California.
 * Edward Moy, Workstation Software Support Group, Workstation Support Serices,
 * Information Systems and Technology.
 *
 * Permission is granted to any individual or institution to use, copy,
 * or redistribute this software so long as it is not sold for profit,
 * provided that this notice and the original copyright notices are
 * retained.  The University of California makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef lint
static char *SCCSid = "@(#)macps.c	2.2 10/25/89";
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "str.h"
#include "ucbwhich.h"

#define	CONFIG		"macps.config"
#ifdef SYSV
#define	index		strchr
#define	rindex		strrchr
#endif

#ifdef SAVE
char *finale = "clear countdictstack 2 sub{end}repeat macps restore\n";
char intro[] = "\
%%! *** Created by macps: %s\
/macps save def\n\
";
#else
char intro[] = "\
%%! *** Created by macps: %s\
";
#endif
char *myname;
int ncopies = 0;
#ifdef CONFIGDIR
char ucblib[UCBMAXPATHLEN] = CONFIGDIR;
#else
int ucbalternate;
char ucbpath[UCBMAXPATHLEN];
char ucblib[UCBMAXPATHLEN];
#endif

void Usage();

int main(argc, argv)
int argc;
char **argv;
{
	register STR* str;
	register char *cp, *pp;
	register FILE *fp;
	register int i, fd;
	char line[BUFSIZ];
	char path[UCBMAXPATHLEN];
	long ltime;
	char *ctime();

#ifndef CONFIGDIR
	ucbwhich(*argv);
#endif
	if((myname = rindex(*argv, '/')))
		myname++;
	else
		myname = *argv;
	cp = NULL;
	for(argc--, argv++ ; argc > 0 && **argv == '-' ; argc--, argv++) {
		switch((*argv)[1]) {
		 case 'c':	/* multiple copies */
			if((*argv)[2])
				ncopies = atoi(&(*argv[2]));
			else {
				if(argc < 2)
					Usage();	/* never returns */
				argc--;
				ncopies = atoi(*++argv);
			}
			if(ncopies <= 0)
				Usage();	/* never returns */
			break;
		 case 'd':	/* alternate directory for config file */
			if((*argv)[2])
				cp = &(*argv[2]);
			else {
				if(argc < 2)
					Usage();	/* never returns */
				argc--;
				cp = *++argv;
			}
			strcpy(ucblib, cp);
			break;
		 case 'r':	/* raw mode */
			rawmode++;
			break;
		 default:
			Usage();	/* never returns */
		}
	}
	if(argc > 1)
		Usage();	/* never returns */
	if(argc == 1 && freopen(*argv, "r", stdin) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", myname, *argv);
		exit(1);
	}
	str = STRalloc();
	if(!STRgets(str, stdin)) {
		fprintf(stderr, "%s: Null input\n", myname);
		exit(1);
	}
	strcat(ucblib, "/");
	strcpy(path, ucblib);
	strcat(path, CONFIG);
	if((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "%s: Can't open %s\n", myname, path);
		exit(1);
	}
	time(&ltime);
	printf(intro, ctime(&ltime));
	do {
		if(ncopies != 0 && STRheadcompare(str, "userdict /#copies ")
		 == 0)
			continue;
		if(STRcompare(str, "%%EOF\n") == 0) {
#ifdef SAVE
			if(finale) {
				fputs(finale, stdout);
				finale = NULL;
			}
#endif
			STRputs(str, stdout);
			continue;
		}
		if(STRheadcompare(str, "%%IncludeProcSet:") == 0) {
			for(cp = (char *)&str->bufptr[17] ; ; cp++) {
				if(!*cp) {
					fprintf(stderr,
				 "%s: Syntax error on IncludeProcSet line\n",
					 myname);
					exit(1);
				}
				if(!isascii(*cp) || !isspace(*cp))
					break;
			}
			pp = (char *)str->curendptr;
			while(--pp >= cp) {
				if(!isascii(*pp) || !isspace(*pp))
					break;
				*pp = 0;
			}
			str->curendptr = (unsigned char *)(pp + 1);
			fseek(fp, 0L, 0);
			for( ; ; ) {
				if(!fgets(line, BUFSIZ, fp)) {
					fprintf(stderr,
					 "%s: Unknown IncludeProcSet %s\n",
					 myname, cp);
					exit(1);
				}
				if(*line == '#')
					continue;
				if((pp = index(line, '\n'))) {
					if(pp == line)
						continue;
					*pp = 0;
				}
				if(!(pp = index(line, '\t'))) {
					fprintf(stderr,
					 "%s: Syntax error in macps.config\n",
					 myname);
					exit(1);
				}
				*pp++ = 0;
				if(STRcompareptr(str, cp, line) == 0)
					break;
			}
			if(*pp == '/')
				strcpy(path, pp);
			else {
				strcpy(path, ucblib);
				strcat(path, pp);
			}
			fflush(stdout);
			if((fd = open(path, O_RDONLY, 0)) < 0) {
				fprintf(stderr, "%s: Can't open %s\n", myname,
				 path);
				exit(1);
			}
			while((i = read(fd, line, BUFSIZ)) > 0)
				write(1, line, i);
			close(fd);
			continue;
		}
		STRputs(str, stdout);
		if(ncopies > 1 && isascii(*str->bufptr) &&
		 isdigit(*str->bufptr)) {
			cp = (char *)str->bufptr;
			while(cp < (char *)str->curendptr && isascii(*cp)
			 && isdigit(*cp))
				cp++;
			if((char *)str->curendptr - cp == 4 &&
			 STRcompareptr(str, cp, " mf\n") == 0) {
				printf("userdict /#copies %d put\n", ncopies);
				ncopies = -1;
			}
		}
	} while(STRgets(str, stdin));
#ifdef SAVE
	if(finale)
		fputs(finale, stdout);
#endif
	exit(0);
}

void
Usage()
{
	fputs("Usage: macps [-c #] [-d directory] [-r] [file]\n", stderr);
	exit(1);
}
