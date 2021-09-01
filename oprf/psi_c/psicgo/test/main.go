package main

import "C"
import (
	"fmt"
	"psicgo/psicgo"
)

//env LD_LIBRARY_PATH=../psicgo/lib/
func main() {
	comSeed := []byte{1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1}
	s, err := psicgo.NewPsiSender(comSeed, 50000, 1)
	if err != nil {
		panic(err)
	}
	pubParam, err := s.GenPublicParam()
	if err != nil {
		panic(err)
	}
	fmt.Printf("pubParam:%+v,len:%v\n", pubParam, len(pubParam))
	s.Release()
	fmt.Printf("s:%p\n", s)
}
