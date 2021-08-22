package base

import (
	"prigo/algo/message"
	"prigo/algo/psi"
)

//register algo
var AlgoTable = map[string]message.NewAlgoFunc{
	"PsiAlgo": psi.NewPsi,
}

//func SetAlgorithm(key string, val Algorithm) {
//	algo, ok := AlgoTable[key]
//	if !ok {
//		AlgoTable[key] = algo
//	}
//	return
//}

func GetAlgorithm(algoId string) message.Algorithm {
	algo, ok := AlgoTable[algoId]
	if !ok {
		return nil
	}
	return algo()
}
