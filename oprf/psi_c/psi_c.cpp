#include "psi_c.h"
#include "psi.h"
#include <stdlib.h>
namespace oc = osuCrypto;
typedef struct psi_sender
{
    oc::PsiSender sender;
    unsigned long sender_size;

} psi_sender;

void *new_psi_sender(char *common_seed, unsigned long sender_size)
{
    psi_sender *psi_s = (psi_sender *)malloc(sizeof(psi_sender));
    if (psi_s == NULL)
    {
        return NULL;
    }
    psi_s->sender_size = sender_size;
    int ret = psi_s->sender.init((oc::u8_t *)common_seed, sender_size, 128, 20, 10, 10240);
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

int gen_public_param(void *psi_s, char **pub_param_buf, unsigned long *pub_param_buf_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.genPublicParamFromNpot((oc::u8_t **)pub_param_buf, pub_param_buf_size);
}

int gen_matrix_T_xor_R(void *psi_s, char *pk0s, const unsigned long pk0s_size,
                       char **matrix_TxorR, unsigned long *matrix_TxorR_size)
{
    psi_sender *s = (psi_sender *)psi_s;
    return s->sender.genMatrixTxorRBuff((oc::u8_t *)pk0s, pk0s_size,
                                        (oc::u8_t **)matrix_TxorR, matrix_TxorR_size);
}
int recover_matrix_C(void *psi_s, const char *recv_matrix_A_xor_D,
                     const unsigned long recv_matrix_A_xor_D_size,
                     const char **sender_set, const unsigned long sender_size)
{

    psi_sender *s = (psi_sender *)psi_s;
    if (s->sender_size != sender_size)
    {
        return -1;
    }
}