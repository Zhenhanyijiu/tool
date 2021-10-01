package main

import (
	"fmt"
	"prigo/channel"
	"strconv"
	"time"
)

func main() {
	urlStr := "http://" + "127.0.0.1:8000" + "/v1/channel/send"
	cyc := 1000000
	start := time.Now()
	data := make([]byte, 2560)
	for i := 0; i < cyc; i++ {
		channel.SendData(urlStr, strconv.Itoa(i), data)
	}
	//cyc:1000000,time:2m1.713473067s(8196.7次/s)
	fmt.Printf("cyc:%v,time:%v\n", cyc, time.Since(start))
}
