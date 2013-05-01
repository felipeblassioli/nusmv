#include "dag.h"
#include "dagInt.h"


#define MAX_DEGREE 10000
#define MAX_DEPTH  10000

struct Statistics {
   int degree_stat    [MAX_DEGREE];
   int depth_stat     [MAX_DEPTH];
   int var_depth_stat [MAX_DEPTH];
   int nodes_num;
  };

typedef struct Statistics Statistics_t;

struct StatData{
   int fatherNum;
   int depth;
   int knownDepthFatherNum;
};

typedef struct StatData StatData_t;

#define FatherNum(N)           ((StatData_t*)((N)->gRef))->fatherNum
#define KnownDepthFatherNum(N) ((StatData_t*)((N)->gRef))->knownDepthFatherNum
#define Depth(N)               ((StatData_t*)((N)->gRef))->depth

static void doNothingAndReturnVoid(Dag_Vertex_t* f, char * visData, int sign) {};
static int  doNothingAndReturnZero(Dag_Vertex_t* f, char * visData, int sign) { return 0; };

static void ResetStat(Statistics_t* stat)
{
  int i;

  stat->nodes_num = 0;

  for (i=0;i<MAX_DEGREE; i++)
   (stat->degree_stat)[i]=0;

  for (i=0;i<MAX_DEPTH; i++)
   (stat->var_depth_stat)[i]=(stat->depth_stat)[i]=0;
}


static int ComputeFatherAndSonNum(Dag_Vertex_t* f, char * visData, int sign)
  {
   if (f->gRef == (char*)NULL) {

       f->gRef = (char *)ALLOC(StatData_t,1);
       KnownDepthFatherNum(f) = FatherNum(f) = Depth(f) = 0;

       (((Statistics_t*)visData)->degree_stat[((f->outList)==NULL) ? 0 : lsLength(f->outList)])++;
   }

   FatherNum(f)++;

   return (0);
}


static void ComputeDepth(Dag_Vertex_t* v, int p_depth, Statistics_t* stat)
 {
   lsGen           gen;
   lsList sonsList;
   Dag_Vertex_t * vSon;

   v = Dag_VertexGetRef(v);

   Depth(v) = MAX(p_depth,Depth(v));

   if ((++(KnownDepthFatherNum(v))) == FatherNum(v)) {

     ((stat->depth_stat)[Depth(v)])++;

     if (v -> outList != (lsList) NULL) {

         if (lsLength(v -> outList)==0)
             ((stat->var_depth_stat)[Depth(v)])++;

         gen = lsStart(v -> outList);
         while (lsNext(gen, (lsGeneric*) &vSon, LS_NH) == LS_OK)
                ComputeDepth(vSon, Depth(v)+1,stat);

         lsFinish(gen);
     }
     else
         ((stat->var_depth_stat)[Depth(v)])++;
   }

  return;
}


static void _PrintStat(Statistics_t* stat,FILE* statFile)
{
  int i;

  for (i=0;i<MAX_DEGREE; i++)
     if ((stat->degree_stat)[i]>0)
          fprintf(statFile, "Nodes with %i sons: %i.\n", i,(stat->degree_stat)[i]);

  for (i=0;i<MAX_DEPTH; i++)
     if ((stat->depth_stat)[i]>0)
          fprintf(statFile, "Nodes at depth %i: %i, leaves among them: %i.\n", i,(stat->depth_stat)[i],(stat->var_depth_stat)[i]);
}


void PrintStat(Dag_Vertex_t* dfsRoot, FILE* statFile)
{
  if (dfsRoot != (Dag_Vertex_t*)NULL) {

      Dag_DfsFunctions_t fathnsonDFS, statDFS;
      Statistics_t stat;
      ResetStat(&stat);

      fathnsonDFS.FirstVisit = statDFS.FirstVisit =
      fathnsonDFS.BackVisit  = statDFS.BackVisit  =
      fathnsonDFS.LastVisit  = statDFS.LastVisit  = doNothingAndReturnVoid;

      fathnsonDFS.Set = ComputeFatherAndSonNum;
      statDFS.Set     = doNothingAndReturnZero;

      Dag_Dfs     (dfsRoot, &dag_DfsClean,   NIL(char));
      Dag_Dfs     (dfsRoot, &fathnsonDFS, (char*)&stat);
      ComputeDepth(dfsRoot, 0           ,        &stat);
      Dag_Dfs     (dfsRoot, &statDFS    , (char*)&stat);

      _PrintStat(&stat,statFile);
  }
}

