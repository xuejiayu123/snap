#include <gtest/gtest.h>

#include "Snap.h"

// Test the default constructor
TEST(TUNGraph, DefaultConstructor) {
  PUNGraph Graph;

  Graph = TUNGraph::New();

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());
  EXPECT_EQ(0,Graph->HasFlag(gfDirected));
}

// Test node, edge creation
TEST(TUNGraph, ManipulateNodesEdges) {
  int NNodes = 10000;
  int NEdges = 100000;
  const char *FName = "test.graph";

  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int i;
  int n;
  int NCount;
  int x,y;
  int Deg, InDeg, OutDeg;

  Graph = TUNGraph::New();
  EXPECT_EQ(1,Graph->Empty());

  // create the nodes
  for (i = 0; i < NNodes; i++) {
    Graph->AddNode(i);
  }
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(NNodes,Graph->GetNodes());

  // create random edges
  NCount = NEdges;
  while (NCount > 0) {
    x = (long) (drand48() * NNodes);
    y = (long) (drand48() * NNodes);
    // Graph->GetEdges() is not correct for the loops (x == y),
    // skip the loops in this test
    if (x != y  &&  !Graph->IsEdge(x,y)) {
      n = Graph->AddEdge(x, y);
      NCount--;
    }
  }

  EXPECT_EQ(NEdges,Graph->GetEdges());

  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(1,Graph->IsOk());

  for (i = 0; i < NNodes; i++) {
    EXPECT_EQ(1,Graph->IsNode(i));
  }

  EXPECT_EQ(0,Graph->IsNode(NNodes));
  EXPECT_EQ(0,Graph->IsNode(NNodes+1));
  EXPECT_EQ(0,Graph->IsNode(2*NNodes));

  // nodes iterator
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NCount++;
  }
  EXPECT_EQ(NNodes,NCount);

  // edges per node iterator
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      NCount++;
    }
  }
  EXPECT_EQ(NEdges*2,NCount);

  // edges iterator
  NCount = 0;
  for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    NCount++;
  }
  EXPECT_EQ(NEdges,NCount);

  // node degree
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Deg = NI.GetDeg();
    InDeg = NI.GetInDeg();
    OutDeg = NI.GetOutDeg();

    EXPECT_EQ(Deg,InDeg);
    EXPECT_EQ(Deg,OutDeg);
  }

  // assignment
  Graph1 = TUNGraph::New();
  *Graph1 = *Graph;

  EXPECT_EQ(NNodes,Graph1->GetNodes());
  EXPECT_EQ(NEdges,Graph1->GetEdges());
  EXPECT_EQ(0,Graph1->Empty());
  EXPECT_EQ(1,Graph1->IsOk());

  // saving and loading
  {
    TFOut FOut(FName);
    Graph->Save(FOut);
    FOut.Flush();
  }

  {
    TFIn FIn(FName);
    Graph2 = TUNGraph::Load(FIn);
  }

  EXPECT_EQ(NNodes,Graph2->GetNodes());
  EXPECT_EQ(NEdges,Graph2->GetEdges());
  EXPECT_EQ(0,Graph2->Empty());
  EXPECT_EQ(1,Graph2->IsOk());

  // remove all the nodes and edges
  for (i = 0; i < NNodes; i++) {
    n = Graph->GetRndNId();
    Graph->DelNode(n);
  }

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());

  Graph1->Clr();

  EXPECT_EQ(0,Graph1->GetNodes());
  EXPECT_EQ(0,Graph1->GetEdges());

  EXPECT_EQ(1,Graph1->IsOk());
  EXPECT_EQ(1,Graph1->Empty());
}

void print_nodes(PUNGraph Graph) {
  printf("Nodes: ");
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    printf("%d ", NI.GetId());
  }
  printf("\n");
}

// Test edge iterator, manipulate edges
TEST(TUNGraph, ManipulateEdges) {
  int NNodes;
  int NNodesStart = 8;
  int NNodesEnd = 50;
  
  int NEdges;
  int NEdgesStart = 0;
  int NEdgesEnd = 25;
  
  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int NCount;
  int x,y;
  TIntV NodeIds;
  
  THashSet<TIntPr> EdgeSet;
  for (NEdges = NEdgesStart; NEdges <= NEdgesEnd; NEdges++) {
    
    for (NNodes = NNodesStart; NNodes <= NNodesEnd; NNodes++) {
      
      Graph = TUNGraph::New();
      EXPECT_TRUE(Graph->Empty());
      
      // Generate NNodes in random order
      NodeIds.Gen(NNodes);
      for (int n = 0; n < NNodes; n++) {
        NodeIds[n] = n;
      }
            
      // Add the nodes in random order
      NodeIds.Shuffle(TInt::Rnd);
      for (int n = 0; n < NNodes; n++) {
        Graph->AddNode(n);
      }
      EXPECT_FALSE(Graph->Empty());
        
      // Create random edges
      NCount = NEdges;
      while (NCount > 0) {
        x = (long) (drand48() * NNodes);
        y = (long) (drand48() * NNodes);
        
        if (!Graph->IsEdge(x,y)) {
          Graph->AddEdge(x, y);
          EdgeSet.AddKey(TIntPr(x, y));
          EdgeSet.AddKey(TIntPr(y, x));
          NCount--;
        }
      }
          
      // Check edge iterator to make sure all edges are valid and no more (in hash set)
      TIntPrV DelEdgeV;
      for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
        
        TIntPr Edge(EI.GetSrcNId(), EI.GetDstNId());
        TIntPr EdgeR(EI.GetDstNId(), EI.GetSrcNId());
//        printf("Edge %d, %d\n", (int)Edge.Val1, (int)Edge.Val2);
        
        EXPECT_TRUE(EdgeSet.IsKey(Edge) || EdgeSet.IsKey(EdgeR));
        if (EdgeSet.IsKey(Edge)) {
            EdgeSet.DelKey(Edge);
        }
        if (EdgeSet.IsKey(EdgeR)) {
          EdgeSet.DelKey(EdgeR);
        }
        DelEdgeV.Add(Edge);
        
      }
      EXPECT_TRUE(EdgeSet.Empty());
      EXPECT_TRUE(DelEdgeV.Len() == NEdges);
      
      // Randomly delete node, check to make sure edges were deleted
      NodeIds.Shuffle(TInt::Rnd);
      for (int n = 0; n < NNodes; n++) {

        TIntPrV DelEdgeNodeV;
        int EdgesBeforeDel;
        EdgesBeforeDel = Graph->GetEdges();
        for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
          if (EI.GetSrcNId() == n || EI.GetDstNId() == n) {
            DelEdgeNodeV.Add(TIntPr(EI.GetSrcNId(), EI.GetDstNId()));
          }
        }
        Graph->DelNode(n);
        EXPECT_TRUE(EdgesBeforeDel == DelEdgeNodeV.Len() + Graph->GetEdges());
        EXPECT_FALSE(Graph->IsNode(n));
        EXPECT_TRUE(Graph->IsOk());
        
        // Make sure all the edges are gone
        for (int e = 0; e < DelEdgeNodeV.Len(); e++) {
          EXPECT_FALSE(Graph->IsEdge(DelEdgeNodeV[e].Val1, DelEdgeNodeV[e].Val2));
        }
        
        // Make sure no edge on graph is connected to deleted node
        for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
          EXPECT_FALSE(EI.GetSrcNId() == n || EI.GetDstNId() == n);
        }
      }

      EXPECT_TRUE(0 == Graph->GetEdges());
    }
  }

}

// Test small graph
TEST(TUNGraph, GetSmallGraph) {
  PUNGraph Graph;

  Graph = TUNGraph::GetSmallGraph();

  EXPECT_EQ(5,Graph->GetNodes());
  EXPECT_EQ(5,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(0,Graph->HasFlag(gfDirected));
}

