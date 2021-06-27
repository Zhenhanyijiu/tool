package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -ltest_c
#include"test_c.h"
*/
import "C"
import (
	"fmt"
	"unsafe"
)

//void getrng(const char *in, const int in_len, char *out, int *out_len);
func getrng() {
	in := []byte("123456")
	out := make([]byte, 1024)
	out_len := 0
	C.getrng((*C.char)(unsafe.Pointer(&in[0])), C.int(0), (*C.char)(unsafe.Pointer(&out[0])), (*C.int)(unsafe.Pointer(&out_len)))
	fmt.Printf("out:%x,%v\n", out[:out_len], out_len)
}

func main() {
	getrng()
	
}
