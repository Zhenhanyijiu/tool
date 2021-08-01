from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
cdef extern from "psi.h" namespace "osuCrypto":
    ctypedef unsigned long int u64_t
    ctypedef unsigned char u8_t
    ctypedef unsigned int u32_t
    