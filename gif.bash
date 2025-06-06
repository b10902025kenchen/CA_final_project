#!/usr/bin/env bash
set -e
cd ./plot

# 批次產生 png
for f in $(ls [0-9]* | sort -V); do
    gnuplot "$f"
done


pngs=$(ls [0-9]*.png | sort -V)   # 找所有 png
last_png=$(ls [0-9]*.png | sort -V | tail -n 1)

[ -z "$pngs" ] && { echo "no PNG to use"; exit 1; }

convert -delay 10 -layers Optimize $pngs ../movie.gif


echo "movie.gif done"

mv "$last_png" ../legalized.png