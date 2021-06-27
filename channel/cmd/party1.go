package main

import (
	"channel/transport"
	"fmt"
)

func main() {
	t, err := transport.NewTransport(transport.SENDER, "127.0.0.1", "8888")
	if err != nil {
		fmt.Printf("error:%v\n", err)
		return
	}
	t.SendData([]byte("99999999999999"))

}
