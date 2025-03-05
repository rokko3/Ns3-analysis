import pyshark
import sys

def analyze_pcap(pcap_file):
    cap = pyshark.FileCapture(pcap_file)
    timestamps = []
    retransmissions = 0
    lost_packets = 0
    last_seq = {}  # Diccionario para rastrear secuencias por IP

    for packet in cap:
        try:
            # Extraer timestamp
            timestamp = float(packet.sniff_time.timestamp())
            timestamps.append(timestamp)
            
            # Extraer información TCP
            if 'TCP' in packet:
                src_ip = packet.ip.src
                seq_num = int(packet.tcp.seq)
                
                # Detectar retransmisión
                if hasattr(packet.tcp, 'analysis_retransmission'):
                    retransmissions += 1
                
                # Detectar pérdida de paquetes basándonos en la secuencia
                if src_ip in last_seq and seq_num < last_seq[src_ip]:
                    lost_packets += 1
                
                last_seq[src_ip] = seq_num
        except Exception as e:
            pass
    
    if timestamps:
        avg_delay = sum(timestamps[i+1] - timestamps[i] for i in range(len(timestamps)-1)) / len(timestamps)
    else:
        avg_delay = 0
    
    print("Análisis de Congestión:")
    print(f"Retransmisiones detectadas: {retransmissions}")
    print(f"Posibles paquetes perdidos: {lost_packets}")
    print(f"Retardo promedio entre paquetes: {avg_delay:.6f} segundos")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python analyze_pcap.py archivo.pcap")
    else:
        analyze_pcap(sys.argv[1])
