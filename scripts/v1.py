import yaml
import re
import os
import pdfplumber
from collections import OrderedDict

# --- 配置 ---
IDIS_PDF_PATH = "/workspaces/projectX/scripts/IDIS-S03-001 - object model Pack3 Ed1.0 - V3.pdf"
OUTPUT_YAML_PATH = "idis_objects_from_text.yaml"
IDIS_START_PAGE = 7
IDIS_END_PAGE = 10

# --- YAML美化 ---
yaml.add_representer(OrderedDict, 
    lambda dumper, data: dumper.represent_mapping('tag:yaml.org,2002:map', data.items()))

# --- 辅助函数 ---
def clean_text(text):
    if text is None: return ""
    return ' '.join(str(text).split())

# --- 核心解析逻辑 (V29) ---
def main():
    if not os.path.exists(IDIS_PDF_PATH):
        print(f"Error: PDF file not found: '{IDIS_PDF_PATH}'")
        return

    all_objects = []
    current_object = None
    current_group = "Uncategorized"
    parsing_state = "ATTRIBUTES" # 初始状态为解析属性

    # 正则表达式
    obj_header_pattern = re.compile(
        r"^(?P<name>[\w\s\(\)\+,'/–-]+?)\s+"
        r"(?:[MDO\s-]+?)\s+"
        r"(?P<class_id>\d+)\s+"
        r"(?P<version>\d+)\s+"
        r"(?P<obis>[\w:.-]+)"
    )
    group_pattern = re.compile(r"^[A-Z][\w\s'-]+objects?|Local communication|Electricity related objects")

    with pdfplumber.open(IDIS_PDF_PATH) as pdf:
        print(f"--- Starting Final Refined Text Stream Parsing ---")
        
        full_text = ""
        for i in range(IDIS_START_PAGE, min(IDIS_END_PAGE + 1, len(pdf.pages))):
            page = pdf.pages[i]
            page_text = page.extract_text(x_tolerance=2, layout=True)
            if page_text:
                full_text += page_text + "\n"

    lines = full_text.split('\n')
    for line in lines:
        clean_line = line.strip()
        if not clean_line or "Page" in line or "Copyright" in line or "©" in line:
            continue

        # 1. 检查是否是对象标题
        header_match = obj_header_pattern.match(clean_line)
        if header_match and len(clean_line.split()) > 4:
            if current_object:
                all_objects.append(current_object)
            
            current_object = OrderedDict([
                ('group', current_group), ('name', header_match.group('name').strip()),
                ('class_id', int(header_match.group('class_id'))), ('version', int(header_match.group('version'))),
                ('obis_code', header_match.group('obis')),
                ('attributes', []), ('methods', []) # 同时创建 attributes 和 methods 列表
            ])
            parsing_state = "ATTRIBUTES" # 每个新对象都从解析属性开始
            print(f"  -> Found Object: {current_object['name']}")
            continue

        # 2. 检查是否是分组标题
        group_match = group_pattern.match(line)
        if group_match and len(line.split()) < 7:
            if current_object:
                all_objects.append(current_object)
                current_object = None
            current_group = group_match.group(0).strip()
            print(f"--- Switched to Group: {current_group} ---")
            continue

        # 3. 如果我们正在一个对象内部，进行解析
        if current_object:
            # 检查是否是 "Specific methods" 子表头
            if "Specific methods" in line:
                parsing_state = "METHODS" # 切换状态
                continue # 跳过这一行本身

            # 使用强大的正则表达式按多空格分割列
            columns = re.split(r'\s{2,}', clean_line)
            if not columns or len(columns[0]) == 0: continue
            
            item_name_part = columns[0]
            
            # 提取索引和名称
            index_match = re.match(r"(\d+)\s+(.*)", item_name_part)
            if index_match:
                index = int(index_match.group(1))
                name = index_match.group(2).strip()
                
                # 从剩余的列中提取类型和默认值
                # 这是一个启发式，因为列对齐不完美
                attr_type = columns[1] if len(columns) > 1 else ""
                default_val = ' '.join(columns[2:]) if len(columns) > 2 else ""

                item = OrderedDict([
                    ('index', index), ('name', name),
                    ('attribute_type', attr_type.strip()),
                    ('default_value', default_val.strip()),
                ])
                
                # 根据当前状态，将解析出的项放入正确的列表
                if parsing_state == "ATTRIBUTES":
                    current_object['attributes'].append(item)
                elif parsing_state == "METHODS":
                    current_object['methods'].append(item)

    if current_object:
        all_objects.append(current_object)

    print(f"\n--- Extraction Complete ---")
    print(f"  -> Successfully parsed {len(all_objects)} IDIS object instances.")

    if all_objects:
        with open(OUTPUT_YAML_PATH, "w", encoding="utf-8") as f:
            yaml.dump(all_objects, f, sort_keys=False, allow_unicode=True, width=150)
        print(f"Final object list has been written to '{OUTPUT_YAML_PATH}'")

if __name__ == "__main__":
    main()