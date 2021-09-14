package psicgo

/*
#cgo CFLAGS:-DOMP_POOL
#cgo CFLAGS: -I./include
#cgo LDFLAGS: -L./lib -lpsi
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "psi_c.h"
void printsize(){
printf("==>>int:%ld\n",sizeof(unsigned int));
printf("==>>long:%ld\n",sizeof(unsigned long int));
printf("==>>long long:%ld\n",sizeof(unsigned long long int));
}
*/
import "C"
import (
	"errors"
	"fmt"
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
	sender := C.new_psi_sender(common_seed, C.ulong(senderSize), C.int(ompNum))
	if sender == nil {
		return nil, errors.New("create psisender handle error")
	}
	C.printsize()
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
	ret := C.gen_public_param(s.sender, &pub_param_buf, (*C.ulong)(unsafe.Pointer(&pub_param_buf_size)))
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
	pk0s_size := C.ulong(head.Len)
	var matrix_TxorR *C.char
	var matrix_TxorR_size uint64 = 0
	ret := C.gen_matrix_T_xor_R(s.sender, pk0Char, pk0s_size, &matrix_TxorR,
		(*C.ulong)(unsafe.Pointer(&matrix_TxorR_size)))
	if int(ret) != 0 {
		return nil, errors.New("gen_matrix_T_xor_R error for C")
	}
	matrixTxorR = make([]byte, matrix_TxorR_size)
	headDest := (*reflect.SliceHeader)(unsafe.Pointer(&matrixTxorR))
	C.memcpy(unsafe.Pointer(headDest.Data), unsafe.Pointer(matrix_TxorR), C.ulong(matrix_TxorR_size))
	return matrixTxorR, nil
}

//int compute_all_hash_output_byH1(void *psi_s, const char **sender_set,
//									const ui64 sender_size, const int id_size);
func (s *PsiSender) ComputeAllHashOutputByH1(senderSet []string, idSize int64) error {
	sender_size := len(senderSet)
	sender_set := make([]*C.char, sender_size)
	for i := 0; i < sender_size; i++ {
		sender_set[i] = C.CString(senderSet[i])
	}
	defer func() {
		for i := 0; i < sender_size; i++ {
			C.free(unsafe.Pointer(sender_set[i]))
		}
	}()
	head := (*reflect.SliceHeader)(unsafe.Pointer(&sender_set))
	ret := C.compute_all_hash_output_byH1(s.sender, (**C.char)(unsafe.Pointer(head.Data)),
		C.ulong(sender_size), C.int(len(senderSet[0])))
	if int(ret) != 0 {
		return errors.New("compute_all_hash_output_byH1 error for C")
	}
	return nil
}

//int recover_matrix_C(void *psi_s, const char *recv_matrix_A_xor_D,
//                         const ui64 recv_matrix_A_xor_D_size);
func (s *PsiSender) RecoverMatrixC(recvMatrixAxorD []byte) error {
	recv_matrix_A_xor_D_size := len(recvMatrixAxorD)
	head := (*reflect.SliceHeader)(unsafe.Pointer(&recvMatrixAxorD))
	ret := C.recover_matrix_C(s.sender, (*C.char)(unsafe.Pointer(head.Data)),
		C.ulong(recv_matrix_A_xor_D_size))
	if int(ret) != 0 {
		return errors.New("recover_matrix_C error for C")
	}
	return nil
}

//int is_send_end(void *psi_s);
func (s *PsiSender) IsSendEnd() bool {
	ret := C.is_send_end(s.sender)
	if int(ret) == 0 {
		return false
	}
	return true
}

//int compute_hash_output_to_receiver_once(void *psi_s,char **hashOutputBuff, ui64 *sendBuffSize);
func (s *PsiSender) ComputeHashOutputToReceiverOnce() (hashOutputBuff []byte, err error) {
	var hashOutput *C.char
	var sendBuffSize C.ulong
	ret := C.compute_hash_output_to_receiver_once(s.sender, &hashOutput, &sendBuffSize)
	if int(ret) != 0 {
		return nil, errors.New("compute_hash_output_to_receiver_once error for C")
	}

	hashOutputBuff = make([]byte, uint64(sendBuffSize))
	head := (*reflect.SliceHeader)(unsafe.Pointer(&hashOutput))
	C.memcpy(unsafe.Pointer(head.Data), unsafe.Pointer(hashOutput), C.ulong(sendBuffSize))
	return hashOutputBuff, nil
}

//oprf psi receiver
type PsiReceiver struct {
	receiver     unsafe.Pointer
	senderSize   uint64
	receiverSize uint64
	ompNum       int
}

//void *new_psi_receiver(char *common_seed, ui64 receiver_size, ui64 sender_size,
//                           int omp_num);
func NewPsiReceiver(comSeed []byte, receiverSize uint64, senderSize uint64, ompNum int) (*PsiReceiver, error) {
	if ompNum < 1 {
		ompNum = 1
	}
	if len(comSeed) != 16 {
		return nil, errors.New("common seed error for length")
	}
	common_seed := (*C.char)(unsafe.Pointer(&comSeed[0]))
	receiver := C.new_psi_receiver(common_seed, C.ulong(receiverSize), C.ulong(senderSize), C.int(ompNum))
	if receiver == nil {
		return nil, errors.New("create psireceiver handle error")
	}
	return &PsiReceiver{
		receiver:     receiver,
		senderSize:   senderSize,
		receiverSize: receiverSize,
		ompNum:       ompNum,
	}, nil
}

//void release_psi_receiver(void *psi_r);
func (r *PsiReceiver) Release() {
	C.release_psi_receiver(r.receiver)
}

//    int gen_pk0s_from_npot(void *psi_r, char *pubParamBuf, const ui64 pubParamBufByteSize,
//                           char **pk0Buf, ui64 *pk0BufSize);
func (r *PsiReceiver) GenPk0sFromNpot(pubParam []byte) (pk0s []byte, err error) {
	head := (*reflect.SliceHeader)(unsafe.Pointer(&pubParam))
	pubParamBuf := (*C.char)(unsafe.Pointer(head.Data))
	pubParamBufByteSize := C.ulong(head.Len)
	var pk0sbuf *C.char
	var pk0sBufSize C.ulong
	fmt.Printf("---------------\n")
	ret := C.gen_pk0s_from_npot(r.receiver, pubParamBuf, pubParamBufByteSize, &pk0sbuf, &pk0sBufSize)
	fmt.Printf("============ret:%v\n", int(ret))
	if int(ret) != 0 {
		fmt.Printf("============ret:%v\n", int(ret))
		return nil, errors.New("gen_pk0s_from_npot error for C")
	}
	pk0s = make([]byte, uint64(pk0sBufSize))
	head2 := (*reflect.SliceHeader)(unsafe.Pointer(&pk0s))
	C.memcpy(unsafe.Pointer(head2.Data), unsafe.Pointer(pk0sbuf), C.ulong(head2.Len))
	return pk0s, nil
}

//int get_matrix_AxorD(void *psi_r, const char *uBuffInputTxorR, const ui64 uBuffInputSize,
//                         const char **receiver_set, const ui64 receiver_set_size, const int id_size,
//                         char **sendMatrixADBuff, ui64 *sendMatixADBuffSize);

func (r *PsiReceiver) GetMatrixAxorD(matrixTxorR []byte, receiverSet []string) (matrixAxorD []byte, err error) {
	head := (*reflect.SliceHeader)(unsafe.Pointer(&matrixTxorR))
	receiver_set_size := len(receiverSet)
	receiver_set := make([]*C.char, receiver_set_size)
	for i := 0; i < receiver_set_size; i++ {
		receiver_set[i] = C.CString(receiverSet[i])
	}
	defer func() {
		for i := 0; i < receiver_set_size; i++ {
			C.free(unsafe.Pointer(receiver_set[i]))
		}
	}()
	head2 := (*reflect.SliceHeader)(unsafe.Pointer(&receiver_set))
	var sendMatrixADBuff *C.char
	var sendMatixADBuffSize C.ulong
	ret := C.get_matrix_AxorD(r.receiver, (*C.char)(unsafe.Pointer(head.Data)), C.ulong(head.Len),
		(**C.char)(unsafe.Pointer(head2.Data)), C.ulong(head2.Len), C.int(len(receiverSet[0])),
		&sendMatrixADBuff, &sendMatixADBuffSize)
	if int(ret) != 0 {
		return nil, errors.New("get_matrix_AxorD error for C")
	}
	matrixAxorD = make([]byte, uint64(sendMatixADBuffSize))
	head3 := (*reflect.SliceHeader)(unsafe.Pointer(&matrixAxorD))
	C.memcpy(unsafe.Pointer(head3.Data), unsafe.Pointer(sendMatrixADBuff), sendMatixADBuffSize)
	return matrixAxorD, nil
}

//int gen_all_hash_map(void *psi_r);
func (r *PsiReceiver) GenAllHashMap() error {
	ret := C.gen_all_hash_map(r.receiver)
	if int(ret) != 0 {
		return errors.New("gen_all_hash_map error for C")
	}
	return nil
}

//    int is_recv_end(void *psi_r);
func (r *PsiReceiver) IsRecvEnd() bool {
	ret := C.is_recv_end(r.receiver)
	if int(ret) == 0 {
		return false
	}
	return true
}

//int recv_from_sender_and_compute_psi_once(void *psi_r, const char *recv_buff,
//                                              const ui64 recv_buf_size);

func (r *PsiReceiver) RecvFromSenderAndComputePsiOnce(hashOutput []byte) error {
	head := (*reflect.SliceHeader)(unsafe.Pointer(&hashOutput))
	ret := C.recv_from_sender_and_compute_psi_once(r.receiver, (*C.char)(unsafe.Pointer(head.Data)), C.ulong(head.Len))
	if int(ret) != 0 {
		return errors.New("recv_from_sender_and_compute_psi_once error for C")
	}
	return nil
}

//    int get_psi_results_for_all(void *psi_r, unsigned int **psi_array, ui64 *psi_array_size);
func (r *PsiReceiver) GetPsiResultsForAll() (psiResults []uint32, err error) {
	var psi_array *C.uint
	var psi_array_size C.ulong
	ret := C.get_psi_results_for_all(r.receiver, &psi_array, &psi_array_size)
	if int(ret) != 0 {
		return nil, errors.New("recv_from_sender_and_compute_psi_once error for C")
	}
	psiResults = make([]uint32, uint64(psi_array_size))
	head := (*reflect.SliceHeader)(unsafe.Pointer(&psiResults))
	C.memcpy(unsafe.Pointer(head.Data), unsafe.Pointer(psi_array), psi_array_size*4)
	return psiResults, nil
}
