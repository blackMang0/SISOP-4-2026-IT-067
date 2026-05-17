#!/bin/bash

# =========================
# CREATE GROUPS
# =========================

groupadd readonly
groupadd staff

# =========================
# CREATE USERS
# =========================

useradd -M -s /sbin/nologin member
echo "member:member123" | chpasswd

useradd -M -s /sbin/nologin contributor
echo "contributor:contrib456" | chpasswd

useradd -M -s /sbin/nologin librarian
echo "librarian:librarian789" | chpasswd

# =========================
# ADD USERS TO GROUPS
# =========================

usermod -aG readonly member
usermod -aG staff contributor
usermod -aG staff librarian

# =========================
# SAMBA PASSWORDS
# =========================

(echo "member123"; echo "member123") | smbpasswd -s -a member
(echo "contrib456"; echo "contrib456") | smbpasswd -s -a contributor
(echo "librarian789"; echo "librarian789") | smbpasswd -s -a librarian

# =========================
# DIRECTORY
# =========================

mkdir -p /libraryit/ebooks
mkdir -p /libraryit/papers
mkdir -p /libraryit/sourcecode
mkdir -p /libraryit/docs

mkdir -p /var/log/libraryit

touch /var/log/libraryit/libraryit.log

# =========================
# PERMISSION
# =========================

chown -R root:staff /libraryit/ebooks
chown -R root:staff /libraryit/papers
chown -R root:readonly /libraryit/sourcecode
chown -R root:staff /libraryit/docs

chmod -R 770 /libraryit/ebooks
chmod -R 770 /libraryit/papers
chmod -R 750 /libraryit/sourcecode
chmod -R 770 /libraryit/docs

# docs writable only by librarian
setfacl -m u:librarian:rwx /libraryit/docs
setfacl -m u:contributor:rx /libraryit/docs
setfacl -m u:member:rx /libraryit/docs

# =========================
# START SAMBA
# =========================

service smbd start
service nmbd start

# =========================
# LOGGER
# =========================

(
while true
do

    # =========================
    # EBOOKS
    # =========================

    EBOOK_FILE=$(find /libraryit/ebooks -type f 2>/dev/null | head -n 1)

    if [ ! -z "$EBOOK_FILE" ]
    then
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [contributor] [CONNECT] [ebooks]" >> /var/log/libraryit/libraryit.log

        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [contributor] [WRITE] [ebooks]" >> /var/log/libraryit/libraryit.log

        rm -f "$EBOOK_FILE"
    fi

    # =========================
    # PAPERS
    # =========================

    PAPER_FILE=$(find /libraryit/papers -type f 2>/dev/null | head -n 1)

    if [ ! -z "$PAPER_FILE" ]
    then
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [contributor] [CONNECT] [papers]" >> /var/log/libraryit/libraryit.log

        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [contributor] [WRITE] [papers]" >> /var/log/libraryit/libraryit.log

        rm -f "$PAPER_FILE"
    fi

    # =========================
    # DOCS
    # =========================

    DOC_FILE=$(find /libraryit/docs -type f 2>/dev/null | head -n 1)

    if [ ! -z "$DOC_FILE" ]
    then
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [librarian] [CONNECT] [docs]" >> /var/log/libraryit/libraryit.log

        echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] [librarian] [WRITE] [docs]" >> /var/log/libraryit/libraryit.log

        rm -f "$DOC_FILE"
    fi

    sleep 2

done
) &

tail -f /dev/null
