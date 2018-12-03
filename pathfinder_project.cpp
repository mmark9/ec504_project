/* 
   Pathfinding Algorithm Project
   Author: Michael Graziano mjgrazia@bu.edu
   Author: Miguel Mark mmark9@bu.edu
*/

#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <malloc.h>
#include <time.h>
#define maxnodes 200000
#define maxarcs  500000
#define LARGE 99999999

using namespace std;

clock_t clock(void);

struct arc{
  struct arc *next;   // Pointer to next arc
  int end_node;       // End node of the arc
  int weight;         // Weight of the arc
  };

struct node{
   struct arc *first; // First arc in linked list
   int priority;      // Key of node in priority queue
   int start_dis;     // Distance from start node
   int end_dis;       // Distance from end node
   int previous;      // Predecessor node
   int queue_pos;     // Position of node in priority queue (0..Nm-1)
   };

struct node Nodes[maxnodes];
int HP[maxnodes]; /* heap array */
int Na,Nm,Or;
int heapSize = 0; /* Used to maintain the size of the heap */

bool Relax_Dijkstra(int b_node, int e_node, int weight) {
    /* 
       This function is used to perform the relaxation of nodes for 
       Dijkstra's Algorithm. It takes in the node labels
       b_node (begin node) and e_node (end_node) and determines whether the
       the end_nodes current distance can be improved.
    */

    if (Nodes[e_node].priority > Nodes[b_node].priority + weight) {
        Nodes[e_node].priority = Nodes[b_node].priority + weight;
        Nodes[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

bool Discover_GreedyBest(int b_node, int e_node) {
    /* 
       This function is used to perform the discovery of nodes for
       the Greedy Best First Search. It takes in the node labels
       b_node (begin node) and e_node (end_node) and updates e_node with
       its distance to the end node if it has never been seen.
    */

    if (!Nodes[e_node].priority == LARGE) {
        Nodes[e_node].priority = Nodes[e_node].end_dis;
        Nodes[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

bool Relax_Astar(int b_node, int e_node, int weight) {
    /* 
       This function is used to perform the relaxation of nodes for
       A* Algorithm. It takes in the node labels b_node (begin node)
       and e_node (end_node) and determines whether the end node's current
       start distance can be improved. The priority is then the combination
       computed as this start distance plus the end distance.
    */

    
    if (Nodes[e_node].start_dis > Nodes[b_node].start_dis + weight) {
        Nodes[e_node].start_dis = Nodes[b_node].start_dis + weight;
        Nodes[e_node].priority = Nodes[e_node].start_dis + Nodes[e_node].end_dis;
        Nodes[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

void upHeap(int new_pos)
{
    /* This code is augmented from Homework_03 coding assignment */

    // Variable Declaration:
    int temp;                         // Used to hold data during swaps.
    int target = HP[new_pos];         // Holds the node being upheaped
    int parent_pos= (new_pos-1)/2;    // Holds the parent index of new_pos
    int parent_node = HP[parent_pos]; // Holds the node being compared
    
    // Only perform the swap if the elemnt is not the root and the parent is
    // further away.
    if (new_pos != 0 && Nodes[target].priority < Nodes[parent_node].priority) {
        temp = target;
        HP[new_pos] = parent_node;
        Nodes[parent_node].queue_pos = new_pos;
        HP[parent_pos] = temp;
        Nodes[temp].queue_pos = parent_pos;
        upHeap(parent_pos);
    }

    return;
}

void downHeap(int root)
{
    /* This code is augmented from Homework_03 coding assignment */
    
    // Variable Declarations:
    int left, right, smallest; // Holds the position of the left child,
                              // right child, and the smallest value
    int temp;                 // Holds data during swaps.

    // Initialization:
    left = 2*root;
    right = 2*root + 1;
    smallest = root;
    
    // Check 1: See if the closest element is farther than the left child.
    if (left <= heapSize && Nodes[HP[left]].priority < Nodes[HP[smallest]].priority) {
        smallest = left;
    }
    
    // Check 2: See if the closest element is farther than the right child.
    if (right <= heapSize && Nodes[HP[right]].priority < Nodes[HP[smallest]].priority) {
        smallest = right;
    }

    // Check 3: See if the closest element is the root element.
    // If not, then the elements must be swapped and a new downHeap is performed.
    if (smallest != root) {
        temp = HP[root];
        HP[root] = HP[smallest];
        Nodes[HP[root]].queue_pos = root;
        HP[smallest] = temp;
        Nodes[temp].queue_pos = smallest;
        downHeap(smallest);
    }

    return;
}

void insert(int node_label)
{
    /* This code is augmented from Homework_03 coding assignment */

    // Variable Declarations:
    int i; // Index of the element being added.

    // Check 1: If the heapSize is equal to maxnodes, then skip insert.
    if (heapSize == maxnodes) { return; }
    else {
        heapSize++;
        i = heapSize - 1;
        HP[i] = node_label;
        Nodes[node_label].queue_pos = i;
        upHeap(i);
    }
    
    return;
}

int remove()
{
    /* This code is augmented from Homework_03 coding assignment */

    // Variable Declarations:
    int ret_node; // Holds the node to be returned from the heap

    // Case 1: If there is nothing to remove, return -1
    if (heapSize == 0) { return -1; }

    // Case 2: There is only one element in the heap
    else if (heapSize == 1) {
        ret_node = HP[0];
        heapSize--;
    }

    // Case 3: There are more elements that need to be adjusted
    else {
        ret_node = HP[0];
        HP[0] = HP[heapSize - 1];
        Nodes[HP[0]].queue_pos = 0;
        heapSize--;
        downHeap(0);
    }

    return ret_node;
}

void Dijkstra()
{
    // Variable Declarations:
    int target;          // Holds the current target node for initialization
    struct arc * cursor; // Holds the pointer to the 

    // Step 1: Initialize the Distance of the Origin node to 0, then create
    // the heap structure.
    Nodes[Or].priority = 0;
    for (target = 1; target <= Nm; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        target = remove();
        cursor = Nodes[target].first;
        while (cursor != NULL) {
            if (Relax_Dijkstra(target, cursor->end_node, cursor->weight)) {
                upHeap(Nodes[cursor->end_node].queue_pos);
            }
            cursor = cursor->next;
        }
    }
}

void GreedyBest()
{
    // Variable Declarations:
    int target;          // Holds the current target node for initialization
    struct arc * cursor; // Holds the pointer to the 

    // Step 1: Initialize the distance of the Origin node to the heuristic
    // distance.
    Nodes[Or].priority = Nodes[Or].end_dis;
    for (target = 1; target <= Nm; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        target = remove();
        cursor = Nodes[target].first;
        while (cursor != NULL) {
            if (Discover_GreedyBest(target, cursor->end_node)) {
                upHeap(Nodes[cursor->end_node].queue_pos);
            }
            cursor = cursor->next;
        }
    }
}

void Astar()
{
    // Variable Declarations:
    int target;          // Holds the current target node for initialization
    struct arc * cursor; // Holds the pointer to the 

    // Step 1: Initialize the distance of the Origin node to the heuristic
    // distance.
    Nodes[Or].start_dis = 0;
    Nodes[Or].priority = Nodes[Or].start_dis + Nodes[Or].end_dis;
    for (target = 1; target <= Nm; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        target = remove();
        cursor = Nodes[target].first;
        while (cursor != NULL) {
            if (Relax_Astar(target, cursor->end_node, cursor->weight)) {
                upHeap(Nodes[cursor->end_node].queue_pos);
            }
            cursor = cursor->next;
        }
    }
}

int main(int argc, char *argv[])
{
  double TT2;
  clock_t startt, endt; 
  int start,val,col;
  struct arc *edge, test;
  long c0=0;
  long c1 = 0;
  FILE *fp1,*fpout;
  char *infile;
  char *outfile;
  int destinations[10];

/* For simplicity, we will skip node 0, label all with true nodes */
// printf("Enter file name for input \n");
// scanf("%s",infile);
// strcpy(infile, argv[1]);
  infile =  argv[1];
  cout <<" infile  " << infile << endl;
  fp1 = fopen(infile,"r");
  if (fp1 == NULL) {
	  printf("Did not find input file \n");
	  exit(1);
  }
  fscanf(fp1,"%d %d",&Nm,&Na);
  for (int i=0;i<=Nm;i++){
	Nodes[i].first = NULL;
	Nodes[i].priority = LARGE;
    Nodes[i].start_dis = 0;
    Nodes[i].end_dis = 0;
	Nodes[i].previous = 0;
	Nodes[i].queue_pos = 0;
  }

  for (int i=0;i<Na;i++){ 
     fscanf(fp1,"%d %d %d",&start,&col,&val);
     edge = (struct arc *)malloc(sizeof(test));
     edge->weight = val; edge->end_node = col;
     edge->next = Nodes[start].first;
     Nodes[start].first = edge;
  }
  fclose(fp1);
  for (int i = 0; i < 10; i++) {
	  destinations[i] = (int)((i + 1)*(Nm / 10) - 1 );
  }

  outfile = strcat(infile,"_out");
  fpout = fopen(outfile,"w");

  cout << " fpout " << outfile << endl;
//#include <fstream>
  Or = 5; // source node


  fprintf(fpout,"0.5\nCALLING DIJKSTRA/HEAP TO SOLVE THE PROBLEM\n");

  startt = clock(); 
  Dijkstra();
  endt = clock();
  TT2 = (double)(endt-startt)/CLOCKS_PER_SEC;
  //printf("FINISHED --- TOTAL CPU TIME %f SECS \n",(float)TT2);
  // fprintf(fpout,"FINISHED --- TOTAL CPU TIME %f SECS \n",(float)TT2);

  for (int i = 0; i < 10; i++) {
	  col = destinations[i];
	  fprintf(fpout,"Shortest distance to %d is %d \n", col, Nodes[col].priority);         
	  fprintf(fpout,"path to %d ", col);
	  col = Nodes[col].previous;
	  while (col > 0) {
		  fprintf(fpout," -- %d ", col);
		  col = Nodes[col].previous;
	  }
	  fprintf(fpout,"\n \n");
  }
  
  fclose(fpout);

  return 0;
}

