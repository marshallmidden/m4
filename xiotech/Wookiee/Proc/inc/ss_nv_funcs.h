/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
UINT32    clear_all_ss_nv(OGER * nv_oger);
void      restore_all_ss_nv(void);
void      restore_snap_data(UINT16 spool_vid);
UINT32    update_oger_nv(OGER * oger, OGER * nv_oger);
OGER      *restore_oger_nv(UINT16 ordinal, OGER * nv_oger);
UINT32    update_ssms_nv(SSMS * ssms, OGER * nv_oger);
UINT32    add_ssms_nv(SSMS * ssms, OGER * nv_oger);
SSMS      *restore_ssms_nv(UINT16 ordinal, OGER * nv_oger);
UINT32    update_header_nv(SS_HEADER_NV * src_header_nv, OGER * nv_oger);
SS_HEADER_NV *restore_header_nv(OGER * nv_oger);
UINT16    find_owning_dcn(UINT16 vid);
void      clean_vdd_refs(UINT8);
UINT32    update_header_nv_magic(UINT16 spool_vid);
void      ss_invalidate_allsnapshots(UINT16 spool_vid);
int       i_should_own_sp(int my_dcn, int test_dcn);
