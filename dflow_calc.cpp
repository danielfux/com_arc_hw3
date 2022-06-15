/* 046267 Computer Architecture - Spring 22 - HW #3 */
/* Implementation for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
#include <utility>
#include <vector>

using namespace std;
static const int entry = -1;
static unsigned int *max_depth;
static unsigned int *instCyc;
typedef std::pair<int,int> Pair;


struct Edge {
    int src_node_;
    int dst_node_;
    int weight_;
};

class Graph {
public:
    unsigned int numOfInsts;
    // a vector of vectors of Pairs to represent an adjacency list
    vector<vector<Pair>> adjList;

    /// Graph Constructor
    Graph(vector<Edge> const &edges, unsigned int n) : numOfInsts(n) {
        // resize the vector to hold `n` elements of type vector<Edge>
        adjList.resize(n); // n rows
        // add edges to the directed graph
        for (auto &edge: edges)
            // insert at the end
            adjList[edge.src_node_].push_back(make_pair(edge.dst_node_, edge.weight_));
    }

    static void setEdge(vector<Edge> &edges, int src_node_idx, int dst_node_idx, int weight) {
        Edge edge;
        edge.src_node_ = src_node_idx;
        edge.dst_node_ = dst_node_idx;
        edge.weight_ = weight;
        edges.push_back(edge);
        unsigned int temp;
        if(dst_node_idx<=-1)
            temp = 0;
        else
            temp = max_depth[dst_node_idx];
        unsigned int tmp_max_weight = instCyc[src_node_idx] + temp;
        if (tmp_max_weight > max_depth[src_node_idx]) {
            max_depth[src_node_idx] = tmp_max_weight;
        }
    }
};

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
   /* if (numOfInsts == 0){
        return PROG_CTX_NULL;
    }
    try{ */
        vector<Edge> edges;
        instCyc = new unsigned int[numOfInsts] ;
        max_depth = new unsigned int[numOfInsts];
        instCyc[numOfInsts] = {0};
        max_depth[numOfInsts]={0};


        for (unsigned lineNum = 0 ; lineNum < numOfInsts ; lineNum++){
            //cout << "lineNum="<< lineNum << endl;
            bool flag_s1 = false;
            bool flag_s2 = false;
            instCyc[lineNum] = opsLatency[progTrace[lineNum].opcode];
            if(lineNum == 0){
                Graph::setEdge(edges, lineNum,entry, opsLatency[progTrace[lineNum].opcode]);
                Graph::setEdge(edges, lineNum,entry, opsLatency[progTrace[lineNum].opcode]);
            }
            else{
                for(unsigned  j = lineNum - 1 ; j >= 0 ; --j) {
                  // cout << "j=" << j << endl;
                    // build edge from the src1 to its dependency
                    if (progTrace[j].dstIdx == progTrace[lineNum].src1Idx && !flag_s1) {
                        flag_s1 = true;
                        Graph::setEdge(edges, lineNum, j, instCyc[j]);
                        break;
                    }
                    if(j == 0) break;
                }
                if(!flag_s1){
                    Graph::setEdge(edges, lineNum,entry, opsLatency[progTrace[lineNum].opcode]);
                }

                    for(unsigned  j = lineNum - 1 ; j >= 0 ; --j) {
                        // build edge from the src2 to its dependency
                        if (progTrace[j].dstIdx == progTrace[lineNum].src2Idx && !flag_s2) {
                            flag_s2 = true;
                            Graph::setEdge(edges, lineNum, j, instCyc[j]);
                            break;
                        }
                        if(j == 0) break;
                    }

                if(!flag_s2){
                    Graph::setEdge(edges,lineNum,entry, opsLatency[progTrace[lineNum].opcode]);
                }
            }
           // cout << "max_dep:" << max_depth[lineNum] << "lineNum:"<<lineNum << endl;
        }

        Graph *graph = new Graph(edges,numOfInsts);
        if(graph == NULL) return PROG_CTX_NULL;
        return graph;
   // }
  /*  catch (...){
        delete[] max_depth;
        delete[] instCyc;
        return PROG_CTX_NULL;
    }
    */


}

void freeProgCtx(ProgCtx ctx) {
    Graph *dataflow = (Graph*)ctx;
    delete[] max_depth;
    delete[] instCyc;
    delete dataflow;
}

/** getInstDepth: Get the dataflow dependency depth in clock cycles
    Instruction that are direct decendents to the entry node (depend only on Entry) should return 0
    \param[in] ctx The program context as returned from analyzeProg()
    \param[in] theInst The index of the instruction of the program trace to query (the index in given progTrace[])
    \returns >= 0 The dependency depth, <0 for invalid instruction index for this program context
*/
int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    Graph *dataflow = (Graph*)ctx;
    if (dataflow->numOfInsts <= theInst || theInst < 0)
        return -1;
    else
        return max_depth[theInst]-instCyc[theInst]; // in cycles
}
/** getInstDeps: Get the instructions that a given instruction depends upon
    \param[in] ctx The program context as returned from analyzeProg()
    \param[in] theInst The index of the instruction of the program trace to query (the index in given progTrace[])
    \param[out] src1DepInst Returned index of the instruction that src1 depends upon (-1 if depends on "entry")
    \param[out] src2DepInst Returned index of the instruction that src2 depends upon (-1 if depends on "entry")
    \returns 0 for success, <0 for error (e.g., invalid instruction index)
*/
int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    Graph *dataflow = (Graph*)ctx;

    *src1DepInst = dataflow->adjList[theInst].at(0).first;
    *src2DepInst = dataflow->adjList[theInst].at(1).first;

    if (dataflow->numOfInsts <= theInst || theInst < 0)
        return -1;
    else
        return 0;
}
/** getProgDepth: Get the longest execution path of this program (from Entry to Exit)
    \param[in] ctx The program context as returned from analyzeProg()
    \returns The longest execution path duration in clock cycles
*/
int getProgDepth(ProgCtx ctx) {
    Graph *dataflow = (Graph*)ctx;
    int max = 0;
    for(unsigned int line = 0 ; line < dataflow -> numOfInsts ; ++line){
        if(max < (int)max_depth[line]){
            max = (int)max_depth[line];
        }
    }
    return max;// in cycles
}



