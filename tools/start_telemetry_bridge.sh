#!/bin/bash
#===========================================
# tools/start_telemetry_bridge.sh
#
# Opens the SSH reverse tunnel + local TCP->UDP bridge needed
# for UDP telemetry to reach this machine's QUdpSocket from Kali.
# WSL2's default NAT blocks direct inbound UDP, so this bridges it.
#
# The Kali-side scripts (udp_telemetry_generator.sh, etc.) are
# run manually, in their own terminals on Kali -- this script
# does not launch them remotely.
#
# Run FROM WSL. Press Ctrl+C once to stop.
#===========================================

KALI_USER="belibung"
KALI_IP="192.168.1.131"
TUNNEL_PORT=9000
UDP_HMI_PORT=5000

KALI_HOST="${KALI_USER}@${KALI_IP}"

trap 'echo "Stopping bridge..."; kill $(jobs -p) 2>/dev/null' EXIT

echo "[1/2] Opening reverse SSH tunnel..."
ssh -N -R "$TUNNEL_PORT:127.0.0.1:$TUNNEL_PORT" "$KALI_HOST" &
sleep 2

echo "[2/2] Starting local TCP->UDP bridge..."
socat TCP-LISTEN:$TUNNEL_PORT,reuseaddr,fork UDP:127.0.0.1:$UDP_HMI_PORT 2>&1 | sed -u 's/^/[BRIDGE] /' &

echo "Bridge up. Press Ctrl+C to stop."
wait