with open('core/src/database.cpp', 'r') as f:
    content = f.read()

if '#include <cmath>' not in content:
    content = content.replace('#include <vector>', '#include <vector>\n#include <cmath>\n#include <algorithm>')

with open('core/src/database.cpp', 'w') as f:
    f.write(content)
