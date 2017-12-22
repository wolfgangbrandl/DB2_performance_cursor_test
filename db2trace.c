#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sqlca.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define TRUE		1
#define FALSE		0

#define CHAR05D2INT(iV,pch) \
  (iV) =(*((pch)  )-'0')*10000,  (iV)+=(*((pch)+1)-'0')*1000,  \
  (iV)+=(*((pch)+2)-'0')*100,    (iV)+=(*((pch)+3)-'0')*10,    \
  (iV)+=(*((pch)+4)-'0')
#define SQL_NOERROR              0L
#define SQL_NOTFOUND             2000L
#define SQL_NULLCOLUMN           1003L
#define SQL_CUTVALUE             1004L
#define SQL_UNIQUE               23505L
#define SQL_MT1R_21000           21000L
#define SQL_DEADLOCK1            40001L
#define SQL_RES_UNAVAIL          57011L
#define SQL_TIMEOUT              57014L
#define SQL_DEADLOCK2            57033L
#define SQL_CONNLOST1            8001L
#define SQL_CONNLOST2            8003L
#define SQL_DESCRIBE             1005L


FILE * pfFile;

struct tnode {
  char               zPackage[10];
  unsigned short     usStatement;
  unsigned short     usType;
  int                iState;
  double             dTime;
  int                count;
  struct tnode      *right;
};
static struct tnode *root;

/*************************************************************************/
/*************************************************************************/
void tstart (struct timeval * _tstart,struct timezone * tz)
{
  gettimeofday (_tstart, tz);
}
/*************************************************************************/
/*************************************************************************/
void tend (struct timeval * _tend,struct timezone * tz)
{
  gettimeofday (_tend, tz);
}
/*************************************************************************/
/*************************************************************************/
double tval (struct timeval * _tstart,struct timeval * _tend,struct timezone * tz)
{
  double t1, t2;
  t1 = (double) _tstart->tv_sec + (double) _tstart->tv_usec/(1000*1000);
  t2 = (double) _tend->tv_sec + (double) _tend->tv_usec/(1000*1000);
  return t2-t1;
}

/*************************************************************************/
/*************************************************************************/
__attribute__ ((visibility("default"))) int printtree ()
{
  struct tnode *p;
  double  dSumTime;
  double  dPercent;
  p = root;
  while (TRUE){
    dSumTime += p->dTime;
    if (p->right == NULL) break;
    p = p->right;
  }
  printf ("SUM Time %f\n",dSumTime);
  p = root;
  printf ("   PACKAGE          Statment Number       Statement Type                 Time                  AVG           Count    Percent\n");
  while (TRUE){
    dPercent = (100.00 * p->dTime)/dSumTime;
    printf("%10.10s %24d %20d %20f %20f %15d %10.2f\n",p->zPackage,p->usStatement,p->usType,p->dTime,(p->dTime/p->count),p->count,dPercent);
    if (p->right == NULL) break;
    p = p->right;
  }
}
/*************************************************************************/
/*************************************************************************/
struct tnode *tree (struct tnode *p, char *w,int i,int j,double dTime)
{
  struct tnode talloc();
  char *strsave();
  int cond;
  if (p == NULL) {
    p = malloc (sizeof(struct tnode));
    strcpy (p->zPackage,w);
    p->count = 1;
    p->usStatement = i;
    p->usType = j;
    p->dTime = dTime;
    p->right = NULL;
  } else {
    if ((strcmp(p->zPackage,w)==0 ) && p->usStatement == i && p->usType == j){
      p->count++;
      p->dTime += dTime;
    } else {
      p->right = tree(p->right,w,i,j,dTime);
    }
  }
  return (p);
}

/*************************************************************************/
/*************************************************************************/

__attribute__ ((visibility("default"))) int DB2_call(char * sqlstate, unsigned short usTYP, unsigned short usSTMT,
                     unsigned short usIN, unsigned short usOUT, void * pv, const char * pszPRG)
{
  struct timeval _tstart;
  struct timeval _tend;
  struct timezone tz;
  register char               *  pchState = sqlstate;
  register int                   iState;
  static  const char          *  pszMess  = "PLAN:%08.08s STMT:%03d TYPE:%02d STATE:%05d\n";
  const char                  *  pzBnd    = pszPRG + 164;  /* offset in sqla_program_id */

                            /* set select to InsUpd if no out-variables: */
  int const                      iType    = (usTYP == 24 && usOUT==0) ? usTYP * -1 : usTYP;
  double                         dTime;


#undef sqlacall
#undef sqlastop
  tstart(&_tstart,&tz);
  sqlacall(usTYP, usSTMT, usIN, usOUT, pv);
  sqlastop(pv);
  tend(&_tend,&tz);
  dTime = tval(&_tstart,&_tend,&tz);

  CHAR05D2INT(iState, pchState);
  if (iState != SQL_NOERROR    &&
      iState != SQL_NOTFOUND   &&
      iState != 1003 &&             // SQL_NULLCOLUMN
      iState != 1602 &&
      iState != SQL_CUTVALUE   &&
      iState != SQL_UNIQUE     &&
      iState != SQL_MT1R_21000 &&
      iState != SQL_DESCRIBE) {
    printf(pszMess, pzBnd, usSTMT, usTYP, iState);
    } 
    root = tree (root, pzBnd,usSTMT,usTYP,dTime);

  return TRUE;
} /* DB2_call */
