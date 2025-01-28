#!/usr/bin/env python3
import os
import re

results = {}
pattern = re.compile(r'//file(\d+)')

for filename in os.listdir("ft_fun"):
    filepath = os.path.join("ft_fun", filename)
    with open(filepath, 'r') as file:
        content = file.read()
    match = pattern.search(content)
    if match:
        results[int(match.group(1))] = content

with open("main.c", "w") as outfile:
    for _, content in sorted(results.items()):
        outfile.write(content + '\n')