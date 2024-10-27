import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

# 设置字体，确保支持中文字符
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK SC']  # 使用思源黑体 Simplified Chinese
plt.rcParams['axes.unicode_minus'] = False  # 解决坐标轴负号显示问题

# 数据
message_sizes = ['1024 字节', '4096 字节', '8192 字节']
moonnet_throughput = [102.06, 373.76, 669.08]
muduo_throughput = [97.51, 355.86, 632.54]
libevent_throughput = [95.22, 281.47, 561.58]

# 设置柱状图的宽度和位置
x = np.arange(len(message_sizes))
width = 0.25  # 每个柱的宽度

# 创建图表
fig, ax = plt.subplots(figsize=(10, 6))

rects1 = ax.bar(x - width, moonnet_throughput, width, label='MoonNet')
rects2 = ax.bar(x, muduo_throughput, width, label='muduo')
rects3 = ax.bar(x + width, libevent_throughput, width, label='Libevent')

# 添加一些文本标签
ax.set_xlabel('消息大小')
ax.set_ylabel('吞吐量 (MiB/s)')
ax.set_title('不同消息大小下的吞吐量比较（1000 并发，10 分钟）')
ax.set_xticks(x)
ax.set_xticklabels(message_sizes)
ax.legend()

# 在柱状图上显示数值
def autolabel(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height:.2f}',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 偏移量
                    textcoords="offset points",
                    ha='center', va='bottom')

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)

plt.tight_layout()
# 保存图表到文件
plt.savefig('throughput_comparison.png')
