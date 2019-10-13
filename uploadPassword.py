Import("env")

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

config = configparser.ConfigParser()
config.read("uploadPassword.ini")

password = config.get("password", "upload_password")

env.Replace(
    UPLOAD_FLAGS="--auth=" + password
)
