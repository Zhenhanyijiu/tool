from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
import numpy as np
# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)
cdef extern from "psi.h" namespace "osuCrypto":
    ctypedef unsigned long int u64_t
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
        int computeHashOutputToReceiverOnce(u8_t ** sendBuff, u64_t *sendBuffSize)

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
    cdef vector[u32_t] psi_msg_index
    def __init__(self, common_seed: bytearray, receiver_size: int, sender_size: int, matrix_width: int = 128):
        self.receiver_size = receiver_size
        self.sender_size = sender_size
        if common_seed is None or len(common_seed) < 16:
            raise Exception('oprf psi receiver: param error')
        cdef u8_t *commonSend = common_seed
        cdef int ret = self.psi_receiver.init(commonSend, receiver_size, sender_size, matrix_width, 20, 10, 1024)
        if ret != 0:
            raise Exception('oprf psi receiver: init error')
    # int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
    #                            u8_t ** pk0Buf, u64_t *pk0BufSize)
    def gen_pk0s(self, public_param: bytearray):
        cdef u8_t *pubParamBuf = public_param
        cdef u64_t pubParamBufByteSize = len(public_param)
        cdef u8_t *pk0s
        cdef u64_t pk0BufSize
        cdef int ret = self.psi_receiver.genPK0FromNpot(pubParamBuf, pubParamBufByteSize, &pk0s, &pk0BufSize)
        if ret != 0:
            raise Exception('oprf psi receiver: generate pk0s error')
        return pk0s, pk0BufSize
    # int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
    #                                 const vector[vector[u8_t]] receiverSet,
    #                                 u8_t ** sendMatrixADBuff, u64_t *sendMatixADBuffSize)
    def gen_matrix_A_xor_D(self, matrix_TxorR: bytearray, receiver_set: np.ndarray):
        recv_size = receiver_set.size
        if recv_size != self.receiver_size:
            raise Exception('oprf psi receiver: data_size is not equal,error')
        cdef u8_t *uBuffInputTxorR = matrix_TxorR
        cdef u64_t uBuffInputSize = len(matrix_TxorR)
        cdef u8_t *matrix_AxorD_buff
        cdef u64_t matrix_AxorD_buff_size
        cdef vector[vector[u8_t]] receiverSet
        cdef int ret = self.psi_receiver.getSendMatrixADBuff(uBuffInputTxorR, uBuffInputSize, receiverSet,
                                                             &matrix_AxorD_buff, &matrix_AxorD_buff_size)
        if ret != 0:
            raise Exception('oprf psi receiver: generate matrix A_xor_D error')
        return matrix_AxorD_buff, matrix_AxorD_buff_size
    # int genenateAllHashesMap()
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

    #int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
    # vector[u32_t] *psiMsgIndex)
    def compute_psi_by_hash2_output(self, hash2_from_sender: bytearray):
        cdef u8_t*recvBuff
        cdef u64_t recvBufSize = len(hash2_from_sender)
        cdef int ret = self.psi_receiver.recvFromSenderAndComputePSIOnce(recvBuff, recvBufSize, &self.psi_msg_index)
        if ret != 0:
            raise Exception('oprf psi receiver: compute psi by hash2 output from sender error')
    def get_psi_result_for_index(self)-> list:
        psi_num = self.psi_msg_index.size()
        result = <list> self.psi_msg_index
        # for
        return result

#Oprf psi sender
cdef class OprfPsiSender:
    cdef PsiSender psi_sender
    def __init__(self, common_seed: bytearray, sender_size: int, matrix_width: int = 128):
        self.sender_size = sender_size
        if common_seed is None or len(common_seed) < 16:
            raise Exception('oprf psi sender: param error')
        cdef u8_t *commonSeed = common_seed
        cdef int ret = self.psi_sender.init(commonSeed, sender_size, matrix_width, 20, 10, 1024)
        if ret != 0:
            raise Exception('oprf psi sender: init error')

    def gen_public_param(self):
        # pub_param = bytearray()
        cdef u8_t *pub_param
        cdef u64_t pub_param_byte_size
        cdef int ret = self.psi_sender.genPublicParamFromNpot(&pub_param, &pub_param_byte_size)
        if ret != 0:
            raise Exception('oprf psi sender: generate public param error')
        return pub_param, pub_param_byte_size
    def gen_matrix_T_xor_R(self, pk0s: bytearray):
        cdef u8_t *pk0Buf = pk0s
        cdef u64_t pk0BufSize = len(pk0s)
        cdef u8_t *uBuffOutputTxorR
        cdef u64_t uBuffOutputSize
        cdef int ret = self.psi_sender.genMatrixTxorRBuff(pk0Buf, pk0BufSize, &uBuffOutputTxorR, &uBuffOutputSize)
        if ret != 0:
            raise Exception('oprf psi sender: generate matrix T_xor_R error')
        return uBuffOutputTxorR, uBuffOutputSize

    def recover_matrix_C(self, recv_matrix_A_xor_D: bytearray, sender_set: np.ndarray):
        sender_size = sender_set.size
        if sender_size != self.sender_size:
            raise Exception('oprf psi sender: data_size is not equal,error')
        cdef u8_t *recvMatrixADBuff = recv_matrix_A_xor_D
        cdef u64_t recvMatixADBuffSize = len(recv_matrix_A_xor_D)
        cdef vector[vector[u8_t]] senderSet
        senderSet.resize(sender_size)
        for i, v in enumerate(sender_set):
            senderSet[i] = sender_set[i].encode('utf-8')
        cdef int ret = self.psi_sender.recoverMatrixC(recvMatrixADBuff, recvMatixADBuffSize, senderSet)
        if ret != 0:
            raise Exception('oprf psi sender: recover matrix_C error')

    def is_sender_end(self)-> bool:
        cdef int ret = self.psi_sender.isSendEnd()
        if ret == 1:
            return True
        return False

    def compute_hash2_output_to_receiver(self):
        cdef u8_t *hash2_output_buff
        cdef u64_t hash2_output_buff_size
        cdef int ret = self.psi_sender.computeHashOutputToReceiverOnce(&hash2_output_buff, &hash2_output_buff_size)
        if ret != 0:
            raise Exception('oprf psi sender: compute hash2 output error')
        return hash2_output_buff, hash2_output_buff_size
