package oprfpsi

import (
	"crypto/md5"
	"encoding/hex"
	"encoding/json"
	"errors"
	"prigo/algo/message"
	"prigo/cgo/psicgo"
	"prigo/channel"
	"strconv"
	"time"
)

var ps = 20000
var timeout = 3600

type OprfPsiParam struct {
	ReceiverSize uint64 `json:"receiver_size"`
	SenderSize   uint64 `json:"sender_size"`
	OmpNum       int    `json:"omp_num"`
}

type OprfPsi struct {
	sendData   channel.SendDataFunc
	recvData   channel.RecvDataFunc
	getSendKey channel.GetSendKeyFunc
	getRecvKey channel.GetRecvKeyFunc
	getSendUrl channel.GetSendUrlFunc
}

func NewOprfPsi() message.Algorithm {
	return &OprfPsi{
		sendData:   channel.SendData,
		recvData:   channel.RecvData,
		getSendKey: channel.GetSendKey,
		getRecvKey: channel.GetRecvKey,
		getSendUrl: channel.GetSendUrl,
	}
}
func getUseTime(start time.Time) int64 {
	useTime := time.Since(start) / 1e6
	return int64(useTime)
}
func genDataSetTest(ptype int, setSize uint64, ps uint64) []string {
	if ps > setSize {
		return nil
	}
	dataSet := make([]string, setSize)
	if ptype == 1 {
		md5Hash := md5.New()
		for i := uint64(0); i < setSize; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte(strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		return dataSet
	}
	if ptype == 0 {
		i := uint64(0)
		md5Hash := md5.New()
		for ; i < ps; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte(strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		for ; i < setSize; i++ {
			md5Hash.Reset()
			md5Hash.Write([]byte("xx" + strconv.Itoa(int(i))))
			hashVal := md5Hash.Sum(nil)
			str := hex.EncodeToString(hashVal)[:21]
			dataSet[i] = str
		}
		return dataSet
	}
	return nil
}
func (p *OprfPsi) startGuest(msg message.MsgRequest) {
	var oprfPsiParam OprfPsiParam
	err := json.Unmarshal(msg.Params, &oprfPsiParam)
	if err != nil {
		panic(errors.New("not ok"))
	}

	senderNode := msg.PartnerNodes[0]
	comSeed := []byte{1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1}
	//receiverSize, senderSize := uint64(100), uint64(100)
	recvSet := genDataSetTest(1, oprfPsiParam.ReceiverSize, uint64(ps))
	start := time.Now()
	//psiReceiver
	r, err := psicgo.NewPsiReceiver(comSeed, oprfPsiParam.ReceiverSize, oprfPsiParam.SenderSize, oprfPsiParam.OmpNum)
	if err != nil {
		panic(err)
	}
	defer func() { r.Release() }()
	//1.接收对方pubParam
	pubParamKey := p.getRecvKey(msg.FlowId, senderNode.NodeID, msg.CurrNode.NodeID, "pubParam")
	pubParam, err := p.recvData(pubParamKey, timeout)
	//2.生成pk0s
	pk0s, err := r.GenPk0sFromNpot(pubParam)
	if err != nil {
		//fmt.Printf("====>>>error:%v\n", err)
		panic(err)
	}
	//3.发送pk0s
	tarUrl := p.getSendUrl(senderNode.Ip)
	pk0sKey := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, senderNode.NodeID, "pk0s")
	p.sendData(tarUrl, pk0sKey, pk0s)
	//4.接收TxorR
	TxorRKey := p.getRecvKey(msg.FlowId, senderNode.NodeID, msg.CurrNode.NodeID, "TxorR")
	TxorR, err := p.recvData(TxorRKey, timeout)
	//5.生成AxorD
	AxorD, err := r.GetMatrixAxorD(TxorR, recvSet)
	if err != nil {
		panic(err)
	}
	//6.发送AxorD
	tarUrl = p.getSendUrl(senderNode.Ip)
	AxorDKey := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, senderNode.NodeID, "AxorD")
	p.sendData(tarUrl, AxorDKey, AxorD)
	//7.计算hashmap
	err = r.GenAllHashMap()
	if err != nil {
		panic(err)
	}
	//8.匹配
	count := 0
	for r.IsRecvEnd() == false {
		hashOutputKey := p.getRecvKey(msg.FlowId, senderNode.NodeID, msg.CurrNode.NodeID, "hashOutput"+strconv.Itoa(count))
		hashOutput, err := p.recvData(hashOutputKey, timeout)
		err = r.RecvFromSenderAndComputePsiOnce(hashOutput)
		if err != nil {
			panic(err)
		}
		count++
	}
	//9.获取结果
	psiResult, err := r.GetPsiResultsForAll()
	if err != nil {
		panic(err)
	}
	message.Log.Infof("psiResult len:%v,count:%v,last index:%v\n", len(psiResult), count, psiResult[len(psiResult)-1])
	message.Log.Infof("===>>guest use time:%vms", getUseTime(start))
}
func (p *OprfPsi) startHost(msg message.MsgRequest) {
	var oprfPsiParam OprfPsiParam
	err := json.Unmarshal(msg.Params, &oprfPsiParam)
	if err != nil {
		panic(errors.New("not ok"))
	}
	comSeed := []byte{1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1}
	//_, senderSize := uint64(100), uint64(100)
	//recvSet := genDataSetTest(1, receiverSize, 100)
	sendSet := genDataSetTest(0, oprfPsiParam.SenderSize, uint64(ps))
	recvNode := msg.PartnerNodes[0]
	startTime := time.Now()
	//1.new psiSender
	s, err := psicgo.NewPsiSender(comSeed, oprfPsiParam.SenderSize, oprfPsiParam.OmpNum)
	if err != nil {
		panic(err)
	}
	defer func() { s.Release() }()
	//2.生成pubParam
	pubParam, err := s.GenPublicParam()
	if err != nil {
		panic(err)
	}
	//3.发送pubParam
	tarUrl := p.getSendUrl(recvNode.Ip)
	pubParamKey := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, recvNode.NodeID, "pubParam")
	p.sendData(tarUrl, pubParamKey, pubParam)
	//4.接收pk0s
	pk0sKey := p.getRecvKey(msg.FlowId, recvNode.NodeID, msg.CurrNode.NodeID, "pk0s")
	pk0s, err := p.recvData(pk0sKey, timeout)
	//5.生成TxorR
	TxorR, err := s.GenMatrixTxorR(pk0s)
	if err != nil {
		panic(err)
	}
	//6.发送TxorR
	tarUrl = p.getSendUrl(recvNode.Ip)
	TxorRKey := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, recvNode.NodeID, "TxorR")
	p.sendData(tarUrl, TxorRKey, TxorR)
	//6.计算hash1
	err = s.ComputeAllHashOutputByH1(sendSet, 21)
	if err != nil {
		panic(err)
	}
	//7.接收AxorD
	AxorDKey := p.getRecvKey(msg.FlowId, recvNode.NodeID, msg.CurrNode.NodeID, "AxorD")
	AxorD, err := p.recvData(AxorDKey, timeout)
	//8.恢复矩阵C
	err = s.RecoverMatrixC(AxorD)
	if err != nil {
		panic(err)
	}
	//9.计算hashOutput
	count := 0
	for s.IsSendEnd() == false {
		hashOutput, err := s.ComputeHashOutputToReceiverOnce()
		//fmt.Printf("hashOutput len:%v,%v\n", len(hashOutput), hashOutput[:200])
		if err != nil {
			panic(err)
		}
		//发送
		tarUrl = p.getSendUrl(recvNode.Ip)
		hashOutputKey := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, recvNode.NodeID, "hashOutput"+strconv.Itoa(count))
		p.sendData(tarUrl, hashOutputKey, hashOutput)
		count++
	}
	message.Log.Infof("===>>guest use time:%vms", getUseTime(startTime))
}
func (p *OprfPsi) Start(msg message.MsgRequest) {
	message.Log.Infof("psi start...")
	switch msg.CurrNode.Role {
	case message.GUEST:
		p.startGuest(msg)
		break
	case message.HOST:
		p.startHost(msg)
		break
	}
	//if msg.CurrNode.Owner == message.OWNER {
	//	fmt.Printf("this is owner...\n")
	//	for _, v := range msg.PartnerNodes {
	//		tar_url := "http://" + v.Ip + "/channel/send"
	//		key := msg.FlowID + "_" + msg.CurrNode.NodeID + "_to_" + v.NodeID
	//		fmt.Printf("===guest send:%v\n", key)
	//		p.sendData(tar_url, key, []byte(key))
	//	}
	//} else {
	//	fmt.Printf("this is participation...\n")
	//	for _, v := range msg.PartnerNodes {
	//		if v.Owner == message.OWNER {
	//			//fmt.Printf("===>>接收数据开始\n")
	//			key := msg.FlowID + "_" + v.NodeID + "_to_" + msg.CurrNode.NodeID
	//			data, _ := p.recvData(key, 30)
	//			fmt.Printf("===>>收到：%v\n", string(data))
	//		}
	//	}
	//}

}
func (p *OprfPsi) Finish() {
	message.Log.Infof("psi finish...")
}
func (p *OprfPsi) init(msg message.MsgRequest) {

}
