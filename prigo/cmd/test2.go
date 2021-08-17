package main

import (
	"fmt"
	"prigo/channel"
	"strconv"
	"time"
)

func main() {
	curIp := "127.0.0.2:18002"
	//curNode := "http://" + curIp + "/cache"
	peerNode := "http://127.0.0.1:18001/cache"

	ch, err := channel.NewChannel(curIp)
	if err != nil {
		fmt.Printf("new channel error:%v\n", err)
		return
	}
	ch.Start()
	recv, err := ch.RecvData("key1", 100)
	if err != nil {
		fmt.Printf("RecvData error:%v\n", err)
		return
	}
	fmt.Printf("recv data:%v\n", string(recv))
	ch.SendData(peerNode, "key2", []byte("key2-2222"))
	for i := 0; i < 100; i++ {
		recv, err := ch.RecvData(strconv.Itoa(i), 100)
		if err != nil {
			fmt.Printf("RecvData error:%v\n", err)
			return
		}
		fmt.Printf("recv data:%v\n", string(recv))
	}
	time.Sleep(time.Second * 1000)
}
