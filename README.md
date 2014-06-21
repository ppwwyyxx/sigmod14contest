#### This is the source code of team 'blxlrsmb' from Tsinghua University in ACM SIGMOD Programming Contest 2014

### Member:
+ [Yuxin Wu](mailto:ppwwyyxxc@gmail.com)
+ [Xinyu Zhou](mailto:zxytim@gmail.com)
+ [Wenbo Tao](mailto:thierryhenrytracy@163.com)
+ [Jiawen Liang](mailto:taobingxue001@126.com)
+ [Junbang Liang](mailto:williamm2006@126.com)
+ [Han Zhao](mailto:nikifor383@gmail.com)
+ Advisor: [Guoliang Li](mailto:liguoliang@tsinghua.edu.cn)
+ Special Thanks: [Jian He](mailto:jianhe25@gmail.com)

## Solutions:
### Task Overview
Implement a social network analysis system.
Given a huge social network as an undirected weighted graph G(V, E) with the
following attributes on each vertex v: a numeric attribute v_d, a set of tags v_t.
The system shall support the following 4 types of queries (in terms of graph theory):

### Query 1
Given Vertex p1, p2, and x, calculate the length of shortest path between p1 and p2
such that each edge e on the path has weight e_w \ge x.

### Solution
Execute BFS(Breadth First Search) from both p1 and p2.
Use a bit-array maintaining all the visited vertices, to see if they have met in the middle.

### Query 2
Given k and d, find top-k tags with
largest range. Here the range of a tag T in graph G(V, E) is
defined as the size of the largest possible subgraph G'(V', E'), such that:
forall v in V', v_d > d && T \in v_t

### Solution:
First sort all the queries and vertices by d in descending order. Build an empty
graph G_0 and incrementally insert sorted vertices and their corresponding edges to G_0.
During the insertion, we maintain the top-k largest connected components for each tag
using Union-Find Set. A query q would finish as soon as all vertices
v with v_d > q_d are inserted.

### Query 3
Given k, h, p, where p is used to define a subset V' of V.
Find the top-k pairs of vertex (u, v) ordered by |u_t \intersect v_t|,
also satisfying: u, v \in V' && dist(u, v) <= h

### Solution:
First we build V' according to p. Then we build a inverted list for
every possible tag T in v_t: L(T) = a list of vertex v \in V' where T \in v_t.
This way, we can quickly find all the vertex in V' with shared tags by merging the inverted lists.

### Query 4
Given k, t, where t is used to define a subgraph G' of G, find the top-k centralized vertex in G'.
Here the centrality for a vertex v in a graph G(V, E) is defined as :
C(v) = \dfrac{r(v)^2}{(|V|-1) * \sum{dist(v, u)}}, where r(v) is
the number of vertex reachable from v (exclusive).

### Solution:
The subgraph can be efficiently built. The difficulty lies in the calculation
of C(v), since S(v)= \sum{} has O(|V|^2) time complexity for each v. The
solution shall make full use of 'top-k' in pruning as well as the properties of social
network to be efficient.

Our general approach is to first estimate an upper bound of C(v) for each v,
and maintain all the upper bounds with a max-heap.
We then iteratively pop the top element from the heap and calculate its
actual value. If the actual value is still the largest
in the heap, then it is certainly larger than the actual value of all the other
uncalculated elements in the heap, so we can take this element as a result.
Otherwise we insert this actual value back to the heap.

Here is an obvious trade-off in the framework: the time spent in estimation
the upper bound vs. the time spent in calculating actual values. A tight
estimation of upper bound will save time in the heap, but generally will
spend more times in estimation.

We use the following method to estimate a lower bound of S(v), i.e. an upper
bound of C(v) for each v. We start a BFS from v and limit the search depth to
l, to calculate all dist(v, u) when dist(v, u) \le l. All the unvisited
vertices are assumed to be l+1 distance away from v, i.e.:
S(v) \ge \sum_{dist(v, u) \le l}{dist(v, u)} + \sum_{}{l + 1}

Since social networks have small diameter, a small l (3 or 4) is sufficient to
guarantee a good lower bound estimation of S(v). However, to further optimize the algorithm, we
need a strategy to choose l. We compare our current estimation of each S(v)
to an approximation of each S(v), and increase l when the mean-square
error is larger than our threshold.

Then we would need a pretty accurate approximation of S(v). We achieve this
by random sampling a subset of V called V', and run a thorough BFS on each of them.
Then for each v \in V, its S(v) can be approximated by \sum{u \in V'}{dist(u, v)} * |V| / |V'|.
This is a highly accurate approximation, which gave an average 1~2% error sampled with 0.1% of V.

We also applied the approximation in prunning: by calculating actual
centrality of the vertices with top-(3 * k) approximated centrality, we get 3 * k
actual centrality that is very likely to contain the actual top-k centralities.
Then the kth centrality of the 3 * k results can give a strong prunning.

### Other Tricks:
+ Use a dynamic programming by a SSE-optimized bitset to replace BFS. This is faster in some cases.
+ Use a self-implemented threadpool to manage threads.
+ Use mmap to quickly read data files.

## Dependencies
+ google sparsehash
+ google tcmalloc in gperftools(depends on liblzma, libunwind in third-party/*.a)
