#!/bin/bash

# =========================
# MEMBUAT FOLDER DATA
# =========================

mkdir -p /libraryit/ebooks
mkdir -p /libraryit/papers
mkdir -p /libraryit/sourcecode
mkdir -p /libraryit/docs

# =========================
# MEMBUAT FOLDER LOG
# =========================

mkdir -p /var/log/libraryit
mkdir -p /var/log/samba

touch /var/log/libraryit/libraryit.log
touch /var/log/samba/log.smbd

# =========================
# MEMBUAT GROUP
# =========================

groupadd readonly 2>/dev/null || true
groupadd staff 2>/dev/null || true

# =========================
# MEMBUAT USER
# =========================

id member >/dev/null 2>&1 || useradd -m member
id contributor >/dev/null 2>&1 || useradd -m contributor
id librarian >/dev/null 2>&1 || useradd -m librarian

# =========================
# PASSWORD USER LINUX
# =========================

echo "member:member123" | chpasswd
echo "contributor:contrib456" | chpasswd
echo "librarian:lib789" | chpasswd

# =========================
# PASSWORD SAMBA
# =========================

(echo member123; echo member123) | smbpasswd -a -s member
(echo contrib456; echo contrib456) | smbpasswd -a -s contributor
(echo lib789; echo lib789) | smbpasswd -a -s librarian

# =========================
# MENAMBAHKAN USER KE GROUP
# =========================

usermod -aG readonly member
usermod -aG staff contributor
usermod -aG staff librarian

# =========================
# PERMISSION FOLDER
# =========================

chown -R root:staff /libraryit/ebooks
chown -R root:staff /libraryit/papers
chown -R root:readonly /libraryit/sourcecode
chown -R root:staff /libraryit/docs

chmod -R 770 /libraryit/ebooks
chmod -R 770 /libraryit/papers
chmod -R 750 /libraryit/sourcecode
chmod -R 770 /libraryit/docs

# =========================
# ACL
# =========================

setfacl -m u:librarian:rwx /libraryit/docs
setfacl -m u:contributor:rx /libraryit/docs
setfacl -m u:member:rx /libraryit/docs

setfacl -m g:readonly:rx /libraryit/sourcecode
setfacl -m g:staff:rx /libraryit/sourcecode

# =========================
# MENJALANKAN SAMBA
# =========================

service smbd start
service nmbd start

# =========================
# CUSTOM LOGGER
# =========================

(
tail -Fn0 /var/log/samba/log.smbd | while read line
do
    TIME=$(date '+[%Y-%m-%d %H:%M:%S]')

    # =====================
    # CONNECT
    # =====================

    if echo "$line" | grep -qi "connect to service"
    then
        SHARE=$(echo "$line" | awk -F"service " '{print $2}' | awk '{print $1}')

        echo "$TIME [INFO] [unknown] [CONNECT] [$SHARE]" >> /var/log/libraryit/libraryit.log
    fi

    # =====================
    # WRITE
    # =====================

    if echo "$line" | grep -qi "opened file"
    then
        FILE=$(echo "$line" | awk -F"opened file " '{print $2}')

        echo "$TIME [INFO] [unknown] [WRITE] [$FILE]" >> /var/log/libraryit/libraryit.log
    fi

    # =====================
    # DENIED
    # =====================

    if echo "$line" | grep -qi "Access denied"
    then
        FILE=$(echo "$line" | grep -oP 'file \K[^:]+' )

        echo "$TIME [WARNING] [unknown] [DENIED] [$FILE]" >> /var/log/libraryit/libraryit.log
    fi

done
) &

# =========================
# CONTAINER TETAP HIDUP
# =========================

tail -f /dev/null
