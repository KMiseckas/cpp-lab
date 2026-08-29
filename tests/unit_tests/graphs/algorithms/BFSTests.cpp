#include <gtest/gtest.h>

#include <graphs/DirectedGraph.h>
#include <graphs/algorithms/BFS.h>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace
{
void expectPathValues(const std::vector<int>& path,
                      const std::initializer_list<int> expected)
{
    ASSERT_EQ(path.size(), expected.size());

    std::size_t index{0U};
    for(const int expectedValue : expected)
    {
        EXPECT_EQ(path[index], expectedValue);
        ++index;
    }
}
} // namespace

TEST(BFSTest, ReturnsFalseWhenTheStartNodeDoesNotExist)
{
    DirectedGraph<int> graph;
    graph.addNode(2);

    EXPECT_FALSE(bfs::hasPath(graph, 1, 2));
}

TEST(BFSTest, FindsTheTrivialPathForAnExistingStartNode)
{
    DirectedGraph<int> graph;
    graph.addNode(1);

    EXPECT_TRUE(bfs::hasPath(graph, 1, 1));
}

TEST(BFSTest, DoesNotTreatAMissingNodeAsHavingATrivialPath)
{
    const DirectedGraph<int> graph;

    EXPECT_FALSE(bfs::hasPath(graph, 1, 1));
}

TEST(BFSTest, FindsADirectEdge)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_TRUE(bfs::hasPath(graph, 1, 2));
}

TEST(BFSTest, FindsAPathAcrossMultipleLevels)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(2, 4));

    EXPECT_TRUE(bfs::hasPath(graph, 1, 4));
}

TEST(BFSTest, ReturnsFalseForAnUnreachableNode)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_FALSE(bfs::hasPath(graph, 1, 3));
}

TEST(BFSTest, RespectsTheDirectionOfEdges)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    EXPECT_FALSE(bfs::hasPath(graph, 2, 1));
}

TEST(BFSTest, FindsAReachableNodeInACyclicGraph)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 1));
    ASSERT_TRUE(graph.addEdge(2, 3));

    EXPECT_TRUE(bfs::hasPath(graph, 1, 3));
}

TEST(BFSTest, ShortestPathIsEmptyWhenTheStartNodeDoesNotExist)
{
    DirectedGraph<int> graph;
    graph.addNode(2);

    expectPathValues(bfs::shortestPath(graph, 1, 2), {});
}

TEST(BFSTest, ShortestPathContainsTheStartNodeForATrivialPath)
{
    DirectedGraph<int> graph;
    graph.addNode(1);

    expectPathValues(bfs::shortestPath(graph, 1, 1), {1});
}

TEST(BFSTest, ShortestPathIncludesBothEndpointsOfADirectEdge)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    expectPathValues(bfs::shortestPath(graph, 1, 2), {1, 2});
}

TEST(BFSTest, ShortestPathChoosesThePathWithTheFewestEdges)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    graph.addNode(5);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(3, 4));
    ASSERT_TRUE(graph.addEdge(4, 5));
    ASSERT_TRUE(graph.addEdge(5, 2));

    expectPathValues(bfs::shortestPath(graph, 1, 2), {1, 2});
}

TEST(BFSTest, ShortestPathIsEmptyWhenTheDestinationIsUnreachable)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));

    expectPathValues(bfs::shortestPath(graph, 1, 3), {});
}

TEST(BFSTest, ShortestPathWorksWhenTheGraphContainsACycle)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    graph.addNode(4);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 3));
    ASSERT_TRUE(graph.addEdge(3, 1));
    ASSERT_TRUE(graph.addEdge(3, 4));

    expectPathValues(bfs::shortestPath(graph, 1, 4), {1, 2, 3, 4});
}
