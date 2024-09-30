import requests
import urllib


class Client:
    server = None
    key = None
    def __init__(self, server, key):
        self.server = server
        self.key = key

    def query(self, query):
        params = {"apikey": self.key,
                  "query": query}

        url = f"{self.server}/query?{params}"

        response = requests.get(url, params=params)

        if response.status_code == 200:
            data = response.json()  # Assuming the response is JSON
            return data
        else:
            # Request failed
            print("Request failed with status code:", response.status_code)

        return