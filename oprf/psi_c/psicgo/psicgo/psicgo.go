package psicgo

/*
#cgo CFLAGS: -I./include
#cgo LDFLAGS: -L./lib -lpsi
#include<string.h>
#include "psi_c.h"
*/
import "C"
import (
	"errors"
	"reflect"
	"unsafe"
)

type PsiSender struct {
	sender     unsafe.Pointer
	senderSize uint64
	ompNum     int
}

//void *new_psi_sender(char *common_seed, ui64 sender_size, int omp_num);
func NewPsiSender(comSeed []byte, senderSize uint64, ompNum int) (*PsiSender, error) {
	if ompNum < 1 {
		ompNum = 1
	}
	if len(comSeed) != 16 {
		return nil, errors.New("common seed error for length")
	}
	common_seed := (*C.char)(unsafe.Pointer(&comSeed[0]))
	sender := C.new_psi_sender(common_seed, C.ulonglong(senderSize), C.int(ompNum))
	if sender == nil {
		return nil, errors.New("create psisender handle error")
	}
	return &PsiSender{
		sender:     sender,
		senderSize: senderSize,
		ompNum:     ompNum,
	}, nil
}

//void release_psi_sender(void *psi_s);
func (s *PsiSender) Release() {
	C.release_psi_sender(s.sender)
}

//int gen_public_param(void *psi_s, char **pub_param_buf, ui64 *pub_param_buf_size);
func (s *PsiSender) GenPublicParam() (pubParam []byte, err error) {
	var pub_param_buf *C.char
	var pub_param_buf_size uint64
	ret := C.gen_public_param(s.sender, &pub_param_buf, (*C.ulonglong)(unsafe.Pointer(&pub_param_buf_size)))
	if int(ret) != 0 {
		return nil, errors.New("gen_public_param error for C")
	}
	pubParam = make([]byte, pub_param_buf_size)
	C.memcpy(unsafe.Pointer(&pubParam[0]), unsafe.Pointer(pub_param_buf), C.ulong(pub_param_buf_size))
	return pubParam, nil
}

//int gen_matrix_T_xor_R(void *psi_s, char *pk0s, const ui64 pk0s_size,
//                           char **matrix_TxorR, ui64 *matrix_TxorR_size);
func (s *PsiSender) GenMatrixTxorR(pk0s []byte) (matrixTxorR []byte, err error) {
	head := (*reflect.SliceHeader)(unsafe.Pointer(&pk0s))
	pk0Char := (*C.char)(unsafe.Pointer(head.Data))
	pk0s_size := C.ulonglong(head.Len)
	var matrix_TxorR *C.char
	var matrix_TxorR_size uint64 = 0
	ret := C.gen_matrix_T_xor_R(s.sender, pk0Char, pk0s_size, &matrix_TxorR,
		(*C.ulongulong)(unsafe.Pointer(&matrix_TxorR_size)))
	if int(ret) != 0 {
		return nil, errors.New("gen_matrix_T_xor_R error for C")
	}
	matrixTxorR = make([]byte, matrix_TxorR_size)
	headDest := (*reflect.SliceHeader)(unsafe.Pointer(&matrixTxorR))
	C.memcpy(unsafe.Pointer(headDest.Data), unsafe.Pointer(matrix_TxorR), matrix_TxorR_size)
	return matrixTxorR, nil
}

//int compute_all_hash_output_byH1(void *psi_s, const char **sender_set,
//                                     const ui64 sender_size);
func (s *PsiSender) ComputeAllHashOutputByH1() {

}
