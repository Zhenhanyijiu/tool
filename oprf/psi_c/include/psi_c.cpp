#include "psi_c.h"
#include "psi.h"
#include <stdlib.h>
#include <vector>
namespace oc = osuCrypto;
using namespace std;

typedef struct psi_sender
{
    oc::PsiSender *sender;
    ui64 sender_size;

} psi_sender;
void *new_psi_sender(char *common_seed, ui64 sender_size, int omp_num)
{
    psi_sender *psi_s = (psi_sender *)malloc(sizeof(psi_sender));
    if (psi_s == NULL)
    {
        return NULL;
    }
    psi_s->sender_size = sender_size;
    oc::PsiSender *sender = new oc::PsiSender;
    int ret = sender->init((oc::u8_t *)common_seed, sender_size,
                           128, 20, omp_num, 10, 10240);
    if (ret)
    {
        free(psi_s);
        psi_s = NULL;
        return NULL;
    }
    psi_s->sender = sender;
    return psi_s;
}
void release_psi_sender(void *psi_s)
{
    psi_sender *s = (psi_sender *)psi_s;
    if (psi_s)
    {
        delete s->sender;
        free(psi_s);
        psi_s = NULL;
    }
}

int gen_public_param(void *psi_s, char **pub_param_buf, ui64 *pub_param_buf_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender->genPublicParamFromNpot((oc::u8_t **)pub_param_buf, (oc::u64_t *)pub_param_buf_size);
}

int gen_matrix_T_xor_R(void *psi_s, char *pk0s, const ui64 pk0s_size,
                       char **matrix_TxorR, ui64 *matrix_TxorR_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender->genMatrixTxorRBuff((oc::u8_t *)pk0s, pk0s_size, (oc::u8_t **)matrix_TxorR,
                                         (oc::u64_t *)matrix_TxorR_size);
}

int compute_all_hash_output_byH1(void *psi_s, const char **sender_set,
                                 const ui64 sender_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    if (s->sender_size != sender_size)
    {
        return -1;
    }

    vector<vector<oc::u8_t>> senderSet(sender_size);
    for (oc::u8_t i = 0; i < sender_size; i++)
    {
        int len = strlen(sender_set[i]);
        senderSet[i].resize(len);
        memcpy(senderSet[i].data(), sender_set[i], len);
    }
    return s->sender->computeAllHashOutputByH1(senderSet);
    // return 0;
}
int recover_matrix_C(void *psi_s, const char *recv_matrix_A_xor_D,
                     const ui64 recv_matrix_A_xor_D_size)
{

    psi_sender *s = (psi_sender *)psi_s;
    return s->sender->recoverMatrixC((oc::u8_t *)recv_matrix_A_xor_D,
                                     (oc::u64_t)recv_matrix_A_xor_D_size);
    // return 0;
}
int is_send_end(void *psi_s)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender->isSendEnd();
}

// int computeHashOutputToReceiverOnce(u8_t *hashOutputBuff, u64_t *sendBuffSize);
int compute_hash_output_to_receiver_once(void *psi_s, char *hashOutputBuff, ui64 *sendBuffSize)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender->computeHashOutputToReceiverOnce((oc::u8_t *)hashOutputBuff,
                                                      (oc::u64_t *)sendBuffSize);
}

////////////////recv
typedef struct psi_receiver
{
    oc::PsiReceiver receiver;
    ui64 receiver_size;
    ui64 sender_size;
    vector<oc::u32_t> psi_array;
} psi_receiver;
// typedef struct int_buffer
// {
//     unsigned int *psi_array;
//     ui64 psi_array_size;
//     ui64 psi_array_max_len;
// } int_buffer;

void *new_psi_receiver(char *common_seed, ui64 receiver_size, ui64 sender_size, int omp_num)
{
    psi_receiver *psi_r = (psi_receiver *)malloc(sizeof(psi_receiver));
    if (psi_r == NULL)
    {
        return NULL;
    }
    // unsigned int *psi_array=(unsigned int)malloc()
    psi_r->sender_size = sender_size;
    psi_r->receiver_size = receiver_size;
    int ret = psi_r->receiver.init((oc::u8_t *)common_seed, receiver_size, sender_size,
                                   128, 20, omp_num, 10, 10240);
    if (ret)
    {
        free(psi_r);
        psi_r = NULL;
        return NULL;
    }
    return psi_r;
}
void release_psi_receiver(void *psi_r)
{
    if (psi_r)
    {
        free(psi_r);
        psi_r = NULL;
    }
}
// int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
//                            u8_t **pk0Buf, u64_t *pk0BufSize);
int gen_pk0s_from_npot(void *psi_r, char *pubParamBuf, const ui64 pubParamBufByteSize,
                       char **pk0Buf, ui64 *pk0BufSize)
{
    psi_receiver *r = (psi_receiver *)psi_r;
    return r->receiver.genPK0FromNpot((oc::u8_t *)pubParamBuf, (oc::u64_t)pubParamBufByteSize,
                                      (oc::u8_t **)pk0Buf, (oc::u64_t *)pk0BufSize);
    // return 0;
}

// int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
//                                 const vector<vector<u8_t>> receiverSet,
//                                 u8_t **sendMatrixADBuff, u64_t *sendMatixADBuffSize);
int get_matrix_AxorD(void *psi_r, const char *uBuffInputTxorR, const ui64 uBuffInputSize,
                     const char **receiver_set, const ui64 receiver_set_size,
                     char **sendMatrixADBuff, ui64 *sendMatixADBuffSize)
{
    psi_receiver *r = (psi_receiver *)psi_r;
    if (r->receiver_size != receiver_set_size)
    {
        return -1;
    }
    vector<vector<oc::u8_t>> receiverSet(receiver_set_size);
    return r->receiver.getSendMatrixADBuff((oc::u8_t *)uBuffInputTxorR, uBuffInputSize, receiverSet,
                                           (oc::u8_t **)sendMatrixADBuff,
                                           (oc::u64_t *)sendMatixADBuffSize);
}

//int genenateAllHashesMap();
int gen_all_hash_map(void *psi_r)
{
    psi_receiver *r = (psi_receiver *)psi_r;
    return r->receiver.genenateAllHashesMap();
}
//int isRecvEnd();
int is_recv_end(void *psi_r)
{
    psi_receiver *r = (psi_receiver *)psi_r;
    return r->receiver.isRecvEnd();
}

// int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
//                                             vector<u32_t> *psiMsgIndex);
int recv_from_sender_and_compute_psi_once(void *psi_r, const char *recv_buff, const ui64 recvBufSize,
                                          unsigned int **psi_array, ui64 *psi_array_size)
{
    // vector<oc::u32_t> psiMsgIndex;
    psi_receiver *r = (psi_receiver *)psi_r;
    r->psi_array.clear();
    int ret = r->receiver.recvFromSenderAndComputePSIOnce((oc::u8_t *)recv_buff, recvBufSize,
                                                          &(r->psi_array));
    if (ret)
    {
        return -3;
    }
    *psi_array = (unsigned int *)(r->psi_array.data());
    *psi_array_size = r->psi_array.size();
    return 0;
}