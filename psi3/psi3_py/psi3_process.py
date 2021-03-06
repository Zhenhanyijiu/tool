from psi3 import psi3_process_by_boost
import numpy as np
from hashlib import sha256, md5
import sys
import getopt
import time
import socket


# from Crypto.Cipher import AES


class Receiver(object):
    def __init__(self, common_deed: bytes, receiver_size: int, sender_size: int,
                 omp_thread_num: int = 1, matrix_width: int = 128):
        # self.psi_msg_index = list()
        self.receiver_size = receiver_size
        self.sender_size = sender_size
        self.PsiReceiver = OprfPsiReceiver(common_deed, receiver_size, sender_size,
                                           omp_thread_num, matrix_width)

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

    # def compute_psi_by_hash2_output(self, hash2_from_sender: bytes):
    #     psi_index = self.PsiReceiver.compute_psi_by_hash2_output(
    #         hash2_from_sender)
    #     self.psi_msg_index += psi_index

    def compute_psi_by_hash2_output(self, hash2_from_sender: bytes):
        self.PsiReceiver.compute_psi_by_hash2_output(hash2_from_sender)

    def get_psi_results_for_all(self) -> list:
        return self.PsiReceiver.get_psi_results_for_all()


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
            if tmp_n == 0:
                raise Exception("conn closed")
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
            if ret_len == 0:
                raise Exception("conn closed")
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


def test_gen_data_set(n: int, psi_size: int = 200000) -> np.array:
    ls = [b''] * n
    for i in range(0, n):
        ls[i] = md5(str(i).encode('utf-8')
                    ).hexdigest()[:21].encode('utf-8')
    return np.array(ls)


def parse_args(argv):
    psi_size, sender_size, receiver_size, ip, port, omp_thread_num = 50, 200, 200, '127.0.0.1', 8888, 1
    if len(argv[1:]) == 0:
        print('test.py --rs <500> --ss <500> --ps <100>')
        sys.exit(2)
    try:
        opts, args = getopt.getopt(
            argv[1:], None, ["rs=", "ss=", "ps=", "ip=", "port=", "omp=", "help="])
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
        if opt in ('--omp'):
            omp_thread_num = int(arg)
    return receiver_size, sender_size, psi_size, ip, port, omp_thread_num


def parse_args_psi3(argv):
    p_id, set_size = 0, 12
    if len(argv[1:]) == 0:
        print('test.py --m <12> --p <0>')
        sys.exit(2)
    try:
        opts, args = getopt.getopt(
            argv[1:], None, ["m=", "p=", "help="])
    except getopt.GetoptError:
        print('test.py --m <12> --p <0>')
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("--help"):
            print('test.py --m <12> --p <0>')
            sys.exit()
        if opt in ('--m'):
            set_size = int(arg)
        if opt in ('--p'):
            p_id = int(arg)

    return p_id, set_size


def get_use_time(start: float) -> float:
    return (time.time() - start) * 1000


def recv_process(receiver_size, sender_size, psi_size, ip, port, omp_thread_num: int = 1):
    server = Server(ip, port)
    start0 = time.time()
    receiver_set = test_gen_data_set(receiver_size, psi_size)
    print("###>>???????????????????????????{}ms".format(get_use_time(start0)))
    # 1. ?????????????????????????????????
    # common_seed:16?????????bytes???????????????????????????
    common_seed = b'1111111111111112'
    # bytearray(b'1111111111111112')
    # 2. ?????????????????????
    start1 = time.time()
    psi_recv = Receiver(common_seed, receiver_size, sender_size, omp_thread_num)

    # 3. ?????????????????????????????????public_param
    public_param = server.recv_data()
    # 4. ?????????????????????????????????????????????????????????pk0s
    pk0s, pk0s_size = psi_recv.gen_pk0s(public_param)
    # 5.???pk0s???????????????
    server.send_data(pk0s)
    # 6.?????????????????????matrix_TxorR???????????????
    matrix_TxorR = server.recv_data()
    ### print('===>>matrix_TxorR:', matrix_TxorR, len(matrix_TxorR))
    # 7.????????????matrix_A_xor_D
    start2 = time.time()
    print("###>>??????????????????matrix_A_xor_D")
    matrix_A_xor_D, _ = psi_recv.gen_matrix_A_xor_D(matrix_TxorR, receiver_set)
    print("###>>??????matrix_A_xor_D?????????{}ms".format(get_use_time(start2)))
    # 8.????????????matrix_A_xor_D?????????
    server.send_data(matrix_A_xor_D)
    # 9.???????????????????????????hash2_output_map,?????????????????????
    start3 = time.time()
    psi_recv.gen_all_hash2_map()
    print("###>>??????hash2 map?????????{}ms".format(get_use_time(start3)))
    # 10.??????????????????????????????????????????
    start4 = time.time()
    count_debug = 0
    while psi_recv.is_receiver_end() == False:
        hash2_output_val = server.recv_data()
        # print("===>>hash2_output_val length:", len(hash2_output_val))
        psi_recv.compute_psi_by_hash2_output(hash2_output_val)
        count_debug += 1
    # 11.????????????psi??????
    psi_results = psi_recv.get_psi_results_for_all()
    print("###>>???????????????{}ms,???????????????{}".format(get_use_time(start4), count_debug))
    # print("###>>recv????????????{}ms".format(get_use_time(start1)))
    print('###>>???????????????????????????', psi_results[-1])
    assert psi_size == (psi_results[-1] + 1)
    print("###>>??????????????????:{}".format(get_use_time(start1)))
    print('###>>?????????????????????...')
    pass


def psi3_process_test(p_id: int, set_size: int):
    ip_array = ["127.0.0.1", "127.0.0.2", "127.0.0.3"]
    ip_array_byte = [x.encode('utf-8') for x in ip_array]
    # ??????????????????
    psi_n = 30
    ls = [b''] * set_size
    print("===>>set_size:", set_size)
    for i in range(0, psi_n):
        ls[i] = md5(str(i).encode('utf-8')).hexdigest()[:16].encode('utf-8')

    for i in range(30, set_size):
        ls[i] = md5((str(i) + 'qqq' + str(p_id)).encode('utf-8')).hexdigest()[:16].encode('utf-8')
    psi_results = psi3_process_by_boost(p_id, set_size, ip_array_byte, np.array(ls))
    print("===>>psi_result:", psi_results, len(psi_results))


if __name__ == '__main__':
    # receiver_size, sender_size, psi_size, ip, port, omp_thread_num = parse_args(sys.argv)
    # print('receiver_size, sender_size, psi_size, ip, port=',
    #       receiver_size, sender_size, psi_size, ip, port, omp_thread_num)
    p_id, set_size = parse_args_psi3(sys.argv)
    print('p_id, set_size=', p_id, set_size)
    psi3_process_test(p_id, set_size)
    print("=========psi3 process==========")
    is_socket_test = False
    # for i in range(1):
    #     # ???????????????????????????socket???oprf-psi??????demo
    #     if is_socket_test == False:
    #         print("======no socket test ======")
    #         recv_process(receiver_size, sender_size, psi_size, ip, port, omp_thread_num)
    #     # ????????????????????????socket???oprf-psi??????demo
    #     if is_socket_test:
    #         print("====== socket test ======")
    #         start00 = time.time()
    #         receiver_set = test_gen_data_set(receiver_size, psi_size)
    #         print("###>>gen test data time:{}ms".format(get_use_time(start00)))
    #         # common_seed:16?????????bytes???????????????????????????
    #         common_seed = b'1111111111111112'
    #         start00 = time.time()
    #         psiResults = oprf_psi_receiver_by_socket(receiver_size, sender_size, receiver_set,
    #                                                  ip.encode("utf-8"), port, common_seed, omp_thread_num)
    #         print("###>>psiResults length:{},last index:{}".format(len(psiResults), psiResults[-1]))
    #         print("###>>oprf_psi_receiver_by_socket time:{}ms".format(get_use_time(start00)))
    #     print("i:{}###>>end".format(i))
    #     # sl = 32
    #     # time.sleep(30)
    #     # print("??????{}s".format(sl))
