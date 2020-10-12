#ifndef NETWORK_H
#define NETWORK_H

enum NetwtorkType {
    REGTEST,
    TESTNET,
    MAINNET,
};

class Network
{
    public:
    Network(NetwtorkType network=REGTEST);
    String as_string();
    void set_network(NetwtorkType type);
    NetwtorkType get_network();

    private:
    NetwtorkType _type;
};

extern Network network;

#endif
