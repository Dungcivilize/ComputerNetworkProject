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

    if [ "$BACKGROUND" == "true" ]; then
        ip netns exec $ns ./test_sensor $PORT sensor_${i} name_${i} password_${i} &
    else
        gnome-terminal --title="TestSensor ns${i}testsensor ($ipaddr:$PORT)" -- bash -c "ip netns exec $ns ./test_sensor $PORT sensor_${i} name_${i} password_${i}; exec bash"
    fi
done

echo "Launched $COUNT sensors. Example client scan: ./tests/test_sensor_client 192.168.1 2 $((COUNT+1)) $PORT 1"

echo ""
echo "✅ Đã khởi tạo xong $i namespace(s) cho test"
echo "📝 Nhập 'exit' để dọn dẹp và thoát..."
echo ""

# Chờ lệnh exit
while true; do
    read -p "> " cmd
    if [ "$cmd" == "exit" ]; then
        break
    fi
done

# Dọn dẹp
echo ""
echo "🧹 Đang dọn dẹp..."

# Kill tất cả process trong namespaces cùng lúc
for i in $(seq 1 $COUNT); do
    ns="ns${i}testsensor"
    if ip netns list | grep "^$ns\b"; then
        ip netns pids $ns 2>/dev/null | xargs -r kill -9 2>/dev/null &
    fi
done

# Tắt tất cả terminals
for i in $(seq 1 $COUNT); do
    ns="ns${i}testsensor"
    kill $(pgrep -f "TestSensor $ns") 2>/dev/null &
done

# Xóa tất cả veth pairs
for i in $(seq 1 $COUNT); do
    ip link delete veth-test${i}-br 2>/dev/null &
done

# Xóa tất cả namespaces
for i in $(seq 1 $COUNT); do
    ns="ns${i}testsensor"
    ip netns delete $ns 2>/dev/null &
    rm -f /run/netns/$ns 2>/dev/null &
    echo "  ✓ Đã xóa $ns" &
done &

# Xóa bridge (chạy sau khi các background hoàn tất)
wait
if ip link show $BRIDGE >/dev/null 2>&1; then
    ip link set $BRIDGE down
    ip link delete $BRIDGE
    echo "  ✓ Đã xóa bridge $BRIDGE"
fi

echo ""
echo "✅ Dọn dẹp hoàn tất!"