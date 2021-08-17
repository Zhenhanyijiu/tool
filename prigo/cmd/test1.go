package main

import (
	"fmt"
	"prigo/channel"
	"strconv"
	"time"
)

func main() {
	curIp := "127.0.0.1:18001"
	//curNode := "http://" + curIp + "/cache"
	peerNode := "http://127.0.0.2:18002/cache"

	ch, err := channel.NewChannel(curIp)
	if err != nil {
		fmt.Printf("new channel error:%v\n", err)
		return
	}
	ch.Start()
	ch.SendData(peerNode, "key1", []byte("key1-1111"))
	recv, err := ch.RecvData("key2", 100)
	if err != nil {
		fmt.Printf("RecvData error:%v\n", err)
		return
	}
	fmt.Printf("recv data:%v\n", string(recv))
	for i := 0; i < 100; i++ {
		index := strconv.Itoa(i)
		ch.SendData(peerNode, index, []byte(index+"__key1-1111"))
	}
	time.Sleep(time.Second * 1000)
}
