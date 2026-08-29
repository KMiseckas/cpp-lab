#pragma once

#include <graphs/DirectedGraph.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dfs
{

    namespace 
    {
        template<typename T>
        bool searchNext(const DirectedGraph<T>& graph, const T& from, const T& to, std::unordered_set<T>& visited)
        {
            for(auto& n : graph.edges(from))
            {
                if(!visited.insert(n).second)
                {
                    continue;
                }

                if(n == to)
                {
                    return true;
                }

                if(searchNext(graph, n, to, visited))
                {
                    return true;
                }
            }

            return false;
        }

        template<typename T>
        bool checkNext(
            const DirectedGraph<T>& graph,
            const T& node,
            std::unordered_set<T>& visited,
            std::unordered_set<T>& currentPath)
        {
            for(auto& n : graph.edges(node))
            {
                if (currentPath.contains(n))
                {
                    return true;
                }

                if (visited.contains(n))
                {
                    continue;
                }

                visited.insert(n);
                currentPath.insert(n);

                if(checkNext(graph, n, visited, currentPath))
                {
                    return true;
                }
            }

            currentPath.erase(node);

            return false;
        }
    }

    template<typename T>
    bool hasPath(const DirectedGraph<T>& graph, const T& from, const T& to)
    {
        if(!graph.hasNode(from) || !graph.hasNode(to))
        {
            return false;
        }

        if(from == to)
        {
            return true;
        }

        std::unordered_set<T> visited;
        visited.insert(from);

        return searchNext(graph, from, to, visited);
    }

    template<typename T>
    bool hasCycle(const DirectedGraph<T>& graph)
    {
        if(graph.empty())
        {
            return false;
        }

        std::unordered_set<T> visited;
        std::unordered_set<T> currentPath;

        for(const auto& nPair : graph)
        {
            const auto& n = std::get<0>(nPair);

            if(!visited.insert(n).second)
            {
                continue;
            }

            currentPath.insert(n);

            if(checkNext(graph, n, visited, currentPath))
            {
                return true;
            }
        }

        return false;
    }
}