#ifndef LIGHTGBM_NETWORK_H_
#define LIGHTGBM_NETWORK_H_

#include <LightGBM/utils/log.h>

#include <LightGBM/meta.h>
#include <LightGBM/config.h>

#include <functional>
#include <vector>

namespace LightGBM {

/*! \brief forward declaration */
class Linkers;

/*! \brief The network structure for all gather */
class BruckMap {
public:
  /*! \brief The communication times for one all gather operation */
  int k;
  /*! \brief in_ranks[i] means the incomming rank on i-th communication */
  std::vector<int> in_ranks;
  /*! \brief out_ranks[i] means the out rank on i-th communication */
  std::vector<int> out_ranks;
  BruckMap();
  explicit BruckMap(int n);
  /*!
  * \brief Create the object of bruck map
  * \param rank Rank of this machine
  * \param num_machines The total number of machines
  * \return The object of bruck map
  */
  static BruckMap Construct(int rank, int num_machines);
};


/*!
* \brief node type on recursive halving algorithm
* When number of machines is not power of 2, need group maiches into power of 2 group.
* And we can let each group has at most 2 machines.
* if the group only has 1 machine. this machine is the normal node
* if the grou has 2 machines, this group will have two type of nodes, one is the leader.
* leader will represent this group and communication with others.
*/
enum RecursiveHalvingNodeType {
  Normal,  // normal node, 1 group only have 1 machine
  GroupLeader,  // leader of group when number of machines in this group is 2.
  Other  // non-leader machines in group
};

/*! \brief Network structure for recursive halving algorithm */
class RecursiveHalvingMap {
public:
  /*! \brief Communication times for one recursize halving algorithm  */
  int k;
  /*! \brief Node type */
  RecursiveHalvingNodeType type;
  /*! \brief Neighbor, only used for non-normal node*/
  int neighbor;
  /*! \brief ranks[i] means the machines that will communicate with on i-th communication*/
  std::vector<int> ranks;
  /*! \brief  send_block_start[i] means send block start index at i-th communication*/
  std::vector<int> send_block_start;
  /*! \brief  send_block_start[i] means send block size at i-th communication*/
  std::vector<int> send_block_len;
  /*! \brief  send_block_start[i] means recv block start index at i-th communication*/
  std::vector<int> recv_block_start;
  /*! \brief  send_block_start[i] means recv block size  at i-th communication*/
  std::vector<int> recv_block_len;

  RecursiveHalvingMap();

  RecursiveHalvingMap(RecursiveHalvingNodeType _type, int n);

  /*!
  * \brief Create the object of recursive halving map
  * \param rank Rank of this machine
  * \param num_machines The total number of machines
  * \return The object of recursive halving map
  */
  static RecursiveHalvingMap Construct(int rank, int num_machines);
};

/*! \brief A static class that contains some collective communication algorithm */
class Network {
public:
  /*!
  * \brief Initialize
  * \param config Config of network setting
  */
  static void Init(NetworkConfig config);
  /*! \brief Free this static class */
  static void Dispose();
  /*! \brief Get rank of this machine */
  static inline int rank();
  /*! \brief Get total number of machines */
  static inline int num_machines();

  /*!
  * \brief Perform all reduce. if data size is small,
           will perform AllreduceByAllGather, else with call ReduceScatter followed allgather
  * \param input Input data
  * \param input_size The size of input data
  * \param type_size The size of one object in the reduce function
  * \param output Output result
  * \param reducer Reduce function
  */
  static void Allreduce(char* input, int input_size, int type_size,
    char* output, const ReduceFunction& reducer);

  /*!
  * \brief Perform all reduce, use all gather. When data is small, can use this to reduce communication times
  * \param input Input data
  * \param input_size The size of input data
  * \param output Output result
  * \param reducer Reduce function
  */
  static void AllreduceByAllGather(char* input, int input_size, char* output,
    const ReduceFunction& reducer);

  /*!
  * \brief Perform all gather, use bruck algorithm. Communication times is O(log(n)), and communication cost is O(send_size * number_machine)
  * if all machine have same input size, can call this function
  * \param input Input data
  * \param send_size The size of input data
  * \param output Output result
  */
  static void Allgather(char* input, int send_size, char* output);

  /*!
  * \brief Perform all gather, use bruck algorithm. Communication times is O(log(n)), and communication cost is O(all_size)
  * if all machine have different input size, can call this function
  * \param input Input data
  * \param all_size The size of input data
  * \param block_start The block start for different machines
  * \param block_len The block size for different machines
  * \param output Output result
  */
  static void Allgather(char* input, int all_size, int* block_start,
    int* block_len, char* output);

  /*!
  * \brief Perform reduce scatter, use recursive halving algorithm. Communication times is O(log(n)), and communication cost is O(input_size)
  * \param input Input data
  * \param input_size The size of input data
  * \param block_start The block start for different machines
  * \param block_len The block size for different machines
  * \param output Output result
  * \param reducer Reduce function
  */
  static void ReduceScatter(char* input, int input_size,
    int* block_start, int* block_len, char* output,
    const ReduceFunction& reducer);

private:
  /*! \brief Number of all machines */
  static int num_machines_;
  /*! \brief Rank of local machine */
  static int rank_;
  /*! \brief The network interface, provide send/recv functions  */
  static Linkers *linkers_;
  /*! \brief Bruck map for all gather algorithm*/
  static BruckMap bruck_map_;
  /*! \brief Recursive halving map for reduce scatter */
  static RecursiveHalvingMap recursive_halving_map_;
  /*! \brief Buffer to store block start index */
  static int* block_start_;
  /*! \brief Buffer to store block size */
  static int* block_len_;
  /*! \brief Buffer  */
  static char* buffer_;
  /*! \brief Size of buffer_ */
  static int buffer_size_;
};

inline int Network::rank() {
  return rank_;
}

inline int Network::num_machines() {
  return num_machines_;
}

}  // namespace LightGBM

#endif   // LightGBM_NETWORK_H_
