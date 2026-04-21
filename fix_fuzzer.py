with open("fuzz/fuzz_rule_engine.cpp", "r") as f:
    text = f.read()

text = text.replace('file.path = "C:/dummy/test/image.jpg";', 'file.uri = "C:/dummy/test/image.jpg";')

with open("fuzz/fuzz_rule_engine.cpp", "w") as f:
    f.write(text)
