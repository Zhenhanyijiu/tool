package main

import (
	"cachego/cache"
	"flag"
	"fmt"
	"strconv"
	"time"
)

func main() {
	n := flag.Int("n", 10, "cyc num")
	flag.Parse()
	urlStr := "127.0.0.1:8000"
	cacheClient := cache.NewCache(urlStr)
	cycNum := 1000000
	cycNum = *n
	value := make([]byte, 2560)
	start := time.Now()
	for i := 0; i < cycNum; i++ {
		cacheClient.SendData("http://"+urlStr+"/v1/cache/save", strconv.Itoa(i), value)
	}
	//./cli_main -n=1000000
	//cycNum:1000000,time:2m26.291012693s
	fmt.Printf("cycNum:%v,time:%v\n", cycNum, time.Since(start))
}
