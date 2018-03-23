import csv
import getpass
import os
import sys
import pwd
import subprocess
import shutil

# MODIFY THESE AS NECESSARY
INPUT_FILE = None
CSV_DELIMITER = ","
CSV_HEADER = True  # we skip the first line of the csv file if this is set to True
USERNAME_COLUMN = 1
PASSWORD_COLUMN = 1

ADMIN_USERS = ['mmoeser', 'hkalodner', 'randomwalker']

def command_create_user(username):
    return "sudo useradd -m {}".format(username)


def command_set_user_password(username, password):
    return "echo {}:{} | sudo chpasswd".format(username, password)


def get_usernames_passwords():
    user_pass = []

    with open(INPUT_FILE, 'r') as f:
        reader = csv.reader(f, delimiter=CSV_DELIMITER)

        if CSV_HEADER:
             # skip header
            next(reader, None)

        for row in reader:
            username = row[USERNAME_COLUMN][:16].lower()
            password = row[PASSWORD_COLUMN][:16].lower()

            # sanitize username and password
            username = "".join(c for c in username if c.isalnum())
            password = "".join(c for c in password if c.isalnum())
            user_pass.append((username, password))

    return user_pass


def get_all_users():
    return [username for username, _ in get_usernames_passwords()] + ADMIN_USERS


def username_exists(username):
    try:
        pwd.getpwnam(username)
    except KeyError:
        return False
    return True


def file_exists(username, file):
    return os.path.isfile("/home/{}/{}".format(username, file))


def create_admin_users():
    for username in ADMIN_USERS:
        if username_exists(username):
            continue

        print("Please enter the password for admin account {}".format(username))
        password = getpass.getpass()

        subprocess.call(command_create_user(username), shell=True)
        subprocess.call(command_set_user_password(username, password), shell=True)
        print("Created admin account for user {}".format(username))


def create_normal_users():
    for username, password in get_usernames_passwords():
        if username_exists(username):
            continue
        subprocess.call(command_create_user(username), shell=True)
        subprocess.call(command_set_user_password(username, password), shell=True)
        print("Created account for user {} with password {}".format(username, password))


def copy_files(files, path):
    for username in get_all_users():
        for file in files:
            if file_exists(username, file):
                continue

            print("Copying file {} to user {}".format(file, username))

            shutil.copy(path + file, "/home/{}/".format(username))
            shutil.chown("/home/{}/{}".format(username, file), user=username, group=username)


def initial_setup():
    if not INPUT_FILE:
        sys.exit("Issue with filename")

    create_admin_users()
    create_normal_users()

    files = ['BlockSci Overview.ipynb', 'MIT Workshop Assignments.ipynb']  # add your files (with full path to the file) here
    path_to_files = "/home/ubuntu/files/"
    copy_files(files, path_to_files)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit('Usage: %s filename' % sys.argv[0])
    INPUT_FILE = sys.argv[1]
    
    initial_setup()

