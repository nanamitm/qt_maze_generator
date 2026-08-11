#ifndef DISJOINTSET_H
#define DISJOINTSET_H

#include <numeric>
#include <vector>

// Union-find over cell indices, shared by the algorithms that build the maze
// by merging groups of cells rather than by walking from one to the next.
class DisjointSet {
public:
    void reset(int count)
    {
        m_parent.resize(count);
        std::iota(m_parent.begin(), m_parent.end(), 0);
        m_rank.assign(count, 0);
    }

    int find(int x)
    {
        while (m_parent[x] != x) {
            m_parent[x] = m_parent[m_parent[x]]; // path halving
            x = m_parent[x];
        }
        return x;
    }

    // Returns false when both were already in the same group, which is how
    // callers detect the edge that would have closed a loop.
    bool unite(int a, int b)
    {
        a = find(a);
        b = find(b);
        if (a == b) {
            return false;
        }
        if (m_rank[a] < m_rank[b]) {
            std::swap(a, b);
        }
        m_parent[b] = a;
        if (m_rank[a] == m_rank[b]) {
            ++m_rank[a];
        }
        return true;
    }

private:
    std::vector<int> m_parent;
    std::vector<int> m_rank;
};

#endif // DISJOINTSET_H
