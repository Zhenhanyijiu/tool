package main

import (
	"fmt"
	"runtime"
)

func main() {
	m := map[int]int{}
	m[2] = 2
	fmt.Printf("mm:%v\n", m)
	fmt.Printf("NumCPU:%v\n", runtime.NumCPU())
	go func() {
		fmt.Println(">>>>")
	}()
	fmt.Printf("NumGoroutine:%v\n", runtime.NumGoroutine())
	fmt.Printf("GOMAXPROCS:%v\n", runtime.GOMAXPROCS(-1))
	fmt.Printf("NumCPU:%v\n", runtime.NumCPU())
	fmt.Printf("GOOS:%v\n", runtime.GOOS)

}
