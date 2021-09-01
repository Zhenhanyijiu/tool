#ifndef _PSI_C_H_
#define _PSI_C_H_
#ifdef __cplusplus
extern "C"
{
#endif
    typedef unsigned long long int ui64;
    //sender
    void *new_psi_sender(char *common_seed, ui64 sender_size, int omp_num);
    void release_psi_sender(void *psi_s);
    int gen_public_param(void *psi_s, char **pub_param_buf, ui64 *pub_param_buf_size);
    int gen_matrix_T_xor_R(void *psi_s, char *pk0s, const ui64 pk0s_size,
                           char **matrix_TxorR, ui64 *matrix_TxorR_size);
    int compute_all_hash_output_byH1(void *psi_s, const char **sender_set,
                                     const ui64 sender_size);
    int recover_matrix_C(void *psi_s, const char *recv_matrix_A_xor_D,
                         const ui64 recv_matrix_A_xor_D_size);
    int is_send_end(void *psi_s);
    int compute_hash_output_to_receiver_once(char *hashOutputBuff, ui64 *sendBuffSize);
    //recevicer
    void *new_psi_receiver(char *common_seed, ui64 receiver_size, ui64 sender_size,
                           int omp_num);
    void release_psi_receiver(void *psi_r);
    int gen_pk0s_from_npot(void *psi_r, char *pubParamBuf, const ui64 pubParamBufByteSize,
                           char **pk0Buf, ui64 *pk0BufSize);
    int get_matrix_AxorD(void *psi_r, const char *uBuffInputTxorR, const ui64 uBuffInputSize,
                         const char **receiver_set, const ui64 receiver_set_size,
                         char **sendMatrixADBuff, ui64 *sendMatixADBuffSize);
    int gen_all_hash_map(void *psi_r);
    int is_recv_end(void *psi_r);
    int recv_from_sender_and_compute_psi_once(void *psi_r, const char *recv_buff, const ui64 recvBufSize,
                                              unsigned int **psi_array, ui64 *psi_array_size);
#ifdef __cplusplus
}
#endif
#endif