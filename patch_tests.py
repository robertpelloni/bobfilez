import sys
with open("tests/test_shadow_sorter_copy.cpp", "r") as f:
    content = f.read()

# Make sure ShadowSorter is fully cleaned up before moving on to the next test suite
if 'watcher->stop();' in content:
    content = content.replace('std::this_thread::sleep_for(std::chrono::milliseconds(250));', 'std::this_thread::sleep_for(std::chrono::milliseconds(250));\n        sorter.reset();\n        watcher.reset();\n        db.reset();\n        std::this_thread::sleep_for(std::chrono::milliseconds(100));')

with open("tests/test_shadow_sorter_copy.cpp", "w") as f:
    f.write(content)
