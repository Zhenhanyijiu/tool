#include "psi_c.h"
#include "psi.h"
#include <stdlib.h>
#include <vector>
namespace oc = osuCrypto;
using namespace std;
typedef struct psi_sender
{
    oc::PsiSender sender;
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
    int ret = psi_s->sender.init((oc::u8_t *)common_seed, sender_size,
                                 128, 20, omp_num, 10, 10240);
    if (ret)
    {
        free(psi_s);
        psi_s = NULL;
        return NULL;
    }
    return psi_s;
}
void release_psi_sender(void *psi_s)
{
    if (psi_s)
    {
        free(psi_s);
        psi_s = NULL;
    }
}

int gen_public_param(void *psi_s, char **pub_param_buf, ui64 *pub_param_buf_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.genPublicParamFromNpot((oc::u8_t **)pub_param_buf, pub_param_buf_size);
}

int gen_matrix_T_xor_R(void *psi_s, char *pk0s, const ui64 pk0s_size,
                       char **matrix_TxorR, ui64 *matrix_TxorR_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.genMatrixTxorRBuff((oc::u8_t *)pk0s, pk0s_size,
                                        (oc::u8_t **)matrix_TxorR, matrix_TxorR_size);
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
    return s->sender.computeAllHashOutputByH1(senderSet);
    // return 0;
}
int recover_matrix_C(void *psi_s, const char *recv_matrix_A_xor_D,
                     const ui64 recv_matrix_A_xor_D_size)
{

    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.recoverMatrixC((oc::u8_t *)recv_matrix_A_xor_D,
                                    (oc::u64_t)recv_matrix_A_xor_D_size);
    // return 0;
}
int is_send_end(void *psi_s)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.isSendEnd();
}

// int computeHashOutputToReceiverOnce(u8_t *hashOutputBuff, u64_t *sendBuffSize);
int compute_hash_output_to_receiver_once(void *psi_s, char *hashOutputBuff, ui64 *sendBuffSize)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.computeHashOutputToReceiverOnce((oc::u8_t *)hashOutputBuff,
                                                     (oc::u64_t *)sendBuffSize);
}

////////////////recv
typedef struct psi_receiver
{
    oc::PsiReceiver receiver;
    ui64 receiver_size;
    ui64 sender_size;

} psi_receiver;

void *new_psi_receiver(char *common_seed, ui64 receiver_size, ui64 sender_size, int omp_num)
{
    psi_receiver *psi_r = (psi_receiver *)malloc(sizeof(psi_receiver));
    if (psi_r == NULL)
    {
        return NULL;
    }
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