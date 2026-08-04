#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#!/usr/bin/env python3
import subprocess
import os
import logging
import sys
import argparse
import shutil
import shlex
import yaml
import datetime

# Configure logging system
logging.basicConfig(
    level=logging.ERROR,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("ui_resource_preprocessor.log"),
        logging.StreamHandler()
    ]
)

def get_module_path(module_name):
    """Get absolute path of specified module using west"""
    logging.info(f"Searching for module: {module_name}")
    try:
        result = subprocess.run(
            ["west", "list", "-f", "{name} {path}"],
            capture_output=True,
            text=True,
            check=True
        )
        
        west_topdir = subprocess.run(
            ["west", "topdir"],
            capture_output=True,
            text=True,
            check=True
        ).stdout.strip()
        
        for line in result.stdout.splitlines():
            parts = line.strip().split(maxsplit=1)
            if len(parts) >= 2 and parts[0] == module_name:
                abs_path = os.path.normpath(os.path.join(west_topdir, parts[1]))
                logging.info(f"Found module '{module_name}', normalized path: {abs_path}")
                return abs_path
                
        logging.error(f"Module '{module_name}' not found in west list")
        return None

    except subprocess.CalledProcessError as e:
        logging.error("Failed to execute west command, return code: %d", e.returncode)
        logging.error("Error output:\n%s", e.stderr)
        if "not a valid west project" in e.stderr:
            logging.error("Hint: Please ensure execution from west workspace root directory")
        return None

def run_command(cmd, cwd=None):
    """Execute command and handle output"""
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            check=True,
            text=True,
            encoding='utf-8',
            errors='replace',
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT
        )
        logging.info(f"Command executed successfully: {' '.join(cmd)}\nOutput:\n{result.stdout}")
        return True
    except subprocess.CalledProcessError as e:
        logging.error(f"Command failed: {' '.join(cmd)}\nError output:\n{e.output}")
        return False
    except UnicodeDecodeError:
        try:
            result = subprocess.run(
                cmd,
                cwd=cwd,
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT
            )
            logging.info(f"Command executed successfully in binary mode: {' '.join(cmd)}")
            return True
        except subprocess.CalledProcessError as e:
            logging.error(f"Command failed in binary mode: {' '.join(cmd)}")
            return False

def find_ui_resource_folder(honeygui_path, ui_demo):
        
    ui_resource_folder = os.path.join(honeygui_path, "example", "application", f"screen_{ui_demo}")
    
    if os.path.exists(ui_resource_folder) and os.path.isdir(ui_resource_folder):
        logging.info(f"Found UI resource folder: {ui_resource_folder}")
        return ui_resource_folder
    else:
        logging.error(f"UI resource folder not found: {ui_resource_folder}")
        return None

def delete_ui_resources(ui_resource_folder, ui_demo):
    """Delete UI resources based on yaml configuration file"""
    config_file = os.path.abspath(os.path.join(os.path.dirname(__file__), "./ui_resource_to_delete.yaml"))
    
    if not os.path.exists(config_file):
        logging.warning(f"UI resource deletion configuration file not found: {config_file}")
        return True
        
    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            delete_config = yaml.safe_load(f)
            
        if not isinstance(delete_config, dict) or "ui_resources" not in delete_config:
            logging.warning("UI resource deletion configuration should be a dict with 'ui_resources' key")
            return True
            
        # Find the matching UI demo configuration
        target_config = None
        for config in delete_config["ui_resources"]:
            if config.get("example") == f"screen_{ui_demo}":
                target_config = config.get("delete", [])
                break
                
        if not target_config:
            logging.info(f"No UI resource deletion configuration found for screen_{ui_demo}")
            return True
            
        for file_path in target_config:
            full_path = os.path.normpath(os.path.join(ui_resource_folder, file_path))
            if os.path.exists(full_path):
                try:
                    if os.path.isfile(full_path):
                        os.remove(full_path)
                        logging.info(f"Deleted UI resource file: {full_path}")
                    elif os.path.isdir(full_path):
                        shutil.rmtree(full_path)
                        logging.info(f"Deleted UI resource directory: {full_path}")
                except Exception as e:
                    logging.error(f"Failed to delete {full_path}: {e}")
            else:
                logging.warning(f"UI resource not found for deletion: {full_path}")
                
        return True
        
    except Exception as e:
        logging.error(f"Failed to read or process UI resource deletion configuration: {e}")
        return False

def run_mkromfs_script(honeygui_path, ui_resource_folder, ui_base_addr, output_dir):

    mkromfs_script = os.path.normpath(os.path.join(honeygui_path, "tool", "mkromfs", "mkromfs_for_honeygui.py"))
    
    if not os.path.exists(mkromfs_script):
        logging.error(f"mkromfs_for_honeygui.py not found at: {mkromfs_script}")
        return None
        
    root = os.path.join(ui_resource_folder, "root_image", "root")
    
    if not ui_base_addr.startswith("0x"):
        logging.error(f"ui_base_addr must be hex format, start with 0x. your addr: {ui_base_addr}")
        return None

    base_address = ui_base_addr
    # Output bin file name
    output_bin = os.path.join(output_dir, f"root_{ui_base_addr}.bin")
    
    # Run mkromfs_for_honeygui.py
    mkromfs_cmd = [
        "python", mkromfs_script,
        "-i", root,
        "-o", output_bin,
        "-b",
        "-a", base_address
    ]
    
    logging.info(f"Executing mkromfs command: {mkromfs_cmd}")
    if run_command(mkromfs_cmd, cwd=output_dir):
        logging.info(f"Successfully generated UI resource bin file: {output_bin}")
        return output_bin
    else:
        logging.error("Failed to run mkromfs_for_honeygui.py")
        return None

def run_prepend_header(output_dir, input_bin):
    """Run prepend_header.exe to add header to generated bin file"""
    # Find realtek-app module path
    realtek_app_path = get_module_path("realtek-app")
    if not realtek_app_path:
        logging.error("Failed to get realtek-app module path")
        return
    try:
        prepend_header_tool_path = os.path.join(realtek_app_path, "tools", "Gadgets", "prepend_header.exe")    
        if not os.path.exists(prepend_header_tool_path):
            logging.error(f"prepend_header.exe not found at: {prepend_header_tool_path}")
            return None

        md5_tool_path = os.path.join(realtek_app_path, "tools", "Gadgets", "md5.exe")    
        if not os.path.exists(md5_tool_path):
            logging.error(f"md5.exe not found at: {md5_tool_path}")
            return None

        # Add image header
        add_image_header_cmd = [
            prepend_header_tool_path,
            "/user_data1", input_bin,
            "/ic_type", "8773G"
        ]

        # get mp_userdata1.ini path
        mp_userdata1_ini = os.path.join(realtek_app_path, "script", "prebuild", "mp_userdata1.ini")
        if not os.path.exists(mp_userdata1_ini):
            logging.error(f"mp_userdata1.ini not found at: {mp_userdata1_ini}")
            return None

        # Add mp header
        add_prepend_cmd = [
            prepend_header_tool_path,
            "/user_data1", input_bin,
            "/mp_ini", mp_userdata1_ini,
            "/ic_type", "8773G"
        ]        

        input_base_name = os.path.splitext(os.path.basename(input_bin))[0]
        final_output_bin = os.path.join(output_dir, f"{input_base_name}_MP.bin")
        # Add md5
        add_md5_cmd = [
            md5_tool_path, 
            final_output_bin
        ]  

        if run_command(add_image_header_cmd, cwd=output_dir):
            logging.info(f"Successfully add image header to: {output_dir}")
        else:
            logging.error("Failed to run mkromfs_for_honeygui.py")
            return None

        if run_command(add_prepend_cmd, cwd=output_dir):
            logging.info(f"Successfully add mp header to: {output_dir}")
        else:
            logging.error("Failed to run mkromfs_for_honeygui.py")
            return None

        if run_command(add_md5_cmd, cwd=output_dir):
            logging.info(f"Successfully add md5 to: {final_output_bin}")
        else:
            logging.error("Failed to run mkromfs_for_honeygui.py")
            return None

        os.remove(os.path.join(output_dir, f"{input_base_name}.bin"))
        os.remove(os.path.join(output_dir, f"{input_base_name}_MP.bin"))
        
        return final_output_bin

    except Exception as e:
        logging.error(f"Failed to run prepend_header.exe: {e}")
        return None

def ensure_output_dir(output_dir):
    """Ensure output directory exists"""
    try:
        os.makedirs(output_dir, exist_ok=True)
        logging.info(f"Output directory ready: {output_dir}")
        return True
    except Exception as e:
        logging.error(f"Failed to create output directory: {e}")
        return False

def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description="UI Resource Preprocessor Tool",
        epilog="Example usage:\n" +
               "  python ui_resource_preprocessor.py --ui_demo 410_502 --ui_base_addr 0x704D1400 --output ./ui_output\n"
    )
    parser.add_argument(
        "--ui_demo",
        required=True,
        help="Specify UI demo name (e.g., 410_502)"
    )
    parser.add_argument(
        "--ui_base_addr",
        required=True,
        help="Specify base address for UI resource"
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Specify output folder for processed UI resources"
    )
    
    return parser.parse_args()

def main():
    # Parse command line arguments
    args = parse_arguments()
    
    # Ensure output directory exists
    if not ensure_output_dir(args.output):
        sys.exit(1)
    
    # Find UI resource folder based on WEST and UI demo name
    honeygui_path = get_module_path("honeygui")
    if not honeygui_path:
        logging.error("Failed to get honeygui module path")
        sys.exit(2)

    # Find UI resource folder
    ui_resource_folder = find_ui_resource_folder(honeygui_path, args.ui_demo)
    if not ui_resource_folder:
        sys.exit(3)
        
    # Delete specified UI resources based on yaml configuration
    if not delete_ui_resources(ui_resource_folder, args.ui_demo):
        sys.exit(4)
        
    # Run mkromfs_for_honeygui.py to merge UI resources
    merged_bin = run_mkromfs_script(honeygui_path, ui_resource_folder, args.ui_base_addr, args.output)
    if not merged_bin:
        sys.exit(5)

    # Run prepend_header.exe to add header
    header_bin = run_prepend_header(args.output, merged_bin)
    if not header_bin:
        sys.exit(6)
        
    logging.info("\n=== UI Resource Preprocessing Complete ===")
    logging.info(f"Output Directory: {args.output}")
    logging.info(f"Merged Bin File: {merged_bin}")
    logging.info(f"Header-prepended Bin File: {header_bin}")

if __name__ == "__main__":
    main()
