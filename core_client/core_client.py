import requests


class Client:
    server = None
    key = None
    def __init__(self, server, key):
        self.server = server
        self.key = key

    def endpoint_heartbeat(self, id, ip):
        r = requests.get(f'{self.server}/endpoint_heartbeat?key={self.key}&id={id}&ip={ip}')
        print(r.status_code)
