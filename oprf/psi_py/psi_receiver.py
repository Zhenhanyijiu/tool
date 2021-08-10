from oprf_psi import OprfPsiReceiver
import numpy as np
from hashlib import sha256, md5
import sys
import getopt
import time
import socket
from Crypto.Cipher import AES


class Receiver(object):
    def __init__(self, common_deed: bytes, receiver_size: int, sender_size: int, matrix_width: int = 128):
        self.psi_msg_index = list()
        self.receiver_size = receiver_size
        self.sender_size = sender_size
        self.PsiReceiver = OprfPsiReceiver(
            common_deed, receiver_size, sender_size, matrix_width)

    def gen_pk0s(self, public_param: bytes):
        return self.PsiReceiver.gen_pk0s(public_param)

    def gen_matrix_A_xor_D(self, matrix_TxorR: bytes, receiver_set: np.array):
        recv_size = len(receiver_set)
        if recv_size != self.receiver_size:
            raise Exception('oprf psi receiver: data_size is not equal,error')
        return self.PsiReceiver.gen_matrix_A_xor_D(matrix_TxorR, receiver_set)

    def gen_all_hash2_map(self):
        self.PsiReceiver.gen_all_hash2_map()

    def is_receiver_end(self) -> bool:
        return self.PsiReceiver.is_receiver_end()

    def compute_psi_by_hash2_output(self, hash2_from_sender: bytes):
        psi_index = self.PsiReceiver.compute_psi_by_hash2_output(
            hash2_from_sender)
        self.psi_msg_index += psi_index


class Server(object):
    def __init__(self, host, port):
        # self.buf_size = 100 * 1024 * 1024
        serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serversocket.bind((host, port))
        serversocket.listen(1)
        self.clientsocket, self.addr = serversocket.accept()
        print('accept ok...')

    def send_data(self, msg):
        head = len(msg)
        head_bytes = self.int_to_bytes(head)
        self.clientsocket.sendall(head_bytes)
        self.clientsocket.sendall(msg)
        # print("send n:", n)

    def recv_data(self):
        head = bytes()
        head_len = 0
        while True:
            tmp = self.clientsocket.recv(4 - head_len)
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
            ret = self.clientsocket.recv(data_len - data_recv)
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
        ls = [b''] * n
        for i in range(0, n):
            ls[i] = md5(str(i).encode('utf-8')).hexdigest()[:21]
        return np.array(ls)


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


def get_use_time(start: int) -> float:
    return (time.time_ns() - start) / 1e6


if __name__ == '__main__':
    receiver_size, sender_size, psi_size, ip, port = parse_args(sys.argv)
    print('receiver_size, sender_size, psi_size, ip, port=',
          receiver_size, sender_size, psi_size, ip, port)
    print("===================")
    server = Server(ip, port)
    start0 = time.time_ns()
    receiver_set = server.test_gen_data_set(receiver_size, psi_size)

    print("===>>生成测试数据用时：{}ms".format(get_use_time(start0)))
    # 1. 双方首先协商的公共种子
    # common_seed:16字节的bytes，双方必须做到统一
    common_seed = b'1111111111111112'
    # bytearray(b'1111111111111112')
    # 2. 创建接收方对象
    start1 = time.time_ns()
    psi_recv = Receiver(common_seed, receiver_size, sender_size)

    # 3. 接收对方发来的公共参数public_param
    public_param = server.recv_data()
    # 4. 将公共参数作为输入，生成协议所需的公钥pk0s
    pk0s, pk0s_size = psi_recv.gen_pk0s(public_param)
    # 5.将pk0s发送给对方
    server.send_data(pk0s)
    # 6.接收对方发来的matrix_TxorR，作为参数
    matrix_TxorR = server.recv_data()
    # print('===>>matrix_TxorR:', matrix_TxorR, len(matrix_TxorR))
    # 7.生成矩阵matrix_A_xor_D
    start2 = time.time_ns()
    matrix_A_xor_D, _ = psi_recv.gen_matrix_A_xor_D(matrix_TxorR, receiver_set)
    print("===>>生成matrix_A_xor_D用时：{}ms".format(get_use_time(start2)))
    # 8.发送矩阵matrix_A_xor_D给对方
    server.send_data(matrix_A_xor_D)
    # 9.生成本方数据的所有hash2_output_map,本接口没有输出
    start3 = time.time_ns()
    psi_recv.gen_all_hash2_map()
    print("===>>生成hash2 map用时：{}ms".format(get_use_time(start3)))
    # 10.循环接收数据，并匹配交集数据
    start4 = time.time_ns()
    count_debug = 0
    while psi_recv.is_receiver_end() == False:
        hash2_output_val = server.recv_data()
        psi_recv.compute_psi_by_hash2_output(hash2_output_val)
        count_debug += 1
    print("===>>匹配用时：{}ms,循环次数：{}".format(get_use_time(start4), count_debug))
    print("===>>recv总用时：{}ms".format(get_use_time(start1)))
    print('交集最后一个元素：', psi_recv.psi_msg_index[-1])
    print('===>>接收方求交结束...')
    assert psi_size == (psi_recv.psi_msg_index[-1] + 1)
    # psi_recv.gen_matrix_A_xor_D()
    pass
