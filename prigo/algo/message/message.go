package message

import (
	"encoding/json"
	"github.com/sirupsen/logrus"
)

var Log *logrus.Logger

func init() {
	Log = logrus.New()
	Log.SetLevel(logrus.InfoLevel)
}

type Algorithm interface {
	Start(msg MsgRequest)
	Finish()
}

type NewAlgoFunc func() Algorithm
type Node struct {
	Role   string `json:"role"`
	Owner  string `json:"owner"`
	NodeID string `json:"node_id"`
	Ip     string `json:"ip"`
}
type MsgRequest struct {
	FlowId       string          `json:"flow_id"`
	AlgoId       string          `json:"algo_id"`
	CurrNode     Node            `json:"curr_node"`
	PartnerNodes []Node          `json:"partner_ids"`
	Params       json.RawMessage `json:"params"`
}
type MsgChannelRequest struct {
	Key  string `json:"key"`
	Data []byte `json:"data"`
}

var OWNER = "owner"
var PARTICIPATION = "participation"
var GUEST = "guest"
var HOST = "host"
