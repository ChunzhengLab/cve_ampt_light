# Lambda Feeddown Analysis

专门用于分析Lambda粒子衰变产物（p和π）与Lambda的关联函数。

## 分析内容

该程序计算：
1. **Lambda-Lambda correlation** (分母)
2. **p from Lambda - Lambda correlation** (分子)
3. **π from Lambda - Lambda correlation** (分子)

输出包含：
- TProfile：Delta/Gamma关联函数 vs 中心度
- TTree：event-by-event数据用于Bootstrap误差分析

## 筛选条件

- **Lambda**: pt > 1.0 GeV/c, |η| < 0.8, |y| < 0.5
- **衰变质子**: pt > 0.7 GeV/c (用于p from Lambda correlation)
- **衰变π**: 无额外pt cut

## 使用方法

### 1. 本地测试

```bash
# 编译
make feeddown

# 运行单个文件
./analysis_lambda_feeddown input.root output.root
```

### 2. 集群提交

#### 步骤1：准备输入文件列表

创建 `joblist_lambda_feeddown.txt`，每行一个ROOT文件路径：

```
/path/to/ampt_data_001.root
/path/to/ampt_data_002.root
/path/to/ampt_data_003.root
```

#### 步骤2：同步代码到集群

```bash
# 在本地运行
./sync_cve_ampt_light.sh
```

#### 步骤3：登录集群并编译

```bash
ssh wangchunzheng@10.155.131.127
cd /storage/fdunphome/wangchunzheng/cve_ampt_light

# 编译feeddown分析程序
make feeddown

# 确认可执行文件存在
ls -lh analysis_lambda_feeddown
```

#### 步骤4：准备joblist

```bash
# 创建输入文件列表
# 例如：使用beforeART的数据
ls /path/to/beforeART_data/*.root > joblist_lambda_feeddown.txt

# 或者使用afterART的数据
ls /path/to/afterART_data/*.root > joblist_lambda_feeddown.txt
```

#### 步骤5：提交任务

```bash
cd lambda_feeddown
condor_submit submit.sub
```

#### 步骤6：监控任务

```bash
# 查看任务状态
condor_q

# 查看特定任务的详细信息
condor_q -nobatch

# 查看日志（实时）
tail -f outputs/job_0.out

# 查看所有任务的状态
watch -n 5 condor_q
```

#### 步骤7：任务管理

```bash
# 移除特定任务
condor_rm <job_id>

# 移除所有自己的任务
condor_rm <username>

# 查看任务历史
condor_history <job_id>
```

## 输出文件

任务完成后，输出文件位于：
```
lambda_feeddown/outputs/feeddown_results_<JOB_ID>.root
```

每个输出文件包含：
- `Delta_lambda_lambda`: Lambda-Lambda Delta关联
- `Gamma_lambda_lambda`: Lambda-Lambda Gamma关联
- `Delta_pFromLambda_vs_Lambda`: p from Lambda Delta关联
- `Gamma_pFromLambda_vs_Lambda`: p from Lambda Gamma关联
- `Delta_piFromLambda_vs_Lambda`: π from Lambda Delta关联
- `Gamma_piFromLambda_vs_Lambda`: π from Lambda Gamma关联
- `EventData`: TTree包含event-by-event数据

## 后续分析

### 合并多个输出文件

```bash
hadd -f merged_feeddown.root outputs/feeddown_results_*.root
```

### 计算ratio并使用Bootstrap方法

```bash
python scripts/bootstrap_ratio.py merged_feeddown.root bootstrap_results.root --n-bootstrap 1000
```

### 绘制ratio图

```bash
python scripts/lambda_feeddown_ratio.py merged_feeddown.root ratio_plot.pdf
```

## 目录结构

```
lambda_feeddown/
├── run.sh              # 运行脚本
├── submit.sub          # HTCondor提交脚本
├── outputs/            # 输出目录
│   ├── feeddown_results_0.root
│   ├── feeddown_results_1.root
│   ├── job_0.out
│   ├── job_0.err
│   └── job_0.log
└── README.md           # 使用说明（本文件）
```

## 与原始分析的区别

| 特性 | analysis_cve | analysis_lambda_feeddown |
|-----|--------------|--------------------------|
| 分析对象 | 所有粒子对 | 只有Lambda相关 |
| 输出 | 所有粒子对关联 | Lambda feeddown专用 |
| pt cut | Lambda: 0.2 | Lambda: 1.0, p: 0.7 |
| Bootstrap支持 | 有 | 有 |
| 衰变模拟 | 有 | 优化版本 |

## 注意事项

1. **确保编译后的可执行文件在集群上**
2. **joblist中的文件路径必须是集群上的绝对路径**
3. **输出目录会自动创建**
4. **任务失败时检查.err文件**
5. **质子和π来自同一次衰变（物理上正确）**

## 故障排除

### 问题1：找不到可执行文件
```bash
# 检查编译是否成功
make feeddown
ls -lh analysis_lambda_feeddown
```

### 问题2：ROOT库找不到
```bash
# 检查conda环境
conda activate cpp_dev
which root
root-config --libs
```

### 问题3：输入文件找不到
```bash
# 检查joblist中的路径
head joblist_lambda_feeddown.txt
# 确保路径存在
ls -lh /path/from/joblist/file.root
```

## 联系方式

如有问题，请检查：
1. HTCondor日志文件 (.log, .out, .err)
2. 编译是否成功
3. 输入文件是否存在
