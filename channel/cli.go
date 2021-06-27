package main

import (
	"fmt"
	"net"
	"time"
)

func main() {
	con, err := net.DialTimeout("tcp", "127.0.0.1:8888", time.Second*2)
	if err != nil {
		fmt.Printf("error:%v\n", err)
		return
	}
	fmt.Printf("comm:%v\n", con)
	con.Write([]byte("abcdef"))
	pbuf := make([]byte, 30)
	n, err := con.Read(pbuf)
	fmt.Printf("=====pb:%v,n:%v\n", pbuf, n)

	con.Close()
	//time.Sleep(88 * time.Second)
}
