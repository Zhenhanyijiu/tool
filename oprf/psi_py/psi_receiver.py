import socket
import sys
from oprf_psi import OprfPsiReceiver

if __name__ == '__main__':
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    host = socket.gethostname()
    print("===>>host:", host)
    port = 9999
    pass
