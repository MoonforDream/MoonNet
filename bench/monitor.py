#!/usr/bin/env python3
# monitor.py

import psutil
import argparse
import time
import datetime
import pandas as pd
import sys
import os

def find_process_by_port(port):
    """
    Find the process ID (PID) listening on the specified port.

    Args:
        port (int): The port number to check.

    Returns:
        list: A list of psutil.Process objects listening on the port.
    """
    processes = []
    for proc in psutil.process_iter(['pid', 'name']):
        try:
            connections = proc.connections(kind='inet')
            for conn in connections:
                if conn.status == psutil.CONN_LISTEN and conn.laddr.port == port:
                    processes.append(proc)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    return processes

def monitor_process(proc, duration, interval, output_dir):
    """
    Monitor CPU and memory usage of the given process.

    Args:
        proc (psutil.Process): The process to monitor.
        duration (int): Total monitoring duration in seconds.
        interval (float): Time interval between measurements in seconds.
        output_dir (str): Directory to save logs and data.

    Returns:
        pd.DataFrame: DataFrame containing the recorded metrics.
    """
    timestamps = []
    cpu_usages = []
    memory_usages = []

    end_time = time.time() + duration
    print(f"开始监控进程 '{proc.name()}' (PID: {proc.pid})，持续 {duration} 秒...")

    try:
        while time.time() < end_time:
            try:
                cpu = proc.cpu_percent(interval=None)  # Non-blocking
                mem = proc.memory_info().rss / (1024 * 1024)  # Convert bytes to MB
                current_time = datetime.datetime.now()
                timestamps.append(current_time)
                cpu_usages.append(cpu)
                memory_usages.append(mem)
                print(f"[{current_time.strftime('%H:%M:%S')}] CPU: {cpu:.2f}% | Memory: {mem:.2f} MB")
                time.sleep(interval)
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                print("进程已终止或访问被拒绝。")
                break
    except KeyboardInterrupt:
        print("\n监控被用户中断。")

    data = {
        'Timestamp': timestamps,
        'CPU_Usage_Percent': cpu_usages,
        'Memory_Usage_MB': memory_usages
    }
    df = pd.DataFrame(data)

    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)

    # 保存原始数据到 CSV
    csv_file = os.path.join(output_dir, 'process_metrics.csv')
    df.to_csv(csv_file, index=False)
    print(f"指标已保存到 {csv_file}")

    return df

def calculate_and_print_averages(df, process_name, pid):
    """
    Calculate and print average CPU and memory usage.

    Args:
        df (pd.DataFrame): DataFrame containing the metrics.
        process_name (str): Name of the monitored process.
        pid (int): PID of the monitored process.
    """
    if df.empty:
        print("没有可用的数据来计算平均值。")
        return

    avg_cpu = df['CPU_Usage_Percent'].mean()
    avg_mem = df['Memory_Usage_MB'].mean()

    print(f"\n===== {process_name} (PID: {pid}) 性能统计 =====")
    print(f"平均 CPU 使用率: {avg_cpu:.2f}%")
    print(f"平均内存使用率: {avg_mem:.2f} MB")
    print("==============================================")

def parse_arguments():
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description='监控在指定端口运行的进程的 CPU 和内存使用情况。')
    parser.add_argument('--port', type=int, required=True, help='要监控的端口号。')
    parser.add_argument('--duration', type=int, default=60, help='监控持续时间（秒）。默认是60秒。')
    parser.add_argument('--interval', type=float, default=1.0, help='每次测量的时间间隔（秒）。默认是1秒。')
    parser.add_argument('--output', type=str, default='monitor_output', help='保存日志和数据的目录。默认是 "monitor_output"。')
    return parser.parse_args()

def main():
    args = parse_arguments()

    processes = find_process_by_port(args.port)

    if not processes:
        print(f"没有进程在端口 {args.port} 上监听。")
        sys.exit(1)
    elif len(processes) > 1:
        print(f"多个进程在端口 {args.port} 上监听。将监控第一个找到的进程。")

    proc = processes[0]
    process_name = proc.name()
    pid = proc.pid

    # 初始化 CPU 百分比计算
    proc.cpu_percent(interval=None)

    # 监控进程
    df = monitor_process(proc, args.duration, args.interval, args.output)

    # 计算并打印平均值
    calculate_and_print_averages(df, process_name, pid)

    print("监控完成。")

if __name__ == "__main__":
    main()
