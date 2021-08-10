from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
import numpy as np
import time


# from libcpp.string cimport string as cppstr
# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)


cdef extern from "psi.h" namespace "osuCrypto":
    ctypedef unsigned long long u64_t
    ctypedef unsigned char u8_t
    ctypedef unsigned int u32_t
    cdef cppclass PsiSender:
        PsiSender()except+
        int init(u8_t *commonSeed, u64_t senderSize, u64_t matrixWidth, u64_t logHeight,
                 u64_t hash2LengthInBytes, u64_t bucket2ForComputeH2Output)
        int genPublicParamFromNpot(u8_t ** pubParamBuf, u64_t *pubParamBufByteSize)
        int genMatrixTxorRBuff(u8_t *pk0Buf, const u64_t pk0BufSize,
                               u8_t ** uBuffOutputTxorR, u64_t *uBuffOutputSize)
        int recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize,
                           const vector[vector[u8_t]] senderSet)
        int isSendEnd()
        int computeHashOutputToReceiverOnce(u8_t *sendBuff, u64_t *sendBuffSize)

    cdef cppclass PsiReceiver:
        PsiReceiver()except+
        int init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize,
                 u64_t matrixWidth, u64_t logHeight, u64_t hash2LengthInBytes,
                 u64_t bucket2ForComputeH2Output)
        int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                           u8_t ** pk0Buf, u64_t *pk0BufSize)
        int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
                                const vector[vector[u8_t]] receiverSet,
                                u8_t ** sendMatrixADBuff, u64_t *sendMatixADBuffSize)
        int genenateAllHashesMap()
        int isRecvEnd()
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                            vector[u32_t] *psiMsgIndex)

#Opef psi receiver
cdef class OprfPsiReceiver:
    cdef PsiReceiver psi_receiver
    #common_seed:16字节的bytes，双方必须做到统一
    def __init__(self, common_seed: bytes, receiver_size: int, sender_size: int, matrix_width: int = 128):
        if common_seed is None or len(common_seed) < 16:
            raise Exception('oprf psi receiver: param error')
        # cdef u8_t *commonSend = common_seed
        cdef int ret = self.psi_receiver.init(common_seed, receiver_size, sender_size, matrix_width, 20, 10, 10240)
        if ret != 0:
            raise Exception('oprf psi receiver: init error')

    def gen_pk0s(self, public_param: bytes):
        # cdef u8_t *pubParamBuf = public_param
        cdef u64_t pubParamBufByteSize = len(public_param)
        cdef u8_t *pk0s
        cdef u64_t pk0BufSize
        cdef int ret = self.psi_receiver.genPK0FromNpot(public_param, pubParamBufByteSize, &pk0s, &pk0BufSize)
        if ret != 0:
            raise Exception('oprf psi receiver: generate pk0s error')
        pk0s_val = bytes(pk0BufSize)
        string.memcpy(<u8_t*> pk0s_val, pk0s, pk0BufSize)
        return pk0s_val, pk0BufSize

    def gen_matrix_A_xor_D(self, matrix_TxorR: bytes, receiver_set: np.array):
        # cdef u8_t *uBuffInputTxorR = matrix_TxorR
        cdef u64_t uBuffInputSize = len(matrix_TxorR)
        cdef u8_t *matrix_AxorD_buff
        cdef u64_t matrix_AxorD_buff_size
        start = time.time_ns()
        # cdef vector[vector[u8_t]] receiverSet = <vector[vector[u8_t]]> receiver_set
        cdef vector[vector[u8_t]] receiverSet
        recv_size = len(receiver_set)
        receiverSet.resize(recv_size)
        for i, v in enumerate(receiver_set):
            # receiverSet[i] = <vector[u8_t]> receiver_set[i]  #.encode('utf-8')
            receiverSet[i] = receiver_set[i].encode('utf-8')
        end = time.time_ns()
        print('===>>循环赋值recvset用时:{}ms'.format((end - start) / 1e6))
        cdef int ret = self.psi_receiver.getSendMatrixADBuff(matrix_TxorR, uBuffInputSize, receiverSet,
                                                             &matrix_AxorD_buff, &matrix_AxorD_buff_size)
        if ret != 0:
            raise Exception('oprf psi receiver: generate matrix A_xor_D error')
        matrix_AxorD_val = bytes(matrix_AxorD_buff_size)
        start = time.time_ns()
        string.memcpy(<u8_t*> matrix_AxorD_val, matrix_AxorD_buff, matrix_AxorD_buff_size)
        end = time.time_ns()
        print('===>>matrix_AxorD_val copy用时:{}ms'.format((end - start) / 1e6))
        return matrix_AxorD_val, matrix_AxorD_buff_size

    def gen_all_hash2_map(self):
        cdef int ret = self.psi_receiver.genenateAllHashesMap()
        if ret != 0:
            raise Exception('oprf psi receiver: generate all hash2 output map error')

    #int isRecvEnd()
    def is_receiver_end(self)-> bool:
        cdef int ret = self.psi_receiver.isRecvEnd()
        if ret == 1:
            return True
        return False

    def compute_psi_by_hash2_output(self, hash2_from_sender: bytes)-> list:
        # cdef u8_t *recvBuff = hash2_from_sender
        cdef u64_t recvBufSize = len(hash2_from_sender)
        cdef vector[u32_t] psi_msg_index
        cdef int ret = self.psi_receiver.recvFromSenderAndComputePSIOnce(hash2_from_sender, recvBufSize, &psi_msg_index)
        if ret != 0:
            raise Exception('oprf psi receiver: compute psi by hash2 output from sender error')
        return <list> psi_msg_index

#Oprf psi sender
cdef class OprfPsiSender(object):
    cdef PsiSender psi_sender
    #common_seed:16字节的bytes，双方必须做到统一
    def __init__(self, common_seed: bytes, sender_size: int, matrix_width: int = 128):
        # self.sender_size = sender_size
        if common_seed is None or len(common_seed) < 16:
            raise Exception('oprf psi sender: param error')
        # cdef u8_t *commonSeed = common_seed
        cdef int ret = self.psi_sender.init(common_seed, sender_size, matrix_width, 20, 10, 10240)
        if ret != 0:
            raise Exception('oprf psi sender: init error')

    def gen_public_param(self):
        # pub_param = bytearray()
        cdef u8_t *pub_param
        cdef u64_t pub_param_byte_size
        cdef int ret = self.psi_sender.genPublicParamFromNpot(&pub_param, &pub_param_byte_size)
        if ret != 0:
            raise Exception('oprf psi sender: generate public param error')
        print('===>>pub_param:', pub_param)
        pub_param_val = bytes(pub_param_byte_size)
        string.memcpy(<u8_t*> pub_param_val, pub_param, pub_param_byte_size)
        return pub_param_val, pub_param_byte_size

    def gen_matrix_T_xor_R(self, pk0s: bytes):
        # cdef u8_t *pk0Buf = pk0s
        cdef u64_t pk0BufSize = len(pk0s)
        cdef u8_t *uBuffOutputTxorR
        cdef u64_t uBuffOutputSize
        cdef int ret = self.psi_sender.genMatrixTxorRBuff(pk0s, pk0BufSize, &uBuffOutputTxorR, &uBuffOutputSize)
        if ret != 0:
            raise Exception('oprf psi sender: generate matrix T_xor_R error')
        T_xor_R = bytes(uBuffOutputSize)
        string.memcpy(<u8_t*> T_xor_R, uBuffOutputTxorR, uBuffOutputSize)
        return T_xor_R, uBuffOutputSize

    def recover_matrix_C(self, recv_matrix_A_xor_D: bytes, sender_set: np.array):
        # cdef u8_t *recvMatrixADBuff = recv_matrix_A_xor_D
        cdef u64_t recvMatixADBuffSize = len(recv_matrix_A_xor_D)
        # cdef vector[vector[u8_t]] senderSet = <vector[vector[u8_t]]> sender_set

        cdef vector[vector[u8_t]] senderSet
        sender_size = len(sender_set)
        senderSet.resize(sender_size)
        for i, v in enumerate(sender_set):
            # senderSet[i] = <vector[u8_t]> sender_set[i]  #.encode('utf-8')
            senderSet[i] = sender_set[i].encode('utf-8')
        cdef int ret = self.psi_sender.recoverMatrixC(recv_matrix_A_xor_D, recvMatixADBuffSize, senderSet)
        if ret != 0:
            raise Exception('oprf psi sender: recover matrix_C error')

    def is_sender_end(self)-> bool:
        cdef int ret = self.psi_sender.isSendEnd()
        if ret == 1:
            return True
        return False

    def compute_hash2_output_to_receiver(self):
        hash2_output_val = bytes(102400)
        cdef u8_t *hash2_output_buff = hash2_output_val
        cdef u64_t hash2_output_buff_size
        cdef int ret = self.psi_sender.computeHashOutputToReceiverOnce(hash2_output_buff, &hash2_output_buff_size)
        if ret != 0:
            raise Exception('oprf psi sender: compute hash2 output error')
        # hash2_output_val = bytes(hash2_output_buff_size)
        # string.memcpy(<u8_t*> hash2_output_val, hash2_output_buff, hash2_output_buff_size)

        return hash2_output_buff[:hash2_output_buff_size], hash2_output_buff_size
