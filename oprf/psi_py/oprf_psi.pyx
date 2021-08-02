from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
# import numpy as np
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
        # ~PsiSender()
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
                 u64_t matrixWidth, u64_t logHeight, u64_t hash2LengthInBytes = 10,
                 u64_t bucket2ForComputeH2Output = 256)
        int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                           u8_t ** pk0Buf, u64_t *pk0BufSize)
        int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
                                const vector[vector[u8_t]] receiverSet,
                                u8_t ** sendMatrixADBuff, u64_t *sendMatixADBuffSize)
        int genenateAllHashesMap()
        int isRecvEnd()
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                            vector[u32_t] *psiMsgIndex)


cdef class OprfPsiSender:
    cdef PsiSender psi_sender
    def __init__(self, common_seed: bytearray, sender_size: int, matrixWidth: int = 128):
        self.sender_size = sender_size
        if common_seed is None or len(common_seed) < 16:
            raise Exception('oprf psi sender param error')
        cdef u8_t * commonSeed = common_seed
        cdef int ret = self.psi_sender.init(commonSeed, sender_size, matrixWidth, 20, 10, 256)
        if ret != 0:
            raise Exception('oprf psi sender init error')

    def gen_public_param(self):
        # pub_param = bytearray()
        cdef u8_t * pub_param
        cdef u64_t pub_param_byte_size
        cdef int ret = self.psi_sender.genPublicParamFromNpot(&pub_param, &pub_param_byte_size)
        if ret != 0:
            raise Exception('oprf psi sender generate public param error')
        return pub_param, pub_param_byte_size
    def gen_matrix_T_xor_R(self, pk0s: bytearray):
        cdef u8_t *pk0Buf = pk0s
        cdef u64_t pk0BufSize = len(pk0s)
        cdef u8_t *uBuffOutputTxorR
        cdef u64_t uBuffOutputSize
        cdef int ret = self.psi_sender.genMatrixTxorRBuff(pk0Buf, pk0BufSize, uBuffOutputTxorR, uBuffOutputSize)
        if ret != 0:
            raise Exception('oprf psi sender generate matrix T_xor_R error')
        return uBuffOutputTxorR, uBuffOutputSize
    # int recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize,
    #                            const vector[vector[u8_t]] senderSet)
    def recover_matrix_C(self, recv_matrix_A_xor_D: bytearray, sender_set: bytearray):

        return
