#pragma once

#include <algorithm>
#include <graphs/DirectedGraph.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bfs
{
    template<typename T>
    bool hasPath(const DirectedGraph<T>& graph, const T& from, const T& to)
    {
        if(!graph.hasNode(from))
        {
            return false;
        }

        if(from == to)
        {
            return true;
        }

        std::queue<const T*> queue;

        // Add node to queue.
        // While has nodes in queue.
        //  pop top node.
        //  foreach neighbour
        //      is found node? Return.
        //      add to queue.

        queue.push(&from);

        while(!queue.empty())
        {
            const T& node = *(queue.front());
            queue.pop();

            auto edges = graph.edges(node);

            for(const auto& n : edges)
            {
                if(n == to)
                {
                    return true;
                }

                queue.push(&n);
            }
        }

        return false;
    }

    template<typename T>
    std::vector<T> shortestPath(const DirectedGraph<T>& graph, const T& from, const T& to)
    {
        if(!graph.hasNode(from))
        {
            return {};
        }

        if(from == to)
        {
            return {from};
        }

        std::queue<T> queue;
        std::unordered_map<T, T> parents;
        std::unordered_set<T> visited;
        queue.push(from);
        visited.insert(from);

        while(!queue.empty())
        {
            const T node = queue.front();
            queue.pop();

            auto edges = graph.edges(node);

            for(const auto& n : edges)
            {
                if(!visited.insert(n).second)
                {
                    continue;
                }

                parents.emplace(n, node);

                if(n == to)
                {
                    std::vector<T> path;

                    T current{n};
                    while(current != from)
                    {
                        path.push_back(current);
                        current = parents.at(current);
                    }

                    path.push_back(from);

                    std::reverse(path.begin(), path.end());
                    return path;
                }
                
                queue.push(n);
            }
        }

        return {};
    }
}
