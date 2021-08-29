#ifndef KNET_NODE_H
#define KNET_NODE_H

#include "../util.h"

struct node_info {
    int port;
    int threadQuantity;
};

struct node {
    struct node_info nodeInfo;
};

enum node_info_type { Port, ThreadQuantity };

struct node_info_pair {
    enum node_info_type nodeInfoType;
    void *value;
};

int node_start( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] );
struct node_info create_node_info( int nodeInfoPairCounter, struct node_info_pair nodeInfoPairVector[] );
struct node_info_pair create_node_info_pair( enum node_info_type nodeInfoType, void *value );

#endif // KNET_NODE_H
