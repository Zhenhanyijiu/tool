from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
cdef extern from "cryptoTools/Common/Defines.h" namespace "osuCrypto":
    ctypedef uint64_t u64
    typedef uint8_t u8
    