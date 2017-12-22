/* ------------------------------------------------------------ */
/* db2-connect-memory-test: Brandl Wolfgang                     */
/* Bundesrechenzentrum,    Vienna. 2016-09-08                   */
/* ------------------------------------------------------------ */
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>



static   FILE      *pfFileI;
static   FILE      *pfFileO;
static   FILE      *pfFileS;
static   char      szInputfilename[128];
static   char      szOutputfilename[128];
static   char      szSourceFilename[128];

static   int       iCase;
char               bufsqlacall[] = "sqlacall((unsigned short)";

/*********************************************************************************************/
/*********************************************************************************************/
int help_do ()
{
  printf("Usage: addstmt -i<inputfile name> -s<source C program> -o<output file name>] \n\n");
  return EXIT_FAILURE;
}

/******************************************************************
* Main
*******************************************************************/
int main(int argc, char *argv[])
{
  int                        iCount;
  char                       szLine[400];
  char                       szLine2[400];
  char                       szTok[400];
  char                      *findstring;
  int                        iStatementnumber;
  int                        iStatementtype;

  char                       szSearchbuffer[200];
  char                       szExecStatement[200];
  char                       szExecLine[200];
  char                       szResultLine[200];
  char                       szPackage[200];
  char                      *szFreeline;
  char                      *ptr;
  int                        iTokcount;
  char                      *s;
  ssize_t                    iReturnedchar;



  memset (szInputfilename,0x00,sizeof(szInputfilename));
  memset (szOutputfilename,0x00,sizeof(szOutputfilename));
  memset (szSourceFilename,0x00,sizeof(szSourceFilename));
  for (iCount = 1; iCount < argc; iCount++) {
    switch (*(argv[iCount]+1)) {
      case '?':
      case 'h':
      case 'H': help_do(); break;
      case 'i':
      case 'I': strcpy (szInputfilename,argv[iCount]+2); break;
      case 's':
      case 'S': strcpy (szSourceFilename,argv[iCount]+2); break;
      case 'o':
      case 'O': strcpy (szOutputfilename,argv[iCount]+2); break;
      default:
        help_do();
        return EXIT_FAILURE;
    }
  }
  if (strlen(szInputfilename) == 0 || strlen(szOutputfilename) == 0 || strlen(szSourceFilename) == 0){
    help_do();
    return EXIT_FAILURE;
  }
  if ((pfFileI = fopen (szInputfilename, "r")) == NULL) {
    fprintf (stderr, "cannot open file %s!\n", szInputfilename);
    exit (16);
  } /* if */
  if ((pfFileS = fopen (szSourceFilename, "r")) == NULL) {
    fprintf (stderr, "cannot open file %s!\n", szSourceFilename);
    exit (16);
  } /* if */
  if ((pfFileO = fopen (szOutputfilename, "w")) == NULL) {
    fprintf (stderr, "cannot open file %s!\n", szOutputfilename);
    exit (16);
  } /* if */


  memset(szLine,0x00,sizeof(szLine));
  while (fgets(szLine, sizeof(szLine), pfFileI)) {
    fwrite (szLine,sizeof(char),strlen(szLine),pfFileO);
    if((s=strstr(szLine, "Statment Number")) != NULL) {
      break;
    }
  }

  iCount=0; 
  memset(szPackage,0x00,sizeof(szPackage));
  while (fgets(szLine, sizeof(szLine), pfFileI)) {
    iTokcount = 0;
    iCount++;
    if (iCount > 1 && ! strstr (szLine,szPackage)){
      fwrite (szLine,sizeof(char),strlen(szLine),pfFileO);
      fflush (pfFileO);
      continue;
    }
    strcpy(szTok,szLine);
    ptr = strtok(szTok," ");
    while (ptr != NULL){
      iTokcount++;
      if (iTokcount == 1 && iCount == 1) strcpy (szPackage,ptr); 
      if (iTokcount == 2 ) iStatementnumber = atoi(ptr); 
      if (iTokcount == 3) iStatementtype = atoi(ptr); 
      ptr = strtok(NULL," \t");
    }
    memset(szSearchbuffer,0x00,sizeof(szSearchbuffer));
    sprintf(szSearchbuffer,"%s%d,%d",bufsqlacall,iStatementtype,iStatementnumber);

    rewind(pfFileS);
    memset(szLine2,0x00,sizeof(szLine2));
    memset(szExecStatement,0x00,sizeof(szExecStatement));
    while (fgets(szLine2, sizeof(szLine2), pfFileS)) {
      if((s=strstr(szLine2, "EXEC")) != NULL) {
        strcpy(szExecStatement,s);
        continue;
      }
      if((s=strstr(szLine2, szSearchbuffer)) != NULL) {
        strcpy (szExecLine,szLine);
        szExecLine[strlen(szExecLine)-1] = ' ';
        strcat (szExecLine,szExecStatement);
        fwrite (szExecLine,sizeof(char),strlen(szExecLine),pfFileO);
        fflush (pfFileO);
        break;
      }
    }
  }
  fclose (pfFileO);
  fclose (pfFileI);
  fclose (pfFileS);
}
