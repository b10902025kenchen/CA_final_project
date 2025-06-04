#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
將 ./plot/placement_graph_*.txt 依數字順序讀入，畫成矩形佈局 PNG。
每個 txt -> 一張 PNG；圖片存到 ./plot/png/。
usage: python make_placement_png.py
"""

import re
import glob
import os
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from natsort import natsorted        # pip install natsort

# === 1. 收集檔名並依序排序 =============================================
txt_paths = natsorted(
    glob.glob("./plot/placement_graph_*.txt"),
    key=lambda p: int(re.search(r"placement_graph_(\d+)\.txt", p).group(1))
)

if not txt_paths:
    raise FileNotFoundError("找不到任何 placement_graph_*.txt 檔案")

# === 2. 預掃描找整體視窗範圍 (axis limits) ==============================
xmin = ymin =  float("inf")
xmax = ymax = -float("inf")

def update_bbox(x0, y0, x1, h):
    global xmin, xmax, ymin, ymax
    xmin = min(xmin, x0)
    xmax = max(xmax, x1)
    ymin = min(ymin, y0)
    ymax = max(ymax, y0 + h)

for path in txt_paths:
    with open(path) as f:
        for line in f:
            if line.strip():
                _, x0, y0, x1, h = line.split()
                update_bbox(float(x0), float(y0), float(x1), float(h))

# 加邊界 padding
pad = max(xmax - xmin, ymax - ymin) * 0.02
xmin, xmax = xmin - pad, xmax + pad
ymin, ymax = ymin - pad, ymax + pad

# 輸出資料夾
out_dir = "./plot/png"
os.makedirs(out_dir, exist_ok=True)

# === 3. 逐檔畫圖並輸出 PNG ==============================================
dpi = 150
figsize = ((xmax - xmin) / dpi, (ymax - ymin) / dpi)

for idx, path in enumerate(txt_paths, 1):
    fig, ax = plt.subplots(figsize=figsize, dpi=dpi)

    with open(path) as f:
        for line in f:
            if line.strip():
                name, x0, y0, x1, h = line.split()
                x0, y0, x1, h = map(float, (x0, y0, x1, h))
                rect = Rectangle(
                    (x0, y0), x1 - x0, h,
                    edgecolor="k", facecolor="skyblue",
                    linewidth=0.5, alpha=0.4          # 半透明
                )
                ax.add_patch(rect)

    ax.set_title(f"Frame {idx}", fontsize=8)
    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    ax.set_aspect("equal", adjustable="box")
    ax.axis("off")

    png_path = os.path.join(out_dir,
                            f"placement_graph_{idx:03}.png")
    plt.savefig(png_path, bbox_inches=None, pad_inches=0)
    plt.close(fig)
    print(f"✓  已輸出 {png_path}")

print(f"全部完成，共 {len(txt_paths)} 張 PNG 存於 {out_dir}/")

