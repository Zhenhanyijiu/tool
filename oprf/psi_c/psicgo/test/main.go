package main

import "C"
import (
	"crypto/md5"
	"encoding/hex"
	"fmt"
	"psicgo/psicgo"
	"strconv"
)

//env LD_LIBRARY_PATH=../psicgo/lib/
func main() {
	comSeed := []byte{1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1}
	receiverSize, senderSize := uint64(500000), uint64(500000)
	recvSet := genDataSetTest(1, receiverSize, 10000)
	sendSet := genDataSetTest(0, senderSize, 10000)
	//psiSender
	s, err := psicgo.NewPsiSender(comSeed, senderSize, 1)
	if err != nil {
		panic(err)
	}
	defer func() { s.Release() }()
	//psiReceiver
	r, err := psicgo.NewPsiReceiver(comSeed, receiverSize, senderSize, 1)
	if err != nil {
		panic(err)
	}
	defer func() { r.Release() }()
	pubParam, err := s.GenPublicParam()
	if err != nil {
		panic(err)
	}
	fmt.Printf("pubParam:%+v,len:%v\n", pubParam, len(pubParam))
	pk0s, err := r.GenPk0sFromNpot(pubParam)
	if err != nil {
		fmt.Printf("====>>>error:%v\n", err)
		panic(err)
	}
	TxorR, err := s.GenMatrixTxorR(pk0s)
	if err != nil {
		panic(err)
	}
	AxorD, err := r.GetMatrixAxorD(TxorR, recvSet)
	if err != nil {
		panic(err)
	}
	err = r.GenAllHashMap()
	if err != nil {
		panic(err)
	}
	err = s.ComputeAllHashOutputByH1(sendSet, 21)
	if err != nil {
		panic(err)
	}
	err = s.RecoverMatrixC(AxorD)
	if err != nil {
		panic(err)
	}
	for s.IsSendEnd() == false {
		hashOutput, err := s.ComputeHashOutputToReceiverOnce()
		if err != nil {
			panic(err)
		}
		err = r.RecvFromSenderAndComputePsiOnce(hashOutput)
		if err != nil {
			panic(err)
		}
	}
	psiResult, err := r.GetPsiResultsForAll()
	if err != nil {
		panic(err)
	}
	fmt.Printf("psiResult len:%v\n", len(psiResult))
	//fmt.Printf("s:%p\n", s)
}
func genDataSetTest(ptype int, setSize uint64, ps uint64) []string {
	if ps >= setSize {
		return nil
	}
	dataSet := make([]string, setSize)
	if ptype == 1 {
		md5Hash := md5.New()
		for i := uint64(0); i < setSize; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte(strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		return dataSet
	}
	if ptype == 0 {
		i := uint64(0)
		md5Hash := md5.New()
		for ; i < ps; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte(strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		for ; i < setSize; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte("xx" + strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		return dataSet
	}
	return nil
}

type OOP struct {
	Name string
}

type OOPFunc func(*OOP)

func NewP(op ...OOPFunc) {

}
func Name(n string) OOPFunc {
	return func(oop *OOP) {
		oop.Name = n
	}
}
