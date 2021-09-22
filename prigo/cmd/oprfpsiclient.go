package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"prigo/algo/message"
	"runtime"
)

//./startServer -nodeId=8800 -sa=127.0.0.1:8800
//./startServer -nodeId=8801 -sa=127.0.0.1:8801
type OprfPsiParam struct {
	ReceiverSize uint64 `json:"receiver_size"`
	SenderSize   uint64 `json:"sender_size"`
	OmpNum       int    `json:"omp_num"`
}

func main() {
	ip1 := "127.0.0.1:8000"
	//ip1 := "10.100.3.15:8800"
	ip2 := "127.0.0.1:8001"
	//ip2 := "10.100.3.16:8801"
	url1 := "http://" + ip1 + "/v1/algo/start"
	url2 := "http://" + ip2 + "/v1/algo/start"
	rs := flag.Uint64("rs", 5000000, "receiver size")
	ss := flag.Uint64("ss", 5000000, "sender size")
	flag.Parse()
	OmpNum := runtime.NumCPU()
	if OmpNum <= 2 {
		OmpNum = 1
	} else {
		OmpNum = OmpNum / 2
	}
	oprfpsiparam := OprfPsiParam{ReceiverSize: *rs, SenderSize: *ss, OmpNum: OmpNum}
	rawParam, _ := json.Marshal(&oprfpsiparam)
	req1 := message.MsgRequest{
		//FlowID: uu.String(),
		AlgoId: "OprfPsi",
		CurrNode: message.Node{
			Role:   message.GUEST,
			Owner:  message.OWNER,
			NodeID: "8800",
			Ip:     ip1},
		PartnerNodes: []message.Node{
			message.Node{
				Role:   message.HOST,
				Owner:  message.PARTICIPATION,
				NodeID: "8801",
				Ip:     ip2},
		},
		Params: rawParam,
	}
	req2 := message.MsgRequest{
		//FlowID: uu.String(),
		AlgoId: "OprfPsi",
		CurrNode: message.Node{
			Role:   message.HOST,
			Owner:  message.PARTICIPATION,
			NodeID: "8801",
			Ip:     ip2},
		PartnerNodes: []message.Node{
			message.Node{
				Role:   message.GUEST,
				Owner:  message.OWNER,
				NodeID: "8800",
				Ip:     ip1},
		},
		Params: rawParam,
	}
	for i := 0; i < 1000; i++ {
		go startOprfPsiPost(req1, url1)
		startOprfPsiPost(req2, url2)
	}

}

func startOprfPsiPost(data interface{}, urlData string) {
	c1 := http.Client{}
	da, _ := json.Marshal(data)
	rsp, err := c1.Post(urlData, "application/json", bytes.NewReader(da))
	if err != nil {
		fmt.Printf("error: %v\n", err)
		return
	}
	rsp.Body.Close()
}
