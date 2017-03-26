// vim: expandtab:ts=2:sw=2
#ifndef MCF_GRAPH_HPP
#define MCF_GRAPH_HPP

#include <tuple>
#include <vector>

namespace mcf {

//! A directed edge.
struct Edge {
  int source_index;  //<! Index of source node.
  int target_index;  //<! Index of target node.
  double cost;       //<! Edge cost.
};

/**
 * This class holds a directed graph representation of a multiple object
 * tracking problem.
 *
 * Problem Formulation
 * -------------------
 *
 * Nodes in the graph represent space-time locations (typically some type of
 * point measurement or bounding box). Each location is associated with an
 * "observation cost" that is usually set to the negative log-likelihood ratio
 * of the multi-object process against the clutter process, e.g.,
 *
 *     -log [b / (1 - b)]
 *
 * with b being probability that the location is part of an object trajectory.
 *
 * Directed edges connect possibly neighboring locations on object
 * trajectories, e.g., near-by locations in successive frames. The cost of such
 * a transition edge (u, v) between locations u and v is usually set to the
 * negative log-probability
 *
 *     -log p(v | u)
 *
 * that v is a direct successor of u on a single object trajectory. For more
 * information on the problem formulation, see [1].
 *
 * [1] Zhang, Li, Nevatia: Global data association for multi-object tracking
 *     using network flows, CVPR (2008).
 *
 * Public Interface
 * ----------------
 *
 * Any function belonging to the public interface starts with a capitalized
 * letter. The two main functions of the public interface are:
 *
 * * int Add(double)
 * * void Link(int, int, double)
 *
 * The first method adds a location to the graph and returns a unique integer
 * location handle that can be used to link locations using Link(). The handle
 * of the first location is 1, remaining location handles are generated by
 * incrementing a counter up by 1, i.e., [1, 2, 3, 4, 5, ...].
 *
 * Location handle Graph::ST = 0 is reserved for the graphs source and sink
 * node, which share a common handle in the public interface. Creating a link
 * originating at Graph::ST creates an edge starting at the source. Creating a
 * link pointing to Graph::ST creates and edge ending at the sink.
 *
 * Internal Interface
 * ------------------
 *
 * Internally the graph creates two nodes for each location, one entry node and
 * one exit node. These are connected by a a single edge with cost
 * corresponding to the location's observation cost (see model description
 * above). Consequently, the node structure holds twice as many nodes as
 * locations seen in the public interface.
 *
 * Converting between public location handles and internal node indices is
 * straight forward: Given a node handle i, the entry node of the corresponding
 * location has index i % 2 and the corresponding exit node has
 * index i % 2 + 1. Fruther, note that
 *
 *     Graph::ST % 2 + 0 = Graph::InternalSinkNode,
 *     Graph::ST % 2 + 1 = Graph::InternalSourceNode.
 *
 * This may seem unusual at first, but is the intended behavior as it makes the
 * mapping from/to Graph::ST in Link() straight forward.
 */
class Graph {
 public:
  // Public interface, used to modify the graph.

  static const int ST = 0;  //!< One common id for source and sink.

  //! Empty constructor.
  Graph();

  //! Reserve space for num_edges edges.
  void Reserve(int num_edges);

  /**
   * Add a location to the graph.
   *
   * @param cost Observation edge cost (see class description).
   * @return A unique location handle that can be used to link locations.
   */
  int Add(double cost);

  /**
   * Link two locations.
   *
   * @param src location handle of the source location.
   * @param dst location handle of the target location.
   * @param cost Transition edge cost (see class description).
   */
  void Link(int src, int dst, double cost);

  // Internal interface, used by algorithms.

  static const int InternalSourceNode;      //!< == 1
  static const int InternalSinkNode;        //!< == 0
  static const int FirstNonSourceSinkNode;  //!< == 2

  //! @return Immutable list of edges refering to the internal graph structure.
  const std::vector<Edge>& edges() const { return edges_; }

  //! @return Mutable list of edges refering to the internal graph structure.
  std::vector<Edge>& mutable_edges() { return edges_; }

  //! @return Total number of internal nodes (= twice the number of locations).
  int num_nodes() const { return next_id_; }

  /**
   * Overwrite the number of nodes in the graph.
   *
   * @note The number of nodes is incremented when locations are added to the
   *       graph. Therefore, this function must only be called when the
   *       internal graph structure has been modified. Be considerate about
   *       calling this function, no checks are performed to validate the
   *       integrity of the graph.
   *
   * @param num_nodes New number of nodes in the graph, including source and
   *        sink node.
   */
  void overwrite_num_nodes(const int num_nodes) { next_id_ = num_nodes; }

 protected:
  std::vector<Edge> edges_;
  int next_id_;
};

}  // namespace mcf

#endif