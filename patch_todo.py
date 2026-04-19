with open("TODO.md", "r") as f:
    text = f.read()

text = text.replace('- [ ] Example `rules.yaml` files for various user personas (Photographer, SysAdmin).', '- [x] Example `rules.yaml` files for various user personas (Photographer, SysAdmin).')

with open("TODO.md", "w") as f:
    f.write(text)
