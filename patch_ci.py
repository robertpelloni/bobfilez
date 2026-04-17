import sys

with open(".github/workflows/ci.yml", "r") as f:
    content = f.read()

# Since GitHub actions checkout fails on the recursive submodule update,
# let's just make it gracefully handle errors by not failing the entire build if a weird nested submodule fails to fetch,
# OR we just change the recursive to false for now while we get the native integration set up
content = content.replace('''      - uses: actions/checkout@v4
        with:
          submodules: recursive''', '''      - uses: actions/checkout@v4
        with:
          submodules: true''')

with open(".github/workflows/ci.yml", "w") as f:
    f.write(content)
