# Lambda Feeddown Analysis - 快速开始

## 3分钟上手指南

### Step 1: 同步代码到集群 (本地执行)

```bash
cd /Users/wangchunzheng/works/Models/cve_ampt_light
./sync_cve_ampt_light.sh
```

### Step 2: 登录集群并编译 (集群执行)

```bash
ssh wangchunzheng@10.155.131.127
cd /storage/fdunphome/wangchunzheng/cve_ampt_light

# 编译Lambda feeddown分析程序
make feeddown

# 验证编译成功
./analysis_lambda_feeddown
# 应该输出：Usage: ./analysis_lambda_feeddown <input.root|.list> <output.root>
```

### Step 3: 准备输入文件列表 (集群执行)

```bash
# 方法1：使用现有的beforeART数据
ls /path/to/beforeART_data/*.root > joblist_lambda_feeddown.txt

# 方法2：使用afterART数据
ls /path/to/afterART_data/*.root > joblist_lambda_feeddown.txt

# 方法3：手动创建（小规模测试）
cat > joblist_lambda_feeddown.txt <<EOF
/path/to/test_file_1.root
/path/to/test_file_2.root
EOF

# 检查joblist
head joblist_lambda_feeddown.txt
wc -l joblist_lambda_feeddown.txt
```

### Step 4: 提交任务 (集群执行)

```bash
cd lambda_feeddown
condor_submit submit.sub
```

### Step 5: 监控任务 (集群执行)

```bash
# 查看任务队列
condor_q

# 实时查看第一个任务的输出
tail -f outputs/job_0.out

# 查看所有任务的状态（每5秒更新）
watch -n 5 condor_q
```

## 任务完成后

### 查看输出

```bash
# 列出所有输出文件
ls -lh outputs/feeddown_results_*.root

# 检查某个输出文件的内容
root -l outputs/feeddown_results_0.root
# 在ROOT中执行：
# .ls
# EventData->Print()
# .q
```

### 合并结果

```bash
cd /storage/fdunphome/wangchunzheng/cve_ampt_light/lambda_feeddown

# 合并所有输出
hadd -f merged_feeddown.root outputs/feeddown_results_*.root

# 检查合并后的文件
root -l merged_feeddown.root
# .ls
# Delta_lambda_lambda->Draw()
```

### 计算Ratio（使用Bootstrap）

```bash
cd /storage/fdunphome/wangchunzheng/cve_ampt_light

# 运行Bootstrap分析
python scripts/bootstrap_ratio.py \
    lambda_feeddown/merged_feeddown.root \
    lambda_feeddown/bootstrap_results.root \
    --n-bootstrap 1000
```

### 绘制图表

```bash
# 绘制ratio图
python scripts/lambda_feeddown_ratio.py \
    lambda_feeddown/merged_feeddown.root \
    lambda_feeddown/ratio_plot.pdf
```

## 常见问题

### Q1: 任务一直在队列中不运行？
```bash
# 查看任务详情
condor_q -better-analyze <job_id>

# 检查系统资源
condor_status
```

### Q2: 任务失败了怎么办？
```bash
# 查看错误日志
cat outputs/job_0.err

# 查看输出日志
cat outputs/job_0.out

# 常见原因：
# - 输入文件不存在
# - 可执行文件未编译
# - conda环境问题
```

### Q3: 如何取消任务？
```bash
# 取消特定任务
condor_rm <job_id>

# 取消所有自己的任务
condor_rm <username>
```

### Q4: 如何本地测试？
```bash
# 在集群上本地运行（不通过condor）
cd /storage/fdunphome/wangchunzheng/cve_ampt_light
./analysis_lambda_feeddown /path/to/test.root test_output.root

# 检查输出
root -l test_output.root
```

## 分析结果解读

输出ROOT文件包含：

1. **TProfiles** (关联函数 vs 中心度)
   - `Delta_lambda_lambda`: Λ-Λ Delta关联（分母）
   - `Delta_pFromLambda_vs_Lambda`: p(from Λ)-Λ Delta关联（分子）
   - 对应的Gamma关联

2. **TTree: EventData** (用于Bootstrap)
   - `centrality`: 中心度bin (0-9)
   - `n_lambda`: 事件中的Lambda数量
   - `delta_lambda_lambda`: 该事件的平均Λ-Λ关联
   - `delta_pFromLambda`: 该事件的平均p(from Λ)-Λ关联
   - `npairs_*`: 粒子对数量（用于加权）

3. **Ratio计算**
   ```
   Feeddown Ratio = <p from Lambda - Lambda> / <Lambda - Lambda>
   ```

## 下一步

- 详细使用说明：查看 `README.md`
- Bootstrap方法：查看 `../scripts/bootstrap_ratio.py`
- 绘图脚本：查看 `../scripts/lambda_feeddown_ratio.py`

## 实验参数

- **Lambda pt cut**: 1.0 GeV/c (提高流信号)
- **衰变质子 pt cut**: 0.7 GeV/c (筛选高pt质子)
- **Lambda |η| < 0.8, |y| < 0.5**
- **每个Lambda只衰变一次**（物理上正确）
