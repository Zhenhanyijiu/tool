package main

import (
	"channel/transport"
	"fmt"
	"time"
)

func main() {
	t, err := transport.NewTransport(transport.RECEIVER, "127.0.0.1", "8888")
	if err != nil {
		fmt.Printf("error:%v\n", err)
		return
	}
	time.Sleep(time.Second * 17)
	buf, err := t.RecvData()
	if err != nil {
		fmt.Printf("error:%v\n")
		return
	}
	fmt.Printf("buf:%v\n", string(buf))

}
