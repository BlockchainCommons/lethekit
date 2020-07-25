#include "network.h"

Network network = Network();

Network::Network(NetwtorkType network) {
   _type = network;
}


String Network::as_string() {
    switch(_type) {
        case REGTEST:
            return "Regtest";
        case TESTNET:
            return "Testnet";
        case MAINNET:
            return "Mainnet";
        default:
            return "Regtest";
    }
}


void Network::set_network(NetwtorkType type) {
    _type = type;
}


NetwtorkType Network::get_network() {
    return _type;
}
