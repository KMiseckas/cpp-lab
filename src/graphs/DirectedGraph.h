#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <span>

/**
 * @brief Directed graph, using adjacency list as method for storing nodes/edges. 
 * Does not enforce acyclic behaviour.
 */
template<typename T>
class DirectedGraph
{
public:
    DirectedGraph() = default;
    ~DirectedGraph() = default;

    void addNode(const T& node)
    {
        if(m_graph.contains(node))
        {
            return; // Early return, node already exists.
        }

        m_graph[node] = std::vector<T>();
    }

    bool addEdge(const T& from, const T& to)
    {
        if(!m_graph.contains(from))
        {
            return false; // No node to form an edge from.
        }
        
        auto& connections = m_graph[from];
        if(std::find(connections.begin(), connections.end(), to) != connections.end())
        {
            return true; // Edge exists, consider as succesfully added.
        }

        m_graph[from].push_back(to);
        return true;
    }

    bool hasNode(const T& node) const
    {
        return m_graph.contains(node);
    }

    std::span<const T> edges(const T& forNode)
    {
        const auto it = m_graph.find(forNode);

        if(it == m_graph.end())
        {
            return {};
        }

        return it->second;
    }

    const std::span<const T> edges(const T& forNode) const
    {
        const auto it = m_graph.find(forNode);

        if(it == m_graph.end())
        {
            return {};
        }

        return it->second;
    }

    void removeNode(const T& node)
    {
        // Does not remove edges with same node.
        m_graph.erase(node);
    }

    void removeEdge(const T& from, const T& to)
    {
        if(!m_graph.contains(from))
        {
            return;
        }

        auto& connections = m_graph[from];
        const auto it = std::find(connections.begin(), connections.end(), to);
        if(it == connections.end())
        {
            return;
        }

        connections.erase(it);
    }

private:

    std::unordered_map<T, std::vector<T>> m_graph;

};