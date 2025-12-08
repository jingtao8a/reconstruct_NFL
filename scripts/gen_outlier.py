
import numpy as np
import os

def generate_large_float_dataset(
        normal_sample_size: int = 199_990_000,  # 正常均匀分布数据量（1.9999亿）
        anomaly_sample_size: int = 10_000, # 异常值数据量（1万）
        normal_range: tuple = (0.0, 100.0),     # 正常数据均匀分布范围
        anomaly_range: tuple = (1e9, 1e12),     # 异常值均匀分布范围（极值）
        output_file: str = "float_dataset.bin", # 输出文件路径
        file_format: str = "binary",            # 输出格式：binary/text
        batch_size: int = 10_000_000            # 分批次生成的批次大小（缓解内存压力）
):
    """
    生成包含正常均匀分布数据+随机异常值的数据集，打乱后写入文件
    兼容所有numpy版本，支持分批次生成以降低内存占用
    """
    # ===================== 1. 分批次生成正常均匀分布数据 =====================
    print("开始分批次生成1亿个正常均匀分布64位浮点数...")
    normal_data = []
    remaining = normal_sample_size
    while remaining > 0:
        current_batch = min(batch_size, remaining)
        # 先生成数据，再显式转换为float64（兼容无dtype参数的numpy版本）
        batch = np.random.uniform(
            low=normal_range[0],
            high=normal_range[1],
            size=current_batch
        ).astype(np.float64)  # 关键修复：显式转换为64位浮点数
        normal_data.append(batch)
        remaining -= current_batch
        print(f"已生成 {normal_sample_size - remaining:,} / {normal_sample_size:,} 个正常数据")
    # 合并批次
    normal_data = np.concatenate(normal_data)
    print(f"正常数据生成完成，数据范围：{normal_data.min():.2f} ~ {normal_data.max():.2f}")

    # ===================== 2. 分批次生成随机异常值 =====================
    print("\n开始分批次生成1亿个随机异常值...")
    anomaly_data = []
    remaining = anomaly_sample_size
    while remaining > 0:
        current_batch = min(batch_size, remaining)
        batch = np.random.uniform(
            low=anomaly_range[0],
            high=anomaly_range[1],
            size=current_batch
        ).astype(np.float64)  # 显式转换为64位浮点数
        anomaly_data.append(batch)
        remaining -= current_batch
        print(f"已生成 {anomaly_sample_size - remaining:,} / {anomaly_sample_size:,} 个异常值")
    # 合并批次
    anomaly_data = np.concatenate(anomaly_data)
    print(f"异常值生成完成，数据范围：{anomaly_data.min():.2e} ~ {anomaly_data.max():.2e}")

    # ===================== 3. 合并并打乱数据 =====================
    print("\n合并并打乱2亿个数据...")
    all_data = np.concatenate([normal_data, anomaly_data])
    # 原地打乱，节省内存
    np.random.shuffle(all_data)
    print("数据打乱完成")

    # ===================== 4. 分块写入本地文件 =====================
    print(f"\n开始写入文件：{output_file}")
    if os.path.exists(output_file):
        os.remove(output_file)  # 覆盖已有文件

    if file_format == "binary":
        # 二进制写入（64位浮点数=8字节/个，2亿个≈1.6GB）
        with open(output_file, "wb") as f:
            chunk_size = 10_000_000  # 每次写入1000万条
            for i in range(0, len(all_data), chunk_size):
                chunk = all_data[i:i+chunk_size]
                chunk.tofile(f)
                print(f"已写入 {min(i+chunk_size, len(all_data)):,} / {len(all_data):,} 条数据")
    elif file_format == "text":
        # 文本格式写入（注意：占用空间约16GB，速度较慢）
        with open(output_file, "w", buffering=1024*1024) as f:  # 1MB缓冲区
            chunk_size = 1_000_000  # 文本分块更小，避免内存溢出
            for i in range(0, len(all_data), chunk_size):
                chunk = all_data[i:i+chunk_size]
                f.write("\n".join(map(str, chunk)) + "\n")
                print(f"已写入 {min(i+chunk_size, len(all_data)):,} / {len(all_data):,} 条数据")
    else:
        raise ValueError("file_format 仅支持 binary 或 text")

    # 验证文件信息
    file_size = os.path.getsize(output_file)
    print("\n===== 生成完成 =====")
    print(f"文件路径：{os.path.abspath(output_file)}")
    print(f"文件大小：{file_size / (1024**3):.2f} GB")
    print(f"总数据量：{len(all_data):,} 个")
    print(f"数据类型：{all_data.dtype}（确认是64位浮点数）")

if __name__ == "__main__":
    # 配置参数（可根据机器内存调整batch_size：内存小则调小，如5_000_000）
    generate_large_float_dataset(
        output_file="/home/chengang/chengang/jingtao8a/reconstruct_NFL/data/outlier_200M_double.bin.data",
        file_format="binary",
        batch_size=10_000_000  # 每批次生成1000万，内存不足可改为500万
    )