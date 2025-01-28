def test_func4(x):
    if x <= 1:
        return 1
    return test_func4(x-1) + test_func4(x-2)

for i in range(1, 20):
    if test_func4(i) == 55:
        print(f"Found solution: {i}")
        break