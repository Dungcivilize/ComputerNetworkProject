#!/bin/bash

# Simple test helper: spawn N sensor instances in network namespaces (requires root)
# Usage: ./test_sensor.sh [PORT] [COUNT] [BACKGROUND]

PORT=${1:-9000}
COUNT=${2:-5}
BACKGROUND=${3:-true}

BASE_IP=$(hostname -I | awk '{print $1}')
IFS='.' read -r a b c d <<< "$BASE_IP"

BRIDGE="br0-test"

echo "Starting $COUNT sensor namespaces on base port $PORT (bridge $BRIDGE)"

if [ "$EUID" -ne 0 ]; then
    echo "This script requires root (netns). Run with sudo.";
    exit 1;
fi

ip link show $BRIDGE >/dev/null 2>&1 || {
    ip link add name $BRIDGE type bridge
    ip addr add ${a}.${b}.${c}.254/24 dev $BRIDGE
    ip link set $BRIDGE up
}

host_id=1
for i in $(seq 1 $COUNT); do
    ns="ns${i}testsensor"
    ipaddr="${a}.${b}.${c}.$((i+1))"
    echo "Create $ns with IP $ipaddr"
    ip netns add $ns
    ip link add veth-test${i} type veth peer name veth-test${i}-br
    ip link set veth-test${i} netns $ns
    ip netns exec $ns ip addr add $ipaddr/24 dev veth-test${i}
    ip netns exec $ns ip link set veth-test${i} up
    ip netns exec $ns ip link set lo up
    ip link set veth-test${i}-br master $BRIDGE
    ip link set veth-test${i}-br up

    sensor_port=$((PORT + i - 1))
    if [ "$BACKGROUND" == "true" ]; then
        ip netns exec $ns ./tests/test_sensor $sensor_port sensor_${i} name_${i} password_${i} &
    else
        ip netns exec $ns ./tests/test_sensor $sensor_port sensor_${i} name_${i} password_${i}
    fi
done

echo "Launched $COUNT sensors. Example client scan: ./tests/test_sensor_client 192.168.1 2 $((COUNT+1)) $PORT 1"
