#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

import os
import struct
import traceback
import tkinter as tk
from tkinter import filedialog, messagebox
from tkinter import ttk

# ==============================
# pack 文件格式定义（和 MCU 一致）
# ==============================

PACK_MAGIC = 0x5041434B      # 'PACK'
PACK_VERSION = 1
PACK_NAME_MAX_LEN = 64       # 含结尾 '\0'

PACK_HEADER_STRUCT = struct.Struct("<IHHII4I")
PACK_ENTRY_STRUCT = struct.Struct(f"<{PACK_NAME_MAX_LEN}sIIHHII")


class ResItem:
    def __init__(self, logical_name, file_path, rtype=0, flags=0):
        self.logical_name = logical_name  # 例如：/SD/activity/a.bin
        self.file_path = file_path
        self.type = rtype
        self.flags = flags
        self.size = 0
        self.offset = 0


# ==============================
# 核心：收集文件 + 打包
# ==============================

def collect_bin_files(root_dir, base_prefix=""):
    """
    从 root_dir 递归收集所有 .bin 文件。
    最终 logical_name 形如： /根目录名/相对路径
    """
    items = []
    root_dir = os.path.abspath(root_dir)

    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if not filename.lower().endswith(".bin"):
                continue

            full_path = os.path.join(dirpath, filename)
            rel_path = os.path.relpath(full_path, root_dir)
            logical_name = rel_path.replace(os.sep, "/")

            # 最终路径前面加上 "/根目录名" f"/{base_prefix}/{logical_name}"
            # 最终路径前面不加 "/根目录名" f"{logical_name}"
            if base_prefix:
                logical_name = f"{logical_name}"

            item = ResItem(logical_name=logical_name, file_path=full_path)
            item.size = os.path.getsize(full_path)
            items.append(item)

    items.sort(key=lambda it: it.logical_name)
    return items


def pack_to_file(root_dir, out_file_path, progress_callback=None, log_callback=None):
    """
    progress_callback(step, total_step) 用于更新进度条
    log_callback(text) 用于输出日志
    """
    def log(msg):
        if log_callback:
            log_callback(msg)

    root_dir = os.path.abspath(root_dir)
    last_dir_name = os.path.basename(root_dir.rstrip("\\/"))
    base_prefix = last_dir_name

    log(f"根目录：{root_dir}")
    log(f"根目录名前缀：/{base_prefix}/")

    if progress_callback:
        progress_callback(0, 100)

    items = collect_bin_files(root_dir, base_prefix=base_prefix)
    if not items:
        return False, "所选目录下未找到任何 .bin 文件"

    entry_count = len(items)
    header_size = PACK_HEADER_STRUCT.size
    entry_size = PACK_ENTRY_STRUCT.size
    index_offset = header_size
    data_offset = header_size + entry_count * entry_size

    cur_offset = data_offset
    for item in items:
        item.offset = cur_offset
        cur_offset += item.size

    total_size = cur_offset
    log(f"共找到 {entry_count} 个资源，预计 pack 文件大小约 {total_size} 字节")

    with open(out_file_path, "wb") as f:
        # 1. 头占位
        f.write(b"\x00" * header_size)

        # 2. 写 entry 表
        for idx, item in enumerate(items):
            name_bytes = item.logical_name.encode("utf-8")
            if len(name_bytes) >= PACK_NAME_MAX_LEN:
                raise ValueError(
                    f"资源名过长（>{PACK_NAME_MAX_LEN - 1} 字节）：{item.logical_name}"
                )

            name_field = (
                name_bytes
                + b"\x00"
                + b"\x00" * (PACK_NAME_MAX_LEN - 1 - len(name_bytes))
            )

            entry_data = PACK_ENTRY_STRUCT.pack(
                name_field,
                item.offset,
                item.size,
                item.type,
                item.flags,
                0,
                0
            )
            f.write(entry_data)

            if progress_callback:
                # entry 写入阶段占 30% 进度
                progress_callback(int(10 + 30 * (idx + 1) / entry_count), 100)

        # 3. 写数据区
        written_data = 0
        total_data = sum(it.size for it in items)

        for idx, item in enumerate(items):
            with open(item.file_path, "rb") as rf:
                data = rf.read()
                if len(data) != item.size:
                    raise IOError(f"读取文件大小不一致: {item.file_path}")
                f.write(data)
                written_data += item.size

            if progress_callback and total_data > 0:
                # 数据写入阶段占 60% 进度
                progress_callback(
                    int(40 + 60 * written_data / total_data),
                    100
                )

        # 4. 回写 header
        f.seek(0, os.SEEK_SET)
        header_data = PACK_HEADER_STRUCT.pack(
            PACK_MAGIC,
            PACK_VERSION,
            entry_count,
            index_offset,
            data_offset,
            0, 0, 0, 0
        )
        f.write(header_data)

    debug_lines = [
        f"[{idx+1:03d}] {it.logical_name}  <--  {it.file_path}  ({it.size} 字节)"
        for idx, it in enumerate(items)
    ]

    msg = (
        f"打包完成：{out_file_path}\n"
        f"资源总数：{entry_count}\n"
        f"所有 entry：\n  " + "\n  ".join(debug_lines)
    )

    if progress_callback:
        progress_callback(100, 100)

    log("打包完成。")
    return True, msg


# ==============================
# GUI
# ==============================

class PackGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("资源打包工具（.bin -> pack）")
        self.root.geometry("780x520")

        # 使用 ttk 主题
        style = ttk.Style()
        # 使用系统可用的一个主题
        if "vista" in style.theme_names():
            style.theme_use("vista")
        elif "clam" in style.theme_names():
            style.theme_use("clam")

        # 整体字体
        default_font = ("Microsoft YaHei", 9)
        self.root.option_add("*Font", default_font)

        # ===== 顶部标题栏 =====
        header_frame = ttk.Frame(root, padding=(10, 8))
        header_frame.pack(fill="x")

        title_label = ttk.Label(
            header_frame,
            text="资源打包工具",
            font=("Microsoft YaHei", 14, "bold")
        )
        title_label.pack(side="left")

        subtitle_label = ttk.Label(
            header_frame,
            text="  将 .bin 文件打包为 pack（路径格式：/根目录名/相对路径）",
            foreground="#555555"
        )
        subtitle_label.pack(side="left")

        # 一条分割线
        sep = ttk.Separator(root, orient="horizontal")
        sep.pack(fill="x", padx=5, pady=(0, 5))

        # ===== 主体区域 =====
        main_frame = ttk.Frame(root, padding=(10, 5))
        main_frame.pack(fill="both", expand=True)

        # --- 输入设置分组 ---
        group_input = ttk.Labelframe(main_frame, text="打包设置", padding=(10, 10))
        group_input.pack(fill="x", pady=(0, 8))

        # 资源根目录
        lbl_dir = ttk.Label(group_input, text="资源根目录：")
        lbl_dir.grid(row=0, column=0, sticky="e", pady=4)

        self.entry_dir = ttk.Entry(group_input)
        self.entry_dir.grid(row=0, column=1, sticky="we", padx=4, pady=4)

        btn_browse = ttk.Button(group_input, text="浏览...", command=self.browse_dir, width=10)
        btn_browse.grid(row=0, column=2, padx=4, pady=4)

        # 输出文件
        lbl_out = ttk.Label(group_input, text="输出文件：")
        lbl_out.grid(row=1, column=0, sticky="e", pady=4)

        self.entry_out = ttk.Entry(group_input)
        self.entry_out.grid(row=1, column=1, sticky="we", padx=4, pady=4)
        self.entry_out.insert(0, "res.bin")

        btn_choose_out = ttk.Button(group_input, text="选择...", command=self.choose_out_file, width=10)
        btn_choose_out.grid(row=1, column=2, padx=4, pady=4)

        group_input.columnconfigure(1, weight=1)

        # --- 操作按钮 + 进度条 ---
        action_frame = ttk.Frame(main_frame)
        action_frame.pack(fill="x", pady=(0, 5))

        self.btn_pack = ttk.Button(
            action_frame,
            text="开始打包",
            command=self.do_pack,
            width=12
        )
        self.btn_pack.pack(side="right", padx=(4, 0))

        self.progress = ttk.Progressbar(
            action_frame,
            orient="horizontal",
            mode="determinate",
            length=260
        )
        self.progress.pack(side="right", padx=(4, 4))

        # --- 日志输出分组 ---
        group_log = ttk.Labelframe(main_frame, text="日志输出", padding=(8, 6))
        group_log.pack(fill="both", expand=True)

        self.text_log = tk.Text(
            group_log,
            height=16,
            wrap="word",
            bg="#1e1e1e",
            fg="#e0e0e0",
            insertbackground="#ffffff",
            borderwidth=0,
            highlightthickness=0
        )
        self.text_log.pack(side="left", fill="both", expand=True)

        scrollbar = ttk.Scrollbar(group_log, orient="vertical", command=self.text_log.yview)
        scrollbar.pack(side="right", fill="y")
        self.text_log.configure(yscrollcommand=scrollbar.set)

        # ===== 状态栏 =====
        status_frame = ttk.Frame(root, relief="sunken")
        status_frame.pack(fill="x", side="bottom")

        self.status_var = tk.StringVar()
        self.status_var.set("就绪")
        self.status_label = ttk.Label(status_frame, textvariable=self.status_var, anchor="w", padding=(5, 2))
        self.status_label.pack(fill="x")

    # ---------- 工具函数 ----------

    def log(self, msg):
        self.text_log.insert(tk.END, msg + "\n")
        self.text_log.see(tk.END)
        self.text_log.update_idletasks()

    def set_status(self, text):
        self.status_var.set(text)
        self.status_label.update_idletasks()

    def set_progress(self, value, maximum):
        self.progress["maximum"] = maximum
        self.progress["value"] = value
        self.progress.update_idletasks()

    # ---------- 事件回调 ----------

    def browse_dir(self):
        directory = filedialog.askdirectory(title="选择资源根目录")
        if directory:
            self.entry_dir.delete(0, tk.END)
            self.entry_dir.insert(0, directory)

    def choose_out_file(self):
        initial = self.entry_out.get().strip() or "res.pack"
        file_path = filedialog.asksaveasfilename(
            title="选择输出 pack 文件",
            defaultextension=".pack",
            initialfile=initial,
            filetypes=[("Pack Files", "*.pack"), ("All Files", "*.*")]
        )
        if file_path:
            self.entry_out.delete(0, tk.END)
            self.entry_out.insert(0, file_path)

    def do_pack(self):
        root_dir = self.entry_dir.get().strip()
        out_path = self.entry_out.get().strip()

        if not root_dir:
            messagebox.showwarning("提示", "请先选择资源根目录")
            return
        if not os.path.isdir(root_dir):
            messagebox.showerror("错误", f"目录不存在：{root_dir}")
            return
        if not out_path:
            messagebox.showwarning("提示", "请填写输出文件路径")
            return

        # 禁用按钮，防止重复点击
        self.btn_pack.config(state="disabled")
        self.set_status("正在打包，请稍候...")
        self.set_progress(0, 100)
        self.log(f"开始打包目录：{root_dir}")
        self.log(f"输出文件：{out_path}")
        self.log("-" * 60)

        self.root.update_idletasks()

        try:
            ok, msg = pack_to_file(
                root_dir,
                out_path,
                progress_callback=self.set_progress,
                log_callback=self.log
            )
            self.log(msg)
            self.log("-" * 60)

            if ok:
                self.set_status("打包完成")
                messagebox.showinfo("完成", msg)
            else:
                self.set_status("打包结束（有警告）")
                messagebox.showwarning("提示", msg)
        except Exception as e:
            err_msg = f"打包失败：{e}"
            self.log(err_msg)
            self.log(traceback.format_exc())
            self.set_status("打包失败")
            messagebox.showerror("错误", err_msg)
        finally:
            self.btn_pack.config(state="normal")

def main():
    root = tk.Tk()
    app = PackGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
