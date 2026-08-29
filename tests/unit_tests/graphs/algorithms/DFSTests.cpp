#include <gtest/gtest.h>

#include <graphs/DirectedGraph.h>
#include <graphs/algorithms/DFS.h>

TEST(DFSTest, ReturnsFalseWhenTheStartNodeDoesNotExist)
{
    DirectedGraph<int> graph;
    graph.addNode(2);

    EXPECT_FALSE(dfs::hasPath(graph, 1, 2));
}

TEST(DFSTest, FindsTheTrivialPathForAnExistingStartNode)
{
    DirectedGraph<int> graph;
    graph.addNode(1);

    EXPECT_TRUE(dfs::hasPath(graph, 1, 1));
}

TEST(DFSTest, DoesNotTreatAMissingNodeAsHavingATrivialPath)
{
    const DirectedGraph<int> graph;

    EXPECT_FALSE(dfs::hasPath(graph, 1, 1));
}

TEST(DFSTest, FindsADirectEdge)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_TRUE(dfs::hasPath(graph, 1, 2));
}

TEST(DFSTest, FindsAPathAcrossMultipleLevels)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(2, 4));

    EXPECT_TRUE(dfs::hasPath(graph, 1, 4));
}

TEST(DFSTest, ReturnsFalseForAnUnreachableNode)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_FALSE(dfs::hasPath(graph, 1, 3));
}

TEST(DFSTest, RespectsTheDirectionOfEdges)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_FALSE(dfs::hasPath(graph, 2, 1));
}

TEST(DFSTest, FindsAReachableNodeInACyclicGraph)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 1));
    ASSERT_TRUE(graph.addEdge(2, 3));

    EXPECT_TRUE(dfs::hasPath(graph, 1, 3));
}

TEST(DFSTest, ContinuesSearchingAfterEncounteringAnAlreadyVisitedNeighbour)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(1, 4));
    ASSERT_TRUE(graph.addEdge(2, 3));

    EXPECT_TRUE(dfs::hasPath(graph, 1, 4));
}

TEST(DFSCycleTest, ReturnsFalseForAGraphWithoutCycles)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 3));

    EXPECT_FALSE(dfs::hasCycle(graph));
}

TEST(DFSCycleTest, DoesNotTreatAGloballyVisitedNodeAsAnAncestor)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(3, 2));
    ASSERT_TRUE(graph.addEdge(3, 4));
    ASSERT_TRUE(graph.addEdge(4, 2));

    EXPECT_FALSE(dfs::hasCycle(graph));
}

TEST(DFSCycleTest, DetectsASimpleCycle)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 3));
    ASSERT_TRUE(graph.addEdge(3, 1));

    EXPECT_TRUE(dfs::hasCycle(graph));
}

TEST(DFSCycleTest, DetectsACycleInADisconnectedComponent)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    graph.addNode(5);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(3, 4));
    ASSERT_TRUE(graph.addEdge(4, 5));
    ASSERT_TRUE(graph.addEdge(5, 3));

    EXPECT_TRUE(dfs::hasCycle(graph));
}

TEST(DFSCycleTest, DetectsASelfLoop)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    ASSERT_TRUE(graph.addEdge(1, 1));

    EXPECT_TRUE(dfs::hasCycle(graph));
}
