import subprocess
import os
import re
from datetime import datetime

def get_submodules():
    submodules = []
    try:
        with open('.gitmodules', 'r') as f:
            content = f.read()
            matches = re.finditer(r'\[submodule "(.*?)"\]\s*path = (.*?)\s*url = (.*?)\s', content, re.MULTILINE | re.DOTALL)
            for match in matches:
                name = match.group(1)
                path = match.group(2).strip()
                url = match.group(3).strip()
                submodules.append({'name': name, 'path': path, 'url': url})
    except FileNotFoundError:
        print("Error: .gitmodules not found")
    return submodules

def get_git_info(path):
    try:
        # Get commit hash
        commit_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], cwd=path).decode('utf-8').strip()
        
        # Get commit date
        commit_date = subprocess.check_output(['git', 'log', '-1', '--format=%cd', '--date=short'], cwd=path).decode('utf-8').strip()
        
        # Get commit count (build number approximation)
        commit_count = subprocess.check_output(['git', 'rev-list', '--count', 'HEAD'], cwd=path).decode('utf-8').strip()
        
        # Get current tag if any
        try:
            tag = subprocess.check_output(['git', 'describe', '--tags', '--abbrev=0'], cwd=path, stderr=subprocess.DEVNULL).decode('utf-8').strip()
        except:
            tag = "-"
            
        return commit_hash, commit_date, commit_count, tag
    except Exception as e:
        return "Error", "Error", "Error", "Error"

def generate_markdown(submodules):
    md = "# Submodule Dashboard\n\n"
    md += f"**Last Updated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
    
    md += "## Project Structure\n\n"
    md += "The project is organized as follows:\n\n"
    md += "- **core/**: Contains the core logic of the filez application (scanning, hashing, database, etc.).\n"
    md += "- **cli/**: Contains the Command Line Interface (CLI) application.\n"
    md += "- **libs/**: Contains all external dependencies included as git submodules.\n"
    md += "- **docs/**: Project documentation.\n"
    md += "- **benchmarks/**: Performance benchmarks.\n"
    md += "- **tests/**: Unit and integration tests.\n"
    md += "- **vcpkg/**: The vcpkg package manager submodule.\n\n"
    
    md += "## Submodules Status\n\n"
    md += "| Name | Path | Version (Tag) | Commit | Date | Build # | URL |\n"
    md += "|---|---|---|---|---|---|---|\n"
    
    for sub in submodules:
        path = sub['path']
        if os.path.exists(path):
            commit, date, count, tag = get_git_info(path)
            md += f"| {sub['name']} | `{path}` | {tag} | `{commit}` | {date} | {count} | [Link]({sub['url']}) |\n"
        else:
            md += f"| {sub['name']} | `{path}` | MISSING | - | - | - | [Link]({sub['url']}) |\n"
            
    return md

if __name__ == "__main__":
    subs = get_submodules()
    md_content = generate_markdown(subs)
    
    with open('docs/SUBMODULES.md', 'w') as f:
        f.write(md_content)
    
    print("Dashboard generated at docs/SUBMODULES.md")
