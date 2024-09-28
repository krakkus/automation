import requests


class Client:
    server = None
    key = None
    def __init__(self, server, key):
        self.server = server
        self.key = key

    def table_1_write(self, type, key_txt, key_int, value_txt="", value_int=0, extra_txt="", extra_int=0):
        url = f"{self.server}/table_1_write?api_key={self.key}&" \
              f"type={type}&key_txt={key_txt}&key_int={key_int}&" \
              f"value_txt={value_txt}&value_int={value_int}&extra_txt={extra_txt}&extra_int={extra_int}"
        r = requests.get(url)
        print(r.status_code)
