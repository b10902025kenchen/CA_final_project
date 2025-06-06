#!/usr/bin/env bash
set -e
cd ./plot

# 批次產生 png
for f in $(ls [0-9]* | sort -V); do
    gnuplot "$f"
done


pngs=$(ls [0-9]*.png | sort -V)   # 找所有 png

# 先確定有找到
[ -z "$pngs" ] && { echo "❌ 沒有 PNG 檔可用！"; exit 1; }

convert -delay 10 -loop 0 -layers Optimize $pngs ../movie.gif

echo "✅  已完成 movie.gif"


rm ./*