#include <gtest/gtest.h>

#include <graphs/DirectedGraph.h>

#include <cstddef>
#include <initializer_list>

namespace
{
void expectEdges(const DirectedGraph<int>& graph, const int node,
                 std::initializer_list<int> expected)
{
    const auto actual = graph.edges(node);

    ASSERT_EQ(actual.size(), expected.size());

    std::size_t index{0U};
    for(const int edge : expected)
    {
        EXPECT_EQ(actual[index], edge);
        ++index;
    }
}
} // namespace

TEST(DirectedGraphTest, StartsWithoutNodesOrEdges)
{
    const DirectedGraph<int> graph;

    EXPECT_FALSE(graph.hasNode(1));
    expectEdges(graph, 1, {});
}

TEST(DirectedGraphTest, AddsNodes)
{
    DirectedGraph<int> graph;

    graph.addNode(1);
    graph.addNode(2);

    EXPECT_TRUE(graph.hasNode(1));
    EXPECT_TRUE(graph.hasNode(2));
    expectEdges(graph, 1, {});
    expectEdges(graph, 2, {});
}

TEST(DirectedGraphTest, AddingAnExistingNodeKeepsItsEdges)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    graph.addNode(1);

    expectEdges(graph, 1, {2});
}

TEST(DirectedGraphTest, AddsDirectedEdgesInInsertionOrder)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);

    ASSERT_TRUE(graph.addEdge(1, 3));
    ASSERT_TRUE(graph.addEdge(1, 2));

    expectEdges(graph, 1, {3, 2});
    expectEdges(graph, 2, {});
    expectEdges(graph, 3, {});
}

TEST(DirectedGraphTest, AddingTheSameEdgeTwiceDoesNotDuplicateIt)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);

    EXPECT_TRUE(graph.addEdge(1, 2));
    EXPECT_TRUE(graph.addEdge(1, 2));

    expectEdges(graph, 1, {2});
}

TEST(DirectedGraphTest, RejectsEdgesWhoseSourceNodeDoesNotExist)
{
    DirectedGraph<int> graph;
    graph.addNode(2);

    EXPECT_FALSE(graph.addEdge(1, 2));
    EXPECT_FALSE(graph.hasNode(1));
    expectEdges(graph, 1, {});
}

TEST(DirectedGraphTest, SupportsCycles)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);

    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(2, 1));

    expectEdges(graph, 1, {2});
    expectEdges(graph, 2, {1});
}

TEST(DirectedGraphTest, RemovesOnlyTheSpecifiedEdge)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    graph.addNode(3);
    ASSERT_TRUE(graph.addEdge(1, 2));
    ASSERT_TRUE(graph.addEdge(1, 3));

    graph.removeEdge(1, 2);

    expectEdges(graph, 1, {3});
}

TEST(DirectedGraphTest, RemovingAMissingEdgeIsANoOp)
{
    DirectedGraph<int> graph;
    graph.addNode(1);
    graph.addNode(2);
    ASSERT_TRUE(graph.addEdge(1, 2));

    graph.removeEdge(1, 3);
    graph.removeEdge(3, 1);

    expectEdges(graph, 1, {2});
}
