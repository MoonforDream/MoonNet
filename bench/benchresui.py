import matplotlib
matplotlib.use('Agg')  # Use a non-interactive backend
import matplotlib.pyplot as plt
import numpy as np

# 设置字体，确保支持中文字符
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK SC']  # 使用思源黑体 Simplified Chinese
plt.rcParams['axes.unicode_minus'] = False  # 解决坐标轴负号显示问题

# 数据
message_sizes = ['1024 字节', '4096 字节', '8192 字节']
moonnet_dynamic_throughput = [119.28, 477.82, 914.60]  # MoonNet 动态均衡 (2-4线程变化)
moonnet_static_throughput = [102.06, 373.76, 669.08]   # MoonNet 静态均衡
muduo_throughput = [97.51, 355.86, 632.54]               # Muduo
libevent_throughput = [95.22, 281.47, 561.58]            # libevent

# 设置柱状图的宽度和位置
x = np.arange(len(message_sizes))  # label locations
width = 0.2  # 每个柱的宽度

# 创建图表
fig, ax = plt.subplots(figsize=(12, 7))

# 绘制柱状图
rects1 = ax.bar(x - 1.5*width, moonnet_dynamic_throughput, width, label='MoonNet (动态均衡, 2-4线程)', color='skyblue')
rects2 = ax.bar(x - 0.5*width, moonnet_static_throughput, width, label='MoonNet (静态均衡)', color='lightgreen')
rects3 = ax.bar(x + 0.5*width, muduo_throughput, width, label='Muduo', color='salmon')
rects4 = ax.bar(x + 1.5*width, libevent_throughput, width, label='libevent', color='gold')

# 添加一些文本标签
ax.set_xlabel('消息大小', fontsize=14)
ax.set_ylabel('吞吐量 (MiB/s)', fontsize=14)
ax.set_title('不同消息大小下的吞吐量比较（1000 并发，10 分钟）', fontsize=16)
ax.set_xticks(x)
ax.set_xticklabels(message_sizes, fontsize=12)
ax.legend(fontsize=12)

# 在柱状图上显示数值
def autolabel(rects):
    """在每个柱子上方添加数值标签"""
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height:.2f}',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 垂直偏移量
                    textcoords="offset points",
                    ha='center', va='bottom',
                    fontsize=10)

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)
autolabel(rects4)

plt.tight_layout()
# 保存图表到文件
plt.savefig('throughput_comparison_updated.png')
