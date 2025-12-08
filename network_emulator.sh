#!/bin/bash

# Nhận tham số từ command line, nếu không có thì dùng giá trị mặc định
PORT=${1:-8080}
COUNT=${2:-100}

# Lấy địa chỉ thật của máy (ví dụ 192.168.1.100)
BASE_IP=$(hostname -I | awk '{print $1}')
IFS='.' read -r a b c d <<< "$BASE_IP"

BRIDGE="br0"

echo "📡 Khởi chạy với PORT=$PORT, COUNT=$COUNT"

# tạo bridge nếu chưa có
ip link show $BRIDGE >/dev/null 2>&1
if [ $? -ne 0 ]; then
    ip link add name $BRIDGE type bridge
    ip addr add ${a}.${b}.${c}.254/24 dev $BRIDGE
    ip link set $BRIDGE up
fi

host_id=0
for i in $(seq 1 $COUNT); do
    ns="ns$i"
    
    # tìm IP khả dụng
    while true; do
        host_id=$((host_id + 1))
        # tránh vượt quá dải IP
        if [ $host_id -gt 253 ]; then
            echo "❌  Không còn IP khả dụng trong dải mạng"
            break
        fi
        ipaddr="${a}.${b}.${c}.${host_id}"
        
        # kiểm tra IP có trùng với IP thật hoặc đã tồn tại không
        if [ "$ipaddr" != "$BASE_IP" ] && ! ip addr show | grep -q "$ipaddr"; then
            break
        fi
        
        echo "⚠️  IP $ipaddr không khả dụng, thử IP tiếp theo"
    done
    if [ $host_id -gt 253 ]; then
        break
    fi
    echo "➡️  Tạo namespace $ns với IP $ipaddr"

    # tạo namespace
    ip netns add $ns

    # tạo veth pair
    ip link add veth${i} type veth peer name veth${i}-br
    ip link set veth${i} netns $ns

    # gán IP cho namespace
    ip netns exec $ns ip addr add $ipaddr/24 dev veth${i}
    ip netns exec $ns ip link set veth${i} up
    ip netns exec $ns ip link set lo up

    # nối veth vào bridge
    ip link set veth${i}-br master $BRIDGE
    ip link set veth${i}-br up

    # chạy chương trình sensor trong namespace
    ip netns exec $ns ./sensor $ipaddr $PORT &
done

echo ""
echo "✅ Đã khởi tạo xong $i namespace(s)"
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

for i in $(seq 1 $COUNT); do
    ns="ns$i"
    
    # Kiểm tra namespace có tồn tại không
    if ip netns list | grep -q "^$ns$"; then
        # Dừng process trong namespace
        ip netns pids $ns 2>/dev/null | xargs -r kill 2>/dev/null
        
        # Xóa veth pair
        ip link delete veth${i}-br 2>/dev/null
        
        # Xóa namespace
        ip netns delete $ns 2>/dev/null
        
        echo "  ✓ Đã xóa $ns"
    fi
done

# Xóa bridge
if ip link show $BRIDGE >/dev/null 2>&1; then
    ip link set $BRIDGE down
    ip link delete $BRIDGE
    echo "  ✓ Đã xóa bridge $BRIDGE"
fi

echo ""
echo "✅ Dọn dẹp hoàn tất!"
