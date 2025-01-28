import itertools
import string

def generate_phase5_solutions():
    # Define the array as per the C code
    array = "isrveawhobpnutfg"

    # Target string after mapping
    target = "giants"

    # Define the mapping manually based on manual analysis
    # Position 0: 'g' -> array[15] -> input chars with (c & 0x0F) == 15 -> 'o'
    # Position 1: 'i' -> array[0]  -> input chars with (c & 0x0F) == 0  -> 'p'
    # Position 2: 'a' -> array[5]  -> input chars with (c & 0x0F) == 5  -> 'e', 'u'
    # Position 3: 'n' -> array[11] -> input chars with (c & 0x0F) == 11 -> 'k'
    # Position 4: 't' -> array[13] -> input chars with (c & 0x0F) == 13 -> 'm'
    # Position 5: 's' -> array[1]  -> input chars with (c & 0x0F) == 1  -> 'a', 'q'

    # Mapping of each position to possible input characters
    position_mapping = {
        0: ['o'],          # 'g' -> 'o'
        1: ['p'],          # 'i' -> 'p'
        2: ['e', 'u'],     # 'a' -> 'e', 'u'
        3: ['k'],          # 'n' -> 'k'
        4: ['m'],          # 't' -> 'm'
        5: ['a', 'q'],     # 's' -> 'a', 'q'
    }

    # Extract possible characters per position in order
    mapping = [
        position_mapping.get(i, []) for i in range(6)
    ]

    # Check if all positions have at least one possible character
    for i, chars in enumerate(mapping):
        if not chars:
            print(f"No possible characters found for position {i}. No solution exists.")
            return []

    # Generate all possible combinations using Cartesian product
    all_combinations = list(itertools.product(*mapping))

    # Convert each combination tuple to a string
    solutions = [''.join(combo) for combo in all_combinations]

    return solutions

if __name__ == "__main__":
    solutions = generate_phase5_solutions()
    print("Possible solutions for phase 5:")
    for sol in solutions:
        print(sol)
    print(f"\nTotal solutions found: {len(solutions)}")