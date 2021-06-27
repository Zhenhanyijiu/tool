package main

import (
	"fmt"
	"net"
	"time"
)

func main() {
	listener, err := net.Listen("tcp", "127.0.0.1:8888")
	//ServerHandleError(err, "net.listen")
	if err != nil {
		fmt.Printf("err:%v\n", err)
		return
	}

	for {
		//循环接入所有客户端得到专线连接
		fmt.Printf("before accept...\n")
		//time.Sleep(21 * time.Second)
		conn, err := listener.Accept()
		if err != nil {
			fmt.Printf("listener err:%v\n", err)
			return
		}
		//开辟独立协程与该客聊天
		go ChatWith(conn)
	}
}
func ChatWith(c net.Conn) {
	pbuf := make([]byte, 30)
	//time.Sleep(7 * time.Second)
	n, err := c.Read(pbuf)
	if err != nil {
		fmt.Printf("listener err:%v\n", err)
		return
	}
	fmt.Printf("===%v,n=%v\n", pbuf, n)
	time.Sleep(7 * time.Second)
	c.Write([]byte("server:9999"))

}
