from oprf_psi import OprfPsiSender, generate_dataset_debug
import numpy as np
import socket
import sys
from hashlib import sha256
import sys
import getopt


class Sender(object):
    def __init__(self, common_seed: bytes, sender_size: int, matrix_width: int = 128):
        self.sender_size = sender_size
        self.PsiSender = OprfPsiSender(common_seed, sender_size, matrix_width)

    def gen_public_param(self):
        return self.PsiSender.gen_public_param()

    def gen_matrix_T_xor_R(self, pk0s: bytes):
        return self.PsiSender.gen_matrix_T_xor_R(pk0s)

    def recover_matrix_C(self, recv_matrix_A_xor_D: bytes, sender_set: np.array):
        sender_size = len(sender_set)
        if sender_size != self.sender_size:
            raise Exception('oprf psi sender: data_size is not equal,error')
        self.PsiSender.recover_matrix_C(recv_matrix_A_xor_D, sender_set)

    def is_sender_end(self) -> bool:
        return self.PsiSender.is_sender_end()

    def compute_hash2_output_to_receiver(self):
        return self.PsiSender.compute_hash2_output_to_receiver()


class Client(object):
    def __init__(self, host, port):
        self.buf_size = 100 * 1024 * 1024
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 连接服务，指定主机和端口

        self.s.connect((host, port))

    def send_data(self, msg):
        head = len(msg)
        head_bytes = self.int_to_bytes(head)
        self.s.sendall(head_bytes)
        self.s.sendall(msg)

    def recv_data(self):
        head = bytes()
        head_len = 0
        while True:
            tmp = self.s.recv(4 - head_len)
            tmp_n = len(tmp)
            head += tmp
            head_len += tmp_n
            if head_len < 4:
                continue
            else:
                break
        data_len = self.byte_to_int(head)
        data = bytes()
        data_recv = 0
        while True:
            ret = self.s.recv(data_len - data_recv)
            ret_len = len(ret)
            data += ret
            data_recv += ret_len
            if data_recv < data_len:
                continue
            else:
                break
        return data

    def int_to_bytes(self, n: int):
        bys = bytearray(4)
        bys[3] = (n >> 24) & 0xff
        bys[2] = (n >> 16) & 0xff
        bys[1] = (n >> 8) & 0xff
        bys[0] = n & 0xff
        return bytes(bys)

    def byte_to_int(self, b: bytes):
        if len(b) == 4:
            b1 = b[0] & 0xff
            b2 = (b[1] << 8) & 0xff00
            b3 = (b[2] << 16) & 0xff0000
            b4 = (b[3] << 24) & 0xff000000
            return b1 + b2 + b3 + b4
        else:
            print("len:", len(b))
            raise Exception('length error')

    def test_gen_data_set(self, n: int, psi_size: int = 200000) -> np.array:
        ls = [''] * n
        for i in range(0, psi_size):
            ls[i] = sha256(str(i).encode('utf-8')
                           ).hexdigest()[:21].encode('utf-8')
            # ls[i] = sha256(str(i).encode('utf-8')).hexdigest()[:21]
        for i in range(psi_size, n):
            ls[i] = sha256((str(i) + "xx").encode('utf-8')
                           ).hexdigest()[:21].encode('utf-8')
            # ls[i] = sha256((str(i) + "xx").encode('utf-8')).hexdigest()[:21]
        return np.array(ls)

    def generate_dataset_debug(self, dataSize: int, psiSize: int = 200000,
                               seed: int = 11, ids: int = 21):
        return generate_dataset_debug(1, dataSize, psiSize, seed, ids)


def parse_args(argv):
    psi_size, sender_size, receiver_size, ip, port = 50, 200, 200, '127.0.0.1', 8888
    if len(argv[1:]) == 0:
        print('test.py --rs <500> --ss <500> --ps <100>')
        sys.exit(2)
    try:
        opts, args = getopt.getopt(
            argv[1:], None, ["rs=", "ss=", "ps=", "ip=", "port=", "help="])
    except getopt.GetoptError:
        print('test.py --rs <500> --ss <500> --ps <100>')
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("--help"):
            print('test.py --rs <500> --ss <500> --ps <100>')
            sys.exit()
        if opt in ('--rs'):
            receiver_size = int(arg)
        if opt in ('--ss'):
            sender_size = int(arg)
        if opt in ('--ps'):
            psi_size = int(arg)
        if opt in ('--ip'):
            ip = arg
        if opt in ('--port'):
            port = int(arg)
    return receiver_size, sender_size, psi_size, ip, port


if __name__ == "__main__":
    receiver_size, sender_size, psi_size, ip, port = parse_args(sys.argv)
    print('receiver_size, sender_size, psi_size, ip, port=',
          receiver_size, sender_size, psi_size, ip, port)
    print("===================")
    client = Client(ip, port)
    # sender_set = client.test_gen_data_set(sender_size, psi_size)
    sender_set = client.generate_dataset_debug(sender_size, psi_size)
    # 1. 双方首先协商的公共种子
    common_seed = b'1111111111111112'  # bytearray(b'1111111111111112')
    # 2. 创建接收方对象
    psi_sender = Sender(common_seed, sender_size)
    # 3. 本地生成公共参数public_param
    pub_param, pub_param_byte_size = psi_sender.gen_public_param()
    print("+++", len(pub_param), pub_param, pub_param_byte_size)
    assert len(pub_param) == pub_param_byte_size
    # 4. 发送公共参数给对方
    client.send_data(pub_param)
    # 5.接收对方发来的公钥pk0s,作为输入，生成matrix_TxorR矩阵
    pk0s = client.recv_data()
    matrix_TxorR, _ = psi_sender.gen_matrix_T_xor_R(pk0s)
    # print('===>>matrix_TxorR:', matrix_TxorR, len(matrix_TxorR))
    # 6.将T_xor_R发送给对方
    client.send_data(matrix_TxorR)
    # 7.接收对方发来的 矩阵matrix_A_xor_D
    matrix_A_xor_D = client.recv_data()
    # 8.恢复矩阵C,本接口没有输出
    psi_sender.recover_matrix_C(matrix_A_xor_D, sender_set)
    # 9.循环发送数据到对方
    while psi_sender.is_sender_end() == False:
        hash2_output_val, _ = psi_sender.compute_hash2_output_to_receiver()
        # 发送数据
        client.send_data(hash2_output_val)
    print('===>>发送方结束...')
    pass
