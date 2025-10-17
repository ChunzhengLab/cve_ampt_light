#!/bin/bash

# sync_cve_ampt_light.sh
# ----------------------
# 同步本地目录到远程服务器的目标目录
# Usage: ./sync_cve_ampt_light.sh [--dry-run]

# 配置部分
LOCAL_DIR="/Users/wangchunzheng/works/Models/cve_ampt_light"
REMOTE_USER="wangchunzheng"
REMOTE_HOST="10.155.131.127"
REMOTE_DIR="/storage/fdunphome/wangchunzheng/cve_ampt_light"

# 可选：开启 dry-run 模式进行预览
DRY_RUN=""
if [[ "$1" == "--dry-run" ]]; then
    DRY_RUN="--dry-run"
    echo "[DRY RUN] 仅预览同步操作，不会执行实际传输"
fi

# 执行 rsync
echo "同步: $LOCAL_DIR --> ${REMOTE_USER}@${REMOTE_HOST}:$REMOTE_DIR"
rsync -avz $DRY_RUN \
      --delete \
      --exclude='*.root' \
      --exclude='*.csv' \
      --exclude='*.pdf' \
      --exclude='scripts/**' \
      -e "ssh" \
      "$LOCAL_DIR/" \
      "$REMOTE_USER@${REMOTE_HOST}:${REMOTE_DIR}/"

if [[ -z "$DRY_RUN" ]]; then
    echo "同步完成: $(date)"
else
    echo "Dry-run 完成: $(date)"
fi
