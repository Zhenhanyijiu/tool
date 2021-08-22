package psi

import (
	"prigo/algo/message"
	"prigo/channel"
	"strconv"
	"time"
)

type Psi struct {
	sendData   channel.SendDataFunc
	recvData   channel.RecvDataFunc
	getSendKey channel.GetSendKeyFunc
	getRecvKey channel.GetRecvKeyFunc
	getSendUrl channel.GetSendUrlFunc
}

func NewPsi() message.Algorithm {
	return &Psi{
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
func (p *Psi) startGuest(msg message.MsgRequest) {
	start := time.Now()
	for i := 0; i < 10000; i++ {
		for _, v := range msg.PartnerNodes {
			tarUrl := p.getSendUrl(v.Ip)
			key := p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, v.NodeID, strconv.Itoa(i))
			p.sendData(tarUrl, key, []byte(key))
			message.Log.Infof("===>>guest send key:%v,tar_url:%v", key, tarUrl)
		}
		for _, v := range msg.PartnerNodes {
			key := p.getRecvKey(msg.FlowId, v.NodeID, msg.CurrNode.NodeID, strconv.Itoa(i))
			message.Log.Infof("===>>guest recv key:%v", key)
			data, err := p.recvData(key, 10)
			if err != nil {
				message.Log.Errorf("===>>recv data error:%v", err)
				continue
			}
			message.Log.Infof("===>>data:%v", string(data))
		}
	}
	message.Log.Infof("===>>guest use time:%vms", getUseTime(start))
}
func (p *Psi) startHost(msg message.MsgRequest) {
	for i := 0; i < 10000; i++ {
		for _, v := range msg.PartnerNodes {
			if v.Role == message.GUEST {
				key := p.getRecvKey(msg.FlowId, v.NodeID, msg.CurrNode.NodeID, strconv.Itoa(i))
				message.Log.Infof("===>>host recv key:%v", key)
				data, err := p.recvData(key, 10)
				if err != nil {
					message.Log.Errorf("===>>recv data error:%v", err)
				}
				message.Log.Infof("===>>data:%v", string(data))
				//send
				tarUrl := p.getSendUrl(v.Ip)
				key = p.getSendKey(msg.FlowId, msg.CurrNode.NodeID, v.NodeID, strconv.Itoa(i))
				p.sendData(tarUrl, key, []byte(key))
				message.Log.Infof("===>>host send key:%v,tar_url:%v", key, tarUrl)
				break
			}
		}
	}
}
func (p *Psi) Start(msg message.MsgRequest) {
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
func (p *Psi) Finish() {
	message.Log.Infof("psi finish...")
}
func (p *Psi) init(msg message.MsgRequest) {

}
