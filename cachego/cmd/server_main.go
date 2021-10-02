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
	sa := "127.0.0.1:8000"
	cacheServer := cache.NewCache(sa)
	cycNum := 1000000
	cycNum = *n
	go func() {
		start := time.Now()
		for i := 0; i < cycNum; i++ {
			_, err := cacheServer.GetValue(strconv.Itoa(i), 30)
			if err != nil {
				panic(err)
			}

		}
		fmt.Printf("cycNum:%v,time:%v\n", cycNum, time.Since(start))
	}()
	cacheServer.CacheHandle()
}
