package main

/*
#cgo CFLAGS: -I./include
#cgo LDFLAGS: -L. -lpsi
#include "psi_c.h"
*/
import "C"
import (
	"fmt"
	"unsafe"
)

//void *new_psi_sender(char *common_seed, ui64 sender_size, int omp_num);
func NewPsiSender() unsafe.Pointer {
	//common_seed := make([]byte, 16)
	common_seed := []byte("1234567890123456")
	comSeed := (*C.char)(unsafe.Pointer(&common_seed[0]))
	sender_size := 50000
	return C.new_psi_sender(comSeed, C.ulonglong(sender_size), C.int(1))
}
func main() {
	s := NewPsiSender()
	fmt.Printf("s:%p\n", s)
}
