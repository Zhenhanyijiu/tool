package main

import (
	"flag"
	"prigo/server"
)

func main() {
	//g := gin.New()
	sa := flag.String("sa", "", "listen address...")
	nodeId := flag.String("nodeId", "", "this node id...")
	flag.Parse()
	if *sa == "" || *nodeId == "" {
		flag.Usage()
		return
	}
	ser := server.NewServer(*nodeId, *sa)
	ser.Route()
}
