import os
import subprocess
import random
import bisect
from tqdm import tqdm

def get_lis_length(A):
    """Computes the length of the LIS using patience sort (O(n log n))."""
    if not A:
        return 0
    tails = []
    for x in A:
        idx = bisect.bisect_left(tails, x)
        if idx < len(tails):
            tails[idx] = x
        else:
            tails.append(x)
    return len(tails)

def is_increasing(a):
    """Checks if the list is strictly increasing."""
    return all(a[i] < a[i+1] for i in range(len(a)-1))

def is_subsequence(sub, A):
    it = iter(A)
    return all(x in it for x in sub)

def generate_tests(num_tests=100, max_n=100000):
    if not os.path.exists('tests'):
        os.makedirs('tests')

    print(f"Generating {num_tests} test cases...")
    for i in tqdm(range(1, num_tests + 1), desc="Generating"):
        n = random.randint(2, max_n)
        s = random.randint(int(n**0.5) + 1, n)
        A = [random.randint(0, 100) for _ in range(n)]

        with open(f'tests/{i}.in', 'w') as f:
            f.write(f"{n} {s}\n")
            f.write(" ".join(map(str, A)) + "\n")

        expected_len = get_lis_length(A)
        with open(f'tests/{i}.out', 'w') as f:
            f.write(str(expected_len) + "\n")

def run_single_test(test_id, input_path, output_path, executable="./lis_prog.exe"):
    """Runs a single test case and returns (passed, detail_message)."""
    try:
        with open(output_path, 'r') as f_out:
            expected_len = int(f_out.read().strip())
        
        with open(input_path, 'r') as f_in:
            lines = f_in.readlines()
            original_A = list(map(int, lines[1].split()))
            
        with open(input_path, 'r') as f_in:
            result = subprocess.run([executable], stdin=f_in, capture_output=True, text=True, timeout=30)
            actual_output = result.stdout.strip()
            
        actual_list = list(map(int, actual_output.split())) if actual_output else []
        
        if len(actual_list) != expected_len:
            return False, f"Test {test_id}: FAILED (Wrong length: expected {expected_len}, got {len(actual_list)})"
        
        if not is_increasing(actual_list):
            return False, f"Test {test_id}: FAILED (Not strictly increasing)"
            
        if not is_subsequence(actual_list, original_A):
            return False, f"Test {test_id}: FAILED (Not a subsequence)"
            
        return True, f"Test {test_id}: PASSED"
        
    except subprocess.TimeoutExpired:
        return False, f"Test {test_id}: TIMEOUT"
    except Exception as e:
        return False, f"Test {test_id}: ERROR ({e})"

def run_tests():
    print("\nCompiling")
    compile_cmd = ["g++", "-static", "-std=c++20", "test_lis.cpp", "-o", "lis_prog.exe"]
    try:
        subprocess.run(compile_cmd, check=True)
        print("Compilation successful")
    except subprocess.CalledProcessError:
        print("Compilation failed")
        return

    test_files = [f for f in os.listdir('tests') if f.endswith('.in')]
    test_files.sort(key=lambda x: int(x.split('.')[0]))
    
    print(f"\nRunning {len(test_files)} tests...")
    passed = 0
    results = []
    
    for test_in in tqdm(test_files, desc="Testing"):
        test_id = test_in.split('.')[0]
        input_path = os.path.join('tests', test_in)
        output_path = os.path.join('tests', f"{test_id}.out")
        
        is_passed, message = run_single_test(test_id, input_path, output_path)
        if is_passed:
            passed += 1
        else:
            results.append(message)
            
    print("\n--- Test Results ---")
    if results:
        for res in results:
            print(res)
    
    print(f"\nSummary: {passed}/{len(test_files)} tests passed.")

if __name__ == "__main__":
    generate_tests()
    run_tests()
